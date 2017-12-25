#pragma once

#include "Pool.hpp"
#include <assert.h>
#include <memory>
#include <stdlib.h>
#include <unordered_map>
#include <utility>
#include <vector>
#include "ComponentGroup.hpp"
#include "Identification.hpp"
#include "MetaUtils.hpp"

/**
 * @brief Entities
 */

namespace Concordia {
	//TODO: This does not have to be a seperate class, its just for keeping a static counter
	struct ICmp {
	protected:
		
	};

	//TODO: Make ICmp and Cmp classes obsolute by creating two static functions -Maiko
	template <typename E> struct Cmp : public ICmp {
		static size_t id() {
			static const size_t s_id = generate_id();
			return s_id;
		}
	};

	class EntityMgr;
	template<typename... Cmps >
	struct EntityComponentDependency;

	struct Entity {
	private:
		friend class EntityMgr;
		Entity(size_t id, EntityMgr &entityMgr) : m_id(id), m_entityMgr(entityMgr) {}

	public:
		template <typename C> inline void addComponent(const C &cmp);

		template <typename C, typename... Args>
		inline void addComponent(Args... args);

		template <typename C> inline bool hasComponent() const;

		template <typename C> inline C &getComponent() const;

		template <typename C> inline void removeComponent() const;

		template<typename... Cmps>
		std::vector<EntityComponentDependency<Cmps...>> EntitiesWith() const;

		inline size_t id() const { return m_id; }

		friend bool operator==(const Entity& a, const Entity& b)
		{
			return a.m_id == b.m_id;
		}

		friend bool operator!=(const Entity& a, const Entity& b)
		{
			return a.m_id != b.m_id;
		}

	private:
		size_t m_id;
		EntityMgr &m_entityMgr;
		std::vector<size_t> m_cmps;
	};

	namespace Impl
	{
		template<typename T, typename... Cmps>
		constexpr std::array<void*, sizeof...(Cmps)> GetAllCmps(Entity entity)
		{
			std::array<void*, sizeof...(Cmps)> ret;

			ret[0] = entity.getComponent<T>();


			return ret;
		}
	}

	template<typename ... Cmps>
	constexpr std::array<void*, sizeof...(Cmps)> GetAllCmps(Entity entity)
	{
		return { static_cast<void*>(&entity.getComponent<Cmps>())... };
	}

	///A entity which is guaranteed to have these components, for easy use in functions
	template<typename... Cmps>
	struct EntityComponentDependency : public ComponentGroup<Cmps...>
	{
		using Super = ComponentGroup<Cmps...>;
		//TODO: Is there a better way to do templated inheritance?
		using Super::ids;
		using Super::cmps;
		using Super::get;
		using Super::size;

		Entity entity;

		EntityComponentDependency(Entity entity)
			: ComponentGroup<Cmps...>(), entity(entity)
		{
			cmps = GetAllCmps<Cmps...>(entity);
		}

		EntityComponentDependency(Entity entity, Cmps... cmps)
			: ComponentGroup<Cmps...>(std::forward<Cmps>(cmps)...), entity(entity)
		{}
	};

	class EntityMgr {
	private:
		template<typename C>
		struct ComponentElement
		{
			size_t entity_id;
			C component;
		};

		template <typename C> 
		using CmpPool = Pool<ComponentElement<C>>;

		template <typename C> inline CmpPool<C> &getPool() {
			auto handle = static_cast<CmpPool<C> *>(m_cmpMap[get_id<C>()]);
			if (handle == nullptr) {
				handle = new CmpPool<C>{};
				m_cmpMap[get_id<C>()] = handle;
			}
			return *handle;
		}


		inline CmpPool<void*>& getPool(size_t cmd_id)
		{
			assert(false && "this should never be called, it is obsolete");

			auto handle = m_cmpMap[cmd_id];
			if (handle == nullptr) {
				//TODO: We do not know the size of the cmp pool...
				//handle = new CmpPool<void*>{};
				//m_cmpMap[cmd_id] = handle;
			}

			//static_assert(false, "We cannot know the size of the type");
			//TODO: So this does not work either...
			CmpPool<void*>* cmp_pool = static_cast<CmpPool<void*>*>(handle);
			return *cmp_pool;
		}

