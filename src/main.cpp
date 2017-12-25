#include "Concordia/EntityMgr.hpp"
#include "Concordia/SystemMgr.hpp"
#include "Concordia/EventMgr.hpp"
#include <iostream>
#include <string>

using namespace Concordia;

struct SystemEvent {};

struct SystemEventsReceiver : Receiver {
  SystemEventsReceiver(EventMgr &ev) { ev.subscribe<SystemEvent>(*this); }

  void receive(const SystemEvent &e) {
    std::cout << "SystemEventsReceiver - Received event from systems!\n";
  }
};

struct GameObjectCmp : Cmp<GameObjectCmp> {
  GameObjectCmp() {}
  GameObjectCmp(const std::string &name, const std::string &tag)
      : name(name), tag(tag) {}
  std::string name;
  std::string tag;
};

struct TransformCmp : Cmp<TransformCmp> {
  TransformCmp() {}
  TransformCmp(float pX, float pY, float sX, float sY, float rot)
      : pX(pX), pY(pY), sX(sX), sY(sY), rot(rot) {}
  float pX = 0, pY = 0;
  float sX = 0, sY = 0;
  float rot = 0;
};

struct GameObjectSys : public System<GameObjectSys> {
  void init(EntityMgr &es) {
    std::cout << "INIT\n";

	auto entities = es.getEntitiesWith<GameObjectCmp>();
	  for (auto components : entities)
	  {
		  auto& goc = components.get<GameObjectCmp>();
		  goc.name = "playerMod";
		  goc.tag = "testMod";

		  std::cout << goc.name << ", " << goc.tag << "\n";
	  }

    /*for (auto &e : es.entities()) {
      if (e.hasComponent<GameObjectCmp>()) {
        auto &goc = e.getComponent<GameObjectCmp>();
        goc.name = "playerMod";
        goc.tag = "testMod";

        std::cout << goc.name << ", " << goc.tag << "\n";
      }
    }*/

    std::cout << "\n";
  }

  void update(EntityMgr &es, float dt) {
    std::cout << "UPDATE\n";

	auto entities = es.getEntitiesWith<GameObjectCmp, TransformCmp>();
	  for (auto components : entities)
	  {
		  auto &goc = components.get<GameObjectCmp>();
		  auto &trx = components.get<TransformCmp>();

		  std::cout << trx.pY << "\n";
		  std::cout << goc.name << ", " << goc.tag << "\n";

		  goc.name = "player";
		  goc.tag = "test";

		  trx.pY += 1;
	  }

    /*for (auto &e : es.entities()) {
      if (e.hasComponent<GameObjectCmp>() && e.hasComponent<TransformCmp>()) {
        auto &goc = e.getComponent<GameObjectCmp>();
        auto &trx = e.getComponent<TransformCmp>();

        std::cout << trx.pY << "\n";
        std::cout << goc.name << ", " << goc.tag << "\n";

        goc.name = "player";
        goc.tag = "test";

        trx.pY += 1;
      }
    }*/

    std::cout << "\n";
  }

  void render(EntityMgr &es) { std::cout << "RENDER\n\n"; }

  void clean(EntityMgr &es) {
    getEventMgr().broadcast<SystemEvent>();
    std::cout << "CLEAN\n\n";
  }
};

EventMgr eventMgr;
EntityMgr entityMgr;
SystemMgr systemMgr(entityMgr, eventMgr);

SystemEventsReceiver receiver(eventMgr);

bool running = true;

void run() {
  float dt = 1.f / 60.f;

  size_t ticks = 0;

  systemMgr.init();
  while (running) {
    systemMgr.update(dt);
    systemMgr.render();

    ticks++;
    if (ticks >= 10)
      running = false;
  }
  systemMgr.clean();
}

int main(int argc, char **args) {
  systemMgr.addSys<GameObjectSys>();

  for (uint i = 0; i < 10; ++i) {
    Entity e = entityMgr.createEntity();
    e.addComponent<GameObjectCmp>("player", "test");
    e.addComponent<TransformCmp>();
    entityMgr.addEntity(e);
  }

  run();

  std::cin.get();
  return 0;
}
