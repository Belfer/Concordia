#pragma once

#include "ComponentPool.hpp"
#include <cassert>
#include <memory>
#include <cstdlib>
#include <unordered_map>
#include <utility>
#include <vector>
#include "ComponentGroup.hpp"
#include "Identification.hpp"
#include "MetaUtils.hpp"
#include "Pool.hpp"

namespace Concordia {
	template <typename E> struct ICmp {
		//Maybe someone wants to add some member functions, however I like having PODs as Components
	};

	class EntityMgr;
	template<typename... Cmps >
	struct EntityWith;

	struct Entity {
	private:
		friend class EntityMgr;
		Entity(size_t id, EntityMgr &entityMgr) : m_id(id), m_entityMgr(entityMgr) {}

		//TODO: The entity class does not have a assignment operator I think

	public:
		template <typename C>
		C& addComponent(const C &cmp);

		template <typename C, typename... Args>
		C& addComponent(Args... args);

		template <typename C>
		bool hasComponent() const;

		template <typename C>
		C* getComponent() const;

		template<typename CBase, typename... Cs> CBase* getAnyComponent() const;

		template <typename C>
		void removeComponent() const;

		template<typename... Cmps>
		std::vector<EntityWith<Cmps...>> EntitiesWith() const;


		size_t id() const { return m_id; }

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
	std::array<void*, sizeof...(Cmps)> GetAllCmps(Entity entity)
	{
		return { static_cast<void*>(entity.getComponent<Cmps>())... };
	}

	/// A struct which stores a group of components (Like ComponentGroup) but
	/// also stores an Entity. Allowing for an entity with these components to be easily
	/// passed around
	template<typename... Cmps>
	struct EntityWith : public ComponentGroup<Cmps...>
	{
		using Super = ComponentGroup<Cmps...>;
		using Super::size;
		using Super::ids;
		using Super::cmps;

		Entity entity;

		EntityWith(Entity entity)
			: ComponentGroup<Cmps...>(*entity.getComponent<Cmps>()...), entity(entity)
		{
		}

		EntityWith(Entity entity, Cmps&... cmps)
			: ComponentGroup<Cmps...>(std::forward<Cmps>(cmps)...), entity(entity)
		{}
	};
}

namespace std
{
	///Support for C++17's structured bindings for the EntityWith class

	template<typename... Types>
	struct tuple_size<Concordia::EntityWith<Types...>> : public integral_constant<size_t, sizeof...(Types)> {};

	template<std::size_t N, typename... Types>
	struct tuple_element<N, Concordia::EntityWith<Types...>> {
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

#ifdef _DEBUG
	public:
#endif
		template <typename C>
		Pool<C> &getPool() {
			auto it = m_cmpMap.find(get_cmp_id<C>());
			if(it == m_cmpMap.end())
			{
				auto* pool = new Pool<C>{};
				m_cmpMap.insert(it, { get_cmp_id<C>(), pool });
				return *pool;
			}

			auto* pool = it->second;

			return * static_cast<Pool<C>*>(pool);
		}

		IPool* getPool(size_t cmp_id) {
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

			throw std::runtime_error("Cannot find an entity with the id");
		}

		/*template <typename C> void addComponent(Entity e, const C &cmp) {
			auto &poolHandle = getPool<C>();
			poolHandle.add({ e.id(), cmp });
		}*/

		template <typename C, typename... Args>
		C& addComponent(Entity e, Args... args) {
			Pool<C>& poolHandle = getPool<C>();
			poolHandle.add(e.id(), C{std::forward<Args>(args)...} );
			return *poolHandle.getById(e.id());
		}

		template <typename C> bool hasComponent(Entity e) {
			Pool<C>& poolHandle = getPool<C>();
			return poolHandle.containsId(e.id());
		}

		template <typename C> bool hasComponent(size_t e_id) {
			Pool<C>& poolHandle = getPool<C>();
			return poolHandle.containsId(e_id);
		}

		bool hasComponent(Entity e, size_t cmp_id) {
			return hasComponent(e.id(), cmp_id);
		}

		bool hasComponent(size_t e_id, size_t cmp_id) {
			auto* pool = getPool(cmp_id);
			if (!pool)
				return false;
			return pool->containsId(e_id);
		}