	public:
		Entity createEntity() {
			static size_t s_entityCounter = -1;
			return Entity(++s_entityCounter, *this);
		}

		void addEntity(Entity e) { m_entities.emplace_back(e); }

		void removeEntity(Entity e)
		{
			//TODO: Remove Entity
		}

		///Prefer not using this, but create a entity and set the EntityMgr
		Entity getEntity(size_t entity_id)
		{
			for (Entity entity : m_entities)
			{
				if (entity.id() == entity_id)
					return entity;
			}

			throw std::exception("Cannot find an entity with the id");
		}

		template <typename C> void addComponent(Entity e, const C &cmp) {
			auto &poolHandle = getPool<C>();
			poolHandle.add({ e.id(), cmp });
		}

		template <typename C, typename... Args>
		void addComponent(Entity e, Args... args) {
			auto &poolHandle = getPool<C>();
			poolHandle.add({ e.id(), C{args...} });
		}

		template <typename C> bool hasComponent(Entity e) {
			auto &poolHandle = getPool<C>();
			for (auto data : poolHandle.data()) {
				if (data.first && data.second.entity_id == e.id()) {
					return true;
				}
			}
			return false;
		}

		//Cannot work because we do not know the type of the component
		/*bool hasComponent(Entity e, size_t cmp_id) {
			auto &poolHandle = getPool(cmp_id);
			for (auto data : poolHandle.data()) {
				if (data.first && data.second.entity_id == e.id()) {
					return true;
				}
			}
			return false;
		}*/

		//TODO: return a pointer and don't throw an error
		template <typename C> C &getComponent(Entity e) {
			auto &poolHandle = getPool<C>();
			for (auto &data : poolHandle.data()) {
				if (data.first && data.second.entity_id == e.id()) {
					return data.second.component;
				}
			}
			assert(false && "Entity doesn't have component!");
		}

		//Cannot work becaues we do not know the type of the component
		/*void* getComponent(Entity e, size_t cmp_id)
		{
			auto &poolHandle = getPool(cmp_id);
			for (auto &data : poolHandle.data()) {
				if (data.first && data.second.entity_id == e.id()) {
					return data.second.component;
				}
			}

			return nullptr;
		}*/

		//TODO: make a variadic getAnyComponent method
		//TODO: Add a check to see if CBase is the base of all Cs...
		template<typename CBase, typename... Cs> 
		CBase* getAnyComponent(Entity e)
		{
			ComponentGroup<Cs...> cmps;
			for (int i = 0; i < cmps.size; ++i)
			{
				if(void* cmp = getComponent(e, cmps.ids[i]))
				{
					return static_cast<CBase*>(cmp);
				}
			}

			return nullptr;
		}

		template <typename C> void removeComponent(Entity e) {
			auto poolHandle = getPool<C>();
			for (auto data : poolHandle.data()) {
				if (data.first == e.id()) {
					poolHandle.remove(data);
				}
			}
		}

		inline std::vector<Entity>& entities() { return m_entities; }

		/// GetNInTypelist a list of entities with those components
		template<typename... Cmps>
		std::vector<EntityComponentDependency<Cmps...>> getEntitiesWith()
		{
			static_assert(sizeof...(Cmps) > 0, "There needs to be at least one component, \
			if you want a list of all entities, call getEntities()");

			ComponentGroup<Cmps...> group;

			return getEntitiesWith(group); 
		}

		/// overload for a cmp group with built in id list
		template<typename... Cmps>
		std::vector<EntityComponentDependency<Cmps...>> getEntitiesWith(const ComponentGroup<Cmps...>& group);

	private:
		std::unordered_map<size_t, IPool*> m_cmpMap;
		std::vector<Entity> m_entities;
	};

	template <typename C> inline void Entity::addComponent(const C &cmp) {
		m_entityMgr.addComponent(id(), cmp);
	}

	template <typename C, typename... Args>
	inline void Entity::addComponent(Args... args) {
		m_entityMgr.addComponent<C>(*this, std::forward<Args>(args)...);
	}

