#pragma once
namespace Concordia
{
	class EventMgr;
	class EntityMgr;

	//IDID: Make this a seperate file so classes that implement this can only include this skeleton
	//IDID: Make this a class, it is clearly not a data structure
	class ISystem {
	public:
		virtual ~ISystem(){};
		virtual void init(EntityMgr &es) = 0;
		virtual void update(EntityMgr &es, float dt) = 0;
		virtual void render(EntityMgr &es) = 0;
		virtual void clean(EntityMgr &es) = 0;

	protected:
		//TODO: Fix
		static size_t regId() {
			static size_t s_sysCounter = -1;
			return ++s_sysCounter;
		}


		inline EventMgr &getEventMgr() { return *m_eventMgr; }

	private:
		friend class SystemMgr;
		EventMgr* m_eventMgr;
	};
}