#pragma once

#include "Pool.hpp"
#include <assert.h>
#include <memory>
#include <stdlib.h>
#include <unordered_map>
#include <utility>
#include <vector>

/**
 * @brief Entities
 */

struct ICmp {
protected:
  static size_t regId() {
    static size_t s_cmpCounter = -1;
    return ++s_cmpCounter;
  }
};

template <typename E> struct Cmp : public ICmp {
  static size_t id() {
    static const size_t s_id = regId();
    return s_id;
  }
};

class EntityMgr;

struct Entity {
private:
  friend class EntityMgr;
  Entity(size_t id, EntityMgr &entityMgr) : m_id(id), m_entityMgr(entityMgr) {}

public:
  template <typename C> inline void addComponent(const C &cmp) const;

  template <typename C, typename... Args>
  inline void addComponent(Args... args) const;

  template <typename C> inline bool hasComponent() const;

  template <typename C> inline C &getComponent() const;

  template <typename C> inline void removeComponent() const;

  inline size_t id() const { return m_id; }

private:
  size_t m_id;
  EntityMgr &m_entityMgr;
  std::vector<size_t> m_cmps;
};

using Entities = std::vector<Entity>;

class EntityMgr {
private:
  template <typename C> using CmpPool = Pool<std::pair<size_t, C>>;
  using CmpMap = std::unordered_map<size_t, IPool *>;

  template <typename C> inline CmpPool<C> &getPool() {
    auto handle = static_cast<CmpPool<C> *>(m_cmpMap[Cmp<C>::id()]);
    if (handle == nullptr) {
      handle = new CmpPool<C>();
      m_cmpMap[Cmp<C>::id()] = handle;
    }
    return *handle;
  }

public:
  Entity createEntity() {
    static size_t s_entityCounter = -1;
    return Entity(++s_entityCounter, *this);
  }

  void addEntity(Entity e) { m_entities.emplace_back(e); }

  void removeEntity(Entity e) {}

  template <typename C> void addComponent(Entity e, const C &cmp) {
    auto &poolHandle = getPool<C>();
    poolHandle.add(std::make_pair(e.id(), cmp));
  }

  template <typename C, typename... Args>
  void addComponent(Entity e, Args... args) {
    auto &poolHandle = getPool<C>();
    poolHandle.add(std::make_pair(e.id(), C(args...)));
  }

  template <typename C> bool hasComponent(Entity e) {
    auto &poolHandle = getPool<C>();
    for (auto data : poolHandle.data()) {
      if (data.first && data.second.first == e.id()) {
        return true;
      }
    }
    return false;
  }

  template <typename C> C &getComponent(Entity e) {
    auto &poolHandle = getPool<C>();
    for (auto &data : poolHandle.data()) {
      if (data.first && data.second.first == e.id()) {
        return data.second.second;
      }
    }
    assert(false && "Entity doesn't have component!");
  }

  template <typename C> void removeComponent(Entity e) {
    auto poolHandle = getPool<C>();
    for (auto data : poolHandle.data()) {
      if (data.first == e.id()) {
        poolHandle.remove(data);
      }
    }
  }

  inline Entities &entities() { return m_entities; }

private:
  CmpMap m_cmpMap;
  Entities m_entities;
};

template <typename C> inline void Entity::addComponent(const C &cmp) const {
  m_entityMgr.addComponent(id(), cmp);
}

template <typename C, typename... Args>
inline void Entity::addComponent(Args... args) const {
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
