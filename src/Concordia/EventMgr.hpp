#pragma once

#include "NonCopyable.hpp"
#include <assert.h>
#include <memory>
#include <stdlib.h>
#include <unordered_map>
#include <utility>
#include <vector>
#include <functional>
#include <queue>
#include <mutex>

namespace Concordia {
	/**
	 * @brief Events
	 */

	struct ISignal {
		virtual ~ISignal() {}
		virtual void operator()(const void *p) = 0;

	protected:
		static size_t regId() {
			static size_t counter = -1;
			return ++counter;
		}
	};

	template <typename E> struct Signal : public ISignal {
		explicit Signal(std::function<void(const E &)> sigFn) : m_sigFn(sigFn) {}
		virtual void operator()(const void *p) {
			m_sigFn(*(static_cast<const E *>(p)));
		}

		static size_t id() {
			static const size_t id = regId();
			return id;
		}

	private:
		std::function<void(const E &)> m_sigFn;
	};

	using SignalHandle = std::pair<size_t, size_t>;

	struct Receiver {
		~Receiver() {
			if (m_sigHandles.size() > 0)
				m_clearSigFn(m_sigHandles);
		}

	private:
		friend class EventMgr;
		std::function<void(std::vector<SignalHandle> &)> m_clearSigFn;
		std::vector<SignalHandle> m_sigHandles;
	};
	
	class EventMgr;
	
	struct IEventData
	{
		virtual ~IEventData() = default;
		
		virtual void invoke(EventMgr&) = 0;
	};
	
	template<typename T>
	struct EventData : IEventData
	{
		T args;
		
		EventData(T args)
			: args(args)
		{
		}
		
		void invoke(EventMgr& evt) override;
	};
	
	class EventMgr : NonCopyable {
	public:
		using SignalPtr = std::shared_ptr<ISignal>;

		EventMgr() {}
		~EventMgr() {
			// TODO: does RAII clean up everything already?
		}

		template <typename E, typename Receiver> void subscribe(Receiver &receiver) {
			if (receiver.m_sigHandles.size() == 0) {
				receiver.m_clearSigFn =
					std::bind(&EventMgr::clearSignals, this, std::placeholders::_1);
			}

			void (Receiver::*receive)(const E &) = &Receiver::receive;
			auto* signal =
				new Signal<E>(std::bind(receive, &receiver, std::placeholders::_1));

			auto &sigSlots = slotsFor(Signal<E>::id());
			sigSlots.second[sigSlots.first] = SignalPtr(signal);

			receiver.m_sigHandles.emplace_back(
				std::make_pair(Signal<E>::id(), sigSlots.first));

			++sigSlots.first;
		}

		template <typename E, typename Receiver>
		void unsubscribe(Receiver &receiver) {
			auto &sigSlots = slotsFor(Signal<E>::id());
			for (auto handle : receiver.m_sigHandles) {
				if (handle.first == Signal<E>::id())
					sigSlots.second.erase(handle.second);
			}
		}

		template <typename E, typename... Args> void broadcast(Args... args) {
            broadcast(E( args... ));
		}

		template <typename E> void broadcast(const E &event) {
//			auto &sigSlots = slotsFor(Signal<E>::id());
//			for (auto sig : sigSlots.second) {
//				(*sig.second)(static_cast<const void *>(&event));
//			}
			event_queue_mutex.lock();
			auto* eventData = new EventData<E>(event);
			event_queue.push(eventData);
			event_queue_mutex.unlock();
		}
		
		template<typename E> void broadcast_now(const E& event) {
			auto &sigSlots = slotsFor(Signal<E>::id());
			for (auto sig : sigSlots.second) {
				(*sig.second)(static_cast<const void *>(&event));
			}
		}
		
		inline void dispatch_event_queue()
		{
			event_queue_mutex.lock();
			while (!event_queue.empty())
			{
				auto* event = event_queue.front();
				event->invoke(*this);
				delete event;
				event_queue.pop();
			}
			event_queue_mutex.unlock();
		}

	private:
		using SigSlots = std::unordered_map<size_t, SignalPtr>;

		/// Gets you the list of slots for the index eId
		std::pair<size_t, SigSlots> &slotsFor(size_t eId) {
			if (eId >= m_bus.size())
				m_bus.resize(eId + 1);
			return m_bus[eId];
		}

		void clearSignals(std::vector<SignalHandle> &sigHandles) {
			for (auto handle : sigHandles) {
				auto &sigSlots = slotsFor(handle.first);
				sigSlots.second.erase(handle.second);
			}
		}

		std::vector<std::pair<size_t, SigSlots>> m_bus;
		//TODO: Can this be faster with a custom allocator?
		std::queue<IEventData*> event_queue;
		std::mutex event_queue_mutex;
	};
	
	template<typename T>
	void EventData<T>::invoke(EventMgr& evt)
	{
		evt.broadcast_now<T>(args);
	}
}

