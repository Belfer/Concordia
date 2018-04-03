#pragma once
#include "EntityMgr.hpp"
#include "EventMgr.hpp"
#include "SystemMgr.hpp"

namespace Concordia {
	class World
	{
		EntityMgr entity_mgr;
		EventMgr event_mgr;
		SystemMgr system_mgr;

	public:
		World()
			: entity_mgr(), event_mgr(), system_mgr(entity_mgr, event_mgr)
		{
		}

		~World()
		{
			system_mgr.clean();
		}

		void Update()
		{
			system_mgr.update(10);
			system_mgr.render();
		}


		Entity CreateEntity()
		{
			return entity_mgr.createEntity();
		}
	};
}