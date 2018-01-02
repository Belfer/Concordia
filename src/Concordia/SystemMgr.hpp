#pragma once

#include <assert.h>
#include <memory>
#include <stdlib.h>
#include <unordered_map>
#include <utility>
#include <vector>
#include "ISystem.hpp"

namespace Concordia {
	/**
	 * @brief Systems
	 */

	class EventMgr;

	//TODO: Phase this class out
	template <typename E> class System : public ISystem {
		/*static size_t element_size() {
			return get_id<E>();
		}*/

	};

	//TODO: Consider not using a type alias for readability
	//using Systems = std::vector<ISystem *>;

	class SystemMgr {
	public:
		SystemMgr(EntityMgr &entityMgr, EventMgr &eventMgr)
			: m_entityMgr(entityMgr), m_eventMgr(eventMgr) {}

		template <typename S, typename... Args> void addSys(Args... args) {
			static_assert(std::is_base_of<ISystem, S>::value, "The system class needs to inherit from ISystem!");

			S* sys = new S{ std::forward<Args>(args)... };
			sys->m_eventMgr = &m_eventMgr;
			m_systems.push_back(sys);
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
		EntityMgr & m_entityMgr;
		EventMgr &m_eventMgr;
		std::vector<ISystem*> m_systems;
	};
}
