#include "Concordia/EntityMgr.hpp"
#include "Concordia/SystemMgr.hpp"
#include "Concordia/EventMgr.hpp"
#include "Concordia/CommonTypes.hpp"
#include <iostream>
#include <string>

using namespace Concordia;

struct vec3
{
	float x,y,z;
};

struct Transform
{
	vec3 position;
	vec3 scale;
	vec3 rot;
};

struct Player
{
	int health = 100;
};

struct Camera
{
	float fov = 90.0f;
};


EventMgr eventMgr;
EntityMgr entityMgr(eventMgr);
SystemMgr systemMgr(entityMgr, eventMgr);

class PlayerController : public ISystem
{
	void init(EntityMgr& es) override
	{
	}

	void update(EntityMgr& es, float dt) override
	{
		auto entities = es.getEntitiesWith<Transform, Player>();
		for (auto& components : entities)
		{
			auto& transform = components.get<Transform>();
			if(/* KeyPressed(KEY_W) */true){
				transform.position.z += 1;
			}
		}
	}

	void render(EntityMgr& es) override
	{
		auto camera = es.getFirstEntityWith<Transform, Camera>();
		if(camera)
			std::cout << "We are rendering with a camera of: " << camera->get<Camera>().fov << " FOV\n";
		else
			std::cout << "There is no camera to render with!\n";		

		auto entities = es.getEntitiesWith<Transform, Player>();
		for (auto& components : entities)
		{
			auto& transform = components.get<Transform>();
			std::cout << "Y: " << transform.position.z << '\n'; 
		}
	}
	
	void clean(EntityMgr& es) override
	{
	}
};


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


int main(){
	Entity entity = entityMgr.createEntity();
	entity.addComponent<Transform>();
	entity.addComponent<Player>();
	entityMgr.addEntity(entity);

	Entity camera_entity = entityMgr.createEntity();
	camera_entity.addComponent<Transform>();
	camera_entity.addComponent<Camera>();
	entityMgr.addEntity(camera_entity);

	systemMgr.addSys<PlayerController>();

	run();

	std::cin.get();
	return 0;
}