	template <typename C> inline bool Entity::hasComponent() const {
		return m_entityMgr.hasComponent<C>(*this);
	}

	template <typename C> inline C &Entity::getComponent() const {
		return m_entityMgr.getComponent<C>(*this);
	}

	template <typename C> inline void Entity::removeComponent() const {
		m_entityMgr.removeComponent<C>();
	}

	template <typename... Cmps>
	std::vector<EntityComponentDependency<Cmps...>> Entity::EntitiesWith() const
	{
		return m_entityMgr.getEntitiesWith<Cmps...>();
	}

	namespace Impl
	{
		//TODO: Will this always be true?
		template<typename ... Cmps>
		struct HasAllCmpsStruct
		{
			const bool value = false;
		};

		template<typename C, typename  ... Cmps>
		struct HasAllCmpsStruct<C, Cmps...>
		{
			const bool value;

			HasAllCmpsStruct(Entity entity)
				: value(entity.hasComponent<C>() ? HasAllCmpsStruct<Cmps...>(entity).value : false )
			{}
		};

		template<typename C>
		struct HasAllCmpsStruct<C>
		{
			const bool value;

			HasAllCmpsStruct(Entity entity)
				: value(entity.hasComponent<C>())
			{}
		};
	}

	template<typename... Cmps>
	bool HasAllCmps(Entity entity) { return Impl::HasAllCmpsStruct<Cmps...>{entity}.value; }

	template <typename ... Cmps>
	std::vector<EntityComponentDependency<Cmps...>> EntityMgr::getEntitiesWith(const ComponentGroup<Cmps...>& group)
	{
		static_assert(sizeof...(Cmps) > 0, "There needs to be at least one component, "
			"if you want a list of all entities, call getEntities()");

		//TODO: Fix naming
		using EntityComponents = EntityComponentDependency<Cmps...>;
		std::vector<EntityComponents> available_entities;

		//TODO: Rip cache coherency
		for (Entity entity : m_entities)
		{
			if (HasAllCmps<Cmps...>(entity))
			{
				EntityComponents entity_components{ entity };
				available_entities.push_back(entity_components);
			}
		}

		//auto &first_pool = getPool<Cmps>();
		//size_t amount_of_components = first_pool.size();
		//std::vector<size_t> available_entities{}; //TODO: call the correct constructor to make it one line.
		//available_entities.reserve(amount_of_components);
		//for (int i = 0; i < first_pool.size(); ++i)
		//{
		//	if (first_pool.active(i))
		//	{
		//		size_t id = first_pool.get(i).entity_id;
		//		available_entities.push_back(id);
		//	}
		//}
		//
		///// We already have the full list of the first component
		//for (int i = 1; i < group.size; ++i)
		//{
		//	CmpPool<void*> &pool = getPool<Cmps>()...;
		//	std::vector<size_t> pool_ids;
		//	pool_ids.reserve(pool.size());
		//
		//	for (int j = 0; j < pool.size(); ++j)
		//	{
		//		//TODO: How can I make pool.GetNInTypelist(i) cast into a void* ComponentElement
		//		ComponentElement<void*>& comp_element = pool.get(i);
		//
		//		//TODO: Add all entities to an array
		//		pool_ids.push_back(comp_element.entity_id);
		//	}
		//
		//	/// Delete all elements not present in both arrays
		//	std::vector<size_t> restricted_entities{};
		//	for (int j = 0; j < pool_ids.size(); ++j)
		//	{
		//		size_t id = pool_ids[j];
		//		auto it = std::find(available_entities.begin(), available_entities.end(), id);
		//		if (it != available_entities.end())
		//			restricted_entities.push_back(id);
		//	}
		//	available_entities = restricted_entities;
		//}
		//
		//for (int i = 0; i < available_entities.size(); ++i)
		//{
		//	EntityComponents ent{Entity{available_entities[i], *this}};
		//
		//	ret.push_back(ent);
		//}

		return available_entities;
	}

	//TODO: Create an overload for removing a component by reference, for when a entity has multiple components of the same type
}
