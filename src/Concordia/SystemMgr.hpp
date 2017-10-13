#pragma once

#include <assert.h>
#include <memory>
#include <stdlib.h>
#include <unordered_map>
#include <utility>
#include <vector>

/**
 * @brief Systems
 */

class EventMgr;

struct ISys {
  virtual void init(EntityMgr &es) = 0;
  virtual void update(EntityMgr &es, float dt) = 0;
  virtual void render(EntityMgr &es) = 0;
  virtual void clean(EntityMgr &es) = 0;

protected:
  static size_t regId() {
    static size_t s_sysCounter = -1;
    return ++s_sysCounter;
  }
};

template <typename E> struct Sys : public ISys {
  static size_t id() {
    static const size_t s_id = regId();
    return s_id;
  }

  inline EventMgr &getEventMgr() { return *m_eventMgr; }

private:
  friend class SystemMgr;
  EventMgr *m_eventMgr;
};

using Systems = std::vector<ISys *>;

class SystemMgr {
public:
  SystemMgr(EntityMgr &entityMgr, EventMgr &eventMgr)
      : m_entityMgr(entityMgr), m_eventMgr(eventMgr) {}

  template <typename S, typename... Args> void addSys(Args... args) {
    auto sys = new S(std::forward<Args>(args)...);
    sys->m_eventMgr = &m_eventMgr;
    m_systems.emplace_back(sys);
  }

  template <typename Sys> void removeSys() {
    // TODO
  }

  inline void init() {
    for (auto &sys : m_systems) {
      sys->init(m_entityMgr);
    }
  }

  inline void update(float dt) {
    for (auto &sys : m_systems) {
      sys->update(m_entityMgr, dt);
    }
  }

  inline void render() {
    for (auto &sys : m_systems) {
      sys->render(m_entityMgr);
    }
  }

  inline void clean() {
    for (auto &sys : m_systems) {
      sys->clean(m_entityMgr);
    }
  }

private:
  EntityMgr &m_entityMgr;
  EventMgr &m_eventMgr;
  Systems m_systems;
};
