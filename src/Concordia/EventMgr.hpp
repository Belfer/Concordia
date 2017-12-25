#pragma once

#include "NonCopyable.hpp"
#include <assert.h>
#include <memory>
#include <stdlib.h>
#include <unordered_map>
#include <utility>
#include <vector>
#include <functional>

namespace Concordia {
	/**
	 * @brief Events
	 */

	 //TODO: Consider renaming sig to 'ISignal' for clarity
	struct ISignal {
		virtual ~ISignal() {}
		virtual void operator()(const void *p) = 0;

	protected:
		static size_t regId() {
			static size_t counter = -1;
			return ++counter;
		}
	};

	//IDID: renaming sig to 'Signal' or 'signal' for clarity
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

	//TODO: Consider renaming to SignalHandle
	using SigHandle = std::pair<size_t, size_t>;

	struct Receiver {
		~Receiver() {
			if (m_sigHandles.size() > 0)
				m_clearSigFn(m_sigHandles);
		}

	private:
		friend class EventMgr;
		std::function<void(std::vector<SigHandle> &)> m_clearSigFn;
		std::vector<SigHandle> m_sigHandles;
	};

	class EventMgr : NonCopyable {
	public:
		//TODO: What does this even stand for?
		using SignalPtr = std::shared_ptr<ISignal>;

		EventMgr() {}
		~EventMgr() {
			// TODO, clean memory
		}

		template <typename E, typename Receiver> void subscribe(Receiver &receiver) {
			if (receiver.m_sigHandles.size() == 0) {
				receiver.m_clearSigFn =
					std::bind(&EventMgr::clearSignals, this, std::placeholders::_1);
			}

			void (Receiver::*receive)(const E &) = &Receiver::receive;
			auto signal =
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
			broadcast(E(args...));
		}

		template <typename E> void broadcast(const E &event) {
			auto &sigSlots = slotsFor(Signal<E>::id());
			for (auto sig : sigSlots.second) {
				(*sig.second)(static_cast<const void *>(&event));
			}
		}

	private:
		using SigSlots = std::unordered_map<size_t, SignalPtr>;

		std::pair<size_t, SigSlots> &slotsFor(size_t eId) {
			if (eId >= m_bus.size())
				m_bus.resize(eId + 1);
			return m_bus[eId];
		}

		void clearSignals(std::vector<SigHandle> &sigHandles) {
			for (auto handle : sigHandles) {
				auto &sigSlots = slotsFor(handle.first);
				sigSlots.second.erase(handle.second);
			}
		}

		std::vector<std::pair<size_t, SigSlots>> m_bus;
	};
}
