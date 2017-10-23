#pragma once

#include "NonCopyable.hpp"
#include <assert.h>
#include <memory>
#include <stdlib.h>
#include <unordered_map>
#include <utility>
#include <vector>
#include <functional>

/**
 * @brief Events
 */

struct ISig {
  virtual ~ISig() {}
  virtual void operator()(const void *p) = 0;

protected:
  static size_t regId() {
    static size_t counter = -1;
    return ++counter;
  }
};

template <typename E> struct Sig : public ISig {
  explicit Sig(std::function<void(const E &)> sigFn) : m_sigFn(sigFn) {}
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
  using SigSPtr = std::shared_ptr<ISig>;

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
        new Sig<E>(std::bind(receive, &receiver, std::placeholders::_1));

    auto &sigSlots = slotsFor(Sig<E>::id());
    sigSlots.second[sigSlots.first] = SigSPtr(signal);

    receiver.m_sigHandles.emplace_back(
        std::make_pair(Sig<E>::id(), sigSlots.first));

    sigSlots.first++;
  }

  template <typename E, typename Receiver>
  void unsubscribe(Receiver &receiver) {
    auto &sigSlots = slotsFor(Sig<E>::id());
    for (auto handle : receiver.m_sigHandles) {
      if (handle.first == Sig<E>::id())
        sigSlots.second.erase(handle.second);
    }
  }

  template <typename E, typename... Args> void broadcast(Args... args) {
    broadcast(E(args...));
  }

  template <typename E> void broadcast(const E &event) {
    auto &sigSlots = slotsFor(Sig<E>::id());
    for (auto sig : sigSlots.second) {
      (*sig.second)(static_cast<const void *>(&event));
    }
  }

private:
  using SigSlots = std::unordered_map<size_t, SigSPtr>;

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
