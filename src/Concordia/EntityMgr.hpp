#pragma once

//#include "Pool.hpp"
#include "ComponentPool.hpp"
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
	template <typename E> struct ICmp {
		//Maybe someone wants to add some member functions, however I like having PODs as Components
	};

	class EntityMgr;
	template<typename... Cmps >
	struct EntityComponentDependency;

	struct Entity {
	private:
		friend class EntityMgr;
		Entity(size_t id, EntityMgr &entityMgr) : m_id(id), m_entityMgr(entityMgr) {}

		//TODO: The entity class does not have a assignment operator I think

	public:
		template <typename C> inline void addComponent(const C &cmp);

		template <typename C, typename... Args>
		inline void addComponent(Args... args);

		template <typename C> inline bool hasComponent() const;

		template <typename C> inline C &getComponent() const;

		template<typename CBase, typename... Cs> CBase* getAnyComponent() const;

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

	template<typename ... Cmps>
	constexpr std::array<void*, sizeof...(Cmps)> GetAllCmps(Entity entity)
	{
		return { static_cast<void*>(&entity.getComponent<Cmps>())... };
	}

	//TODO: Allow structured bindings for this class
	///A entity which is guaranteed to have these components, for easy use in functions
	template<typename... Cmps>
	struct EntityComponentDependency : public ComponentGroup<Cmps...>
	{
		using Super = ComponentGroup<Cmps...>;
		//TODO: Is there a better way to do templated inheritance?
		using Super::size;
		using Super::ids;
		using Super::cmps;

		Entity entity;

		EntityComponentDependency(Entity entity)
			: ComponentGroup<Cmps...>(entity.getComponent<Cmps>()...), entity(entity)
		{
		}

		EntityComponentDependency(Entity entity, Cmps... cmps)
			: ComponentGroup<Cmps...>(std::forward<Cmps>(cmps)...), entity(entity)
		{}
	};
}

namespace std
{
	template<typename... Types>
	struct tuple_size<Concordia::EntityComponentDependency<Types...>> : public integral_constant<size_t, sizeof...(Types)> {};

	template<std::size_t N, typename... Types>
	struct tuple_element<N, Concordia::EntityComponentDependency<Types...>> {
		//using type = decltype( std::declval<Concordia::ComponentGroup<Types...>>().template get<N>());
		using type = Concordia::Impl::nth_type_in_pack<N, Types...>;
	};
}

namespace Concordia{

	class EntityMgr {
	private:
		template<typename C>
		struct ComponentElement
		{
			size_t entity_id;
			C component;
		};

		/*template <typename C> 
		using CmpPool = Pool<ComponentElement<C>>;*/

		template <typename C> 
		inline CmpPool<C> &getPool() {
			auto it = m_cmpMap.find(get_id<C>());
			if(it == m_cmpMap.end())
			{
				auto* pool = new CmpPool<C>{};
				m_cmpMap.insert(it, { get_id<C>(), pool });
				return *pool;
			}

			auto* pool = it->second;

			return * static_cast<CmpPool<C>*>(pool);

			//auto* handle = static_cast<CmpPool<C>*>(m_cmpMap.at(get_id<C>()));
			//if (handle == nullptr) {
			//	handle = new CmpPool<C>{};
			//	m_cmpMap[get_id<C>()] = handle;
			//}
			//return *handle;
		}

		inline ICmpPool* getPool(size_t cmp_id) {
			auto it = m_cmpMap.find(cmp_id);
			if(it != m_cmpMap.end())
			{
				return it->second;
			}
			return nullptr;
		}

	public:
		Entity createEntity() {
			// We begin counting at 1. So 0 can be used as a inactive flag
			static size_t s_entityCounter = 0;
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

		/*template <typename C> void addComponent(Entity e, const C &cmp) {
			auto &poolHandle = getPool<C>();
			poolHandle.add({ e.id(), cmp });
		}*/

		template <typename C, typename... Args>
		void addComponent(Entity e, Args... args) {
			CmpPool<C>& poolHandle = getPool<C>();
			poolHandle.addComponent(e.id(), C{std::forward<Args>(args)...} );
		}

		template <typename C> bool hasComponent(Entity e) {
			CmpPool<C>& poolHandle = getPool<C>();
			return poolHandle.hasComponent(e.id());
		}

		bool hasComponent(Entity e, size_t cmp_id) {
			auto* pool = getPool(cmp_id);
			if (!pool)
				return false;
			return pool->hasComponent(e.id());
		}

		//TODO: return a pointer and don't throw an error
		template <typename C> C &getComponent(Entity e) {
			CmpPool<C>& poolHandle = getPool<C>();
			auto sz = poolHandle.size();
			for (int i = 0; i < poolHandle.size(); ++i)
			{
				if(poolHandle.entity_ids[i] == e.id()){
					C* cmp = poolHandle.getComponentPtr(i);
					assert(cmp != nullptr);
					return *cmp;
				}
			}
			EmptyFunction();
			assert(false && "Entity doesn't have component!");
			throw std::exception("cannot find component"); 
		}



		/// A temporairy function for returning any of Cs... components, casted into CBase.
		/// This class is temporairy because I should make a system of generating inheritance graphs
		template<typename CBase, typename... Cs> 
		CBase* getAnyComponent(Entity e)
		{
			//MAYBE: Add a check to see if CBase is the base of all Cs...
			using CmpGroup = ComponentGroup<Cs...>;
			for (int i = 0; i < CmpGroup::size; ++i)
			{
				if(hasComponent(e, CmpGroup::ids[i]))
				{
					ICmpPool* pool = getPool(CmpGroup::ids[i]);
					return reinterpret_cast<CBase*>(pool->getRawComponent(e.id()));
				}
			}

			return nullptr;
		}

		template <typename C> void removeComponent(Entity e) {
			CmpPool<C>& poolHandle = getPool<C>();
			poolHandle.removeComponent(e.id());
		}

		inline std::vector<Entity>& entities() { return m_entities; }

		template<typename... Cmps>
		std::vector<EntityComponentDependency<Cmps...>> getEntitiesWith();

	private:
		std::unordered_map<size_t, ICmpPool*> m_cmpMap;
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

	template <typename CBase, typename ... Cs>
	CBase* Entity::getAnyComponent() const
	{
		return m_entityMgr.getAnyComponent<CBase, Cs...>(*this);
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
	std::vector<EntityComponentDependency<Cmps...>> EntityMgr::getEntitiesWith()
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

#pragma region first attempt
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
		//		ComponentElement<void*>& comp_element = pool.get(i);
		//
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
#pragma endregion 

		return available_entities;
	}

	//TODO: Create an overload for removing a component by reference, for when a entity has multiple components of the same type
}