		template <typename C> C* getComponent(Entity e) {
			Pool<C>& poolHandle = getPool<C>();
			return poolHandle.getById(e.id());
		}

		template <typename C> C* getComponent(size_t e_id) {
			Pool<C>& poolHandle = getPool<C>();
			return poolHandle.getById(e_id);
		}


	private:
		template<typename C, typename... Cs>
		struct getAnyComponentImpl{

			void* value;

			explicit getAnyComponentImpl(Entity& e) {
				//TODO: Remove after debugging
				auto id = get_cmp_id<C>();
				if (e.hasComponent<C>())
					value = e.getComponent<C>();
				else
					value = getAnyComponentImpl<Cs...>(e).value;
			}
		};

		template<typename C>
		struct getAnyComponentImpl<C>
		{
			void* value;

			explicit getAnyComponentImpl(Entity& e) {
				if (e.hasComponent<C>())
					value = e.getComponent<C>();
				else
					value = nullptr;
			}
		};
	public:

		/// A temporairy function for returning any of Cs... components, casted into CBase.
		/// This class is temporairy because I should make a system of generating inheritance graphs
		template<typename CBase, typename... Cs> 
		CBase* getAnyComponent(Entity e)
		{
			//MAYBE: Add a check to see if CBase is the base of all Cs...
			
			return static_cast<CBase*>(getAnyComponentImpl<Cs...>(e).value);
		}

		template <typename C> void removeComponent(Entity e) {
			Pool<C>& poolHandle = getPool<C>();
			poolHandle.removeComponent(e.id());
		}

		std::vector<Entity>& entities() { return m_entities; }

		template<typename... Cmps>
		std::vector<EntityWith<Cmps...>> getEntitiesWith();
		
		template<typename... Cmps>
		std::unique_ptr<EntityWith<Cmps...>> getFirstEntityWith();

	private:
		std::unordered_map<size_t, IPool*> m_cmpMap;
		std::vector<Entity> m_entities;
	};

	template <typename C>
	C& Entity::addComponent(const C &cmp) {
		return m_entityMgr.addComponent<C>(*this, cmp);
	}

	template <typename C, typename... Args>
	C& Entity::addComponent(Args... args) {
		return m_entityMgr.addComponent<C>(*this, args...);
	}

	template <typename C>
	bool Entity::hasComponent() const {
		return m_entityMgr.hasComponent<C>(*this);
	}

	template <typename C>
	C* Entity::getComponent() const {
		return m_entityMgr.getComponent<C>(*this);
	}

	template <typename CBase, typename ... Cs>
	CBase* Entity::getAnyComponent() const
	{
		return m_entityMgr.getAnyComponent<CBase, Cs...>(*this);
	}

	template <typename C>
	void Entity::removeComponent() const {
		m_entityMgr.removeComponent<C>();
	}

	template <typename... Cmps>
	std::vector<EntityWith<Cmps...>> Entity::EntitiesWith() const
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
	std::vector<EntityWith<Cmps...>> EntityMgr::getEntitiesWith()
	{
		static_assert(sizeof...(Cmps) > 0, "There needs to be at least one component, "
			"if you want a list of all entities, call getEntities()");

		using EntityComponents = EntityWith<Cmps...>;
		std::vector<EntityWith<Cmps...>> available_entities;

		//TODO: Rip cache coherency
		for (Entity entity : m_entities)
		{
			if (HasAllCmps<Cmps...>(entity))
			{
				EntityWith<Cmps...> entity_components{entity};
				available_entities.push_back(entity_components);
			}
		}

		return available_entities;
	}

	template <typename ... Cmps>
	std::unique_ptr<EntityWith<Cmps...>> EntityMgr::getFirstEntityWith()
	{
		static_assert(sizeof...(Cmps) > 0, "There needs to be at least one component, "
			"if you want a list of all entities, call getEntities()");

		using EntityComponents = EntityWith<Cmps...>;

		//TODO: Rip cache coherency
		for (Entity entity : m_entities)
		{
			if (HasAllCmps<Cmps...>(entity))
			{
				return std::make_unique<EntityWith<Cmps...>>(entity);
			}
		}

		std::unique_ptr<EntityWith<Cmps...>> null_entity{nullptr};
		return std::move(null_entity);
	}

	//TODO: Create an overload for removing a component by reference, for when a entity has multiple components of the same type
}
