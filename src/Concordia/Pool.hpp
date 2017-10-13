#pragma once

#include <assert.h>
#include <memory>
#include <stdlib.h>
#include <unordered_map>
#include <utility>
#include <vector>

struct IPool {
  virtual ~IPool() {}
  virtual void clear() = 0;
};

template <typename T> class Pool : public IPool {
public:
  Pool(int size = 100) { resize(size); }
  virtual ~Pool() {}

  inline bool empty() const { return m_data.empty(); }

  inline uint size() const { return m_data.size(); }

  inline void resize(int n) {
    const size_t dataSize = size();
    m_data.resize(n);
    if (size() > dataSize) {
      for (auto it = m_data.begin() + dataSize; it != m_data.end(); ++it) {
        it->first = false;
      }
    }
  }

  inline void clear() { m_data.clear(); }

  inline void remove(const T &object) {
    m_data.erase(std::remove(m_data.begin(), m_data.end(), object),
                 m_data.end());
  }

  inline void remove(uint index) { m_data.erase(m_data.begin() + index); }

  inline bool set(uint index, const T &object) {
    assert(index < size());
    m_data[index].first = true;
    m_data[index].second = object;
    return true;
  }

  inline T &get(uint index) {
    assert(index < size());
    return static_cast<T &>(m_data[index].second);
  }

  inline T &recycle() {
    for (auto &e : m_data) {
      if (!e.first) {
        e.first = true;
        return static_cast<T &>(e.second);
      }
    }

    resize(size() * 2);
    return recycle();
  }

  inline void add(const T &object) {
    auto &obj = recycle();
    obj = object;
  }

  inline T &operator[](uint index) { return m_data[index].second; }

  inline const T &operator[](uint index) const { return m_data[index].second; }

  inline std::vector<std::pair<bool, T>> &data() { return m_data; }

private:
  std::vector<std::pair<bool, T>> m_data;
};
