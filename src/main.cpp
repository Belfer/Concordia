#include "Concordia/EntityMgr.hpp"
#include "Concordia/SystemMgr.hpp"
#include "Concordia/EventMgr.hpp"
#include "Concordia/CommonTypes.hpp"
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

struct GameObjectCmp : ICmp<GameObjectCmp> {
  GameObjectCmp() {}
  GameObjectCmp(const std::string &name, const std::string &tag)
      : name(name), tag(tag) {}
  std::string name;
  std::string tag;
};

struct TransformCmp : ICmp<TransformCmp> {
  TransformCmp() {}
  TransformCmp(float pX, float pY, float sX, float sY, float rot)
      : pX(pX), pY(pY), sX(sX), sY(sY), rot(rot) {}
  float pX = 0, pY = 0;
  float sX = 0, sY = 0;
  float rot = 0;
};

enum class ColliderType
{
	BOX,
	CIRCLE
};

struct ColliderCmp
{
	const ColliderType type;

	ColliderCmp(ColliderType type) : type(type){}
	//virtual ~ColliderCmp(){};
};

struct BoxCollider : ColliderCmp
{
	BoxCollider() : ColliderCmp(ColliderType::BOX)
	{
	}
};

struct CircleCollider : ColliderCmp
{
	CircleCollider() : ColliderCmp(ColliderType::CIRCLE)
	{
	}
};

struct GameObjectSys : public System<GameObjectSys> {
  void init(EntityMgr &es) override
  {
    std::cout << "INIT\n";

	auto entities = es.getEntitiesWith<GameObjectCmp>();
	  for (auto components : entities)
	  {
		  GameObjectCmp& goc = components.get<GameObjectCmp>();
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

  void update(EntityMgr &es, float dt) override
  {
    std::cout << "UPDATE\n";

	auto entities = es.getEntitiesWith<GameObjectCmp, TransformCmp>();
	  for (auto& components : entities)
	  {
		  auto& tr = components.get<TransformCmp>();
		  auto& obj = components.get<GameObjectCmp>();
		  
		  std::cout << tr.pY << "\n";
		  std::cout << obj.name << ", " << obj.tag << "\n";

		  obj.name = "player";
		  obj.tag = "test";

		  tr.pY += 1;
	  }

    std::cout << "\n";
  }

  void render(EntityMgr &es) override { std::cout << "RENDER\n\n"; }

  void clean(EntityMgr &es) override
  {
    getEventMgr().broadcast<SystemEvent>();
    std::cout << "CLEAN\n\n";
  }
};

struct PhysicsSystem : System<PhysicsSystem>
{
	void init(EntityMgr& es) override{}
	void update(EntityMgr& es, float dt) override
	{
		auto entities = es.getEntitiesWith<GameObjectCmp, TransformCmp>();
		for (auto& components : entities)
		{
			ColliderCmp* collider = components.entity.getAnyComponent<ColliderCmp, BoxCollider, CircleCollider>();
			if(collider)
			{
				std::cout << "Collider type: " << (int)collider->type << '\n';
			}
		}
	}
	void render(EntityMgr& es) override{}
	void clean(EntityMgr& es) override{}
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
  systemMgr.addSys<PhysicsSystem>();

  for (uint i = 0; i < 10; ++i) {
    Entity e = entityMgr.createEntity();
    e.addComponent<GameObjectCmp>("player", std::string( "test" ) + std::to_string(i));
    e.addComponent<TransformCmp>();
    entityMgr.addEntity(e);
  }

  Entity e_box = entityMgr.createEntity();
  e_box.addComponent<GameObjectCmp>("wall", "box_col");
  e_box.addComponent<TransformCmp>();
  e_box.addComponent<BoxCollider>();
  entityMgr.addEntity(e_box);

  Entity e_box2 = entityMgr.createEntity();
  e_box2.addComponent<GameObjectCmp>("wall2", "box_col");
  e_box2.addComponent<TransformCmp>();
  e_box2.addComponent<BoxCollider>();
  entityMgr.addEntity(e_box2);

  Entity e_cir = entityMgr.createEntity();
  e_cir.addComponent<GameObjectCmp>("ball", "circle_col");
  e_cir.addComponent<TransformCmp>();
  e_cir.addComponent<CircleCollider>();
  entityMgr.addEntity(e_cir);

  Entity e_cir2 = entityMgr.createEntity();
  e_cir2.addComponent<GameObjectCmp>("ball", "circle_col");
  e_cir2.addComponent<TransformCmp>();
  e_cir2.addComponent<CircleCollider>();
  entityMgr.addEntity(e_cir2);

  run();

  std::cin.get();
  return 0;
}
