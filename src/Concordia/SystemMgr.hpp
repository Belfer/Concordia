#pragma once

#include <cassert>
#include <memory>
#include <cstdlib>
#include <unordered_map>
#include <utility>
#include <vector>
#include "ISystem.hpp"
#include "MetaDebugging.hpp"

namespace Concordia {
	/**
	 * @brief Systems
	 */

	class EventMgr;

	template <typename E> class System : public ISystem {
		
	};

	class SystemMgr {
	public:
		SystemMgr(EntityMgr &entityMgr, EventMgr &eventMgr)
			: m_entityMgr(entityMgr), m_eventMgr(eventMgr) {}

		template <typename S, typename... Args> void addSys(Args... args) {
			static_assert(is_complete<S>::value, "The System is an incomplete type! Check if it is not a forward declaration.");
			static_assert(std::is_base_of<ISystem, S>::value, "The system class needs to inherit from ISystem!");

			S* sys = new S{ std::forward<Args>(args)... };
			sys->m_eventMgr = &m_eventMgr;
			m_systems.push_back(sys);
		}

		template <typename Sys> void removeSys() {
			// TODO: remove system
		}

		//TODO: Create method for removing system by pointer/ref

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
