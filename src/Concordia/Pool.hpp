#pragma once

#include <cassert>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>
#include "CommonTypes.hpp"
#include "MetaDebugging.hpp"

namespace Concordia {

	/*
	 * @brief A generic Pool for components
	 */

	class IPool {
	public:
		virtual ~IPool() = default;
		virtual void clear() = 0;
		virtual bool containsId(size_t entity_id)
		{
			return std::find(entity_ids.begin(), entity_ids.end(), entity_id) != entity_ids.end();
		}
		virtual void removeById(size_t entity_id) = 0;
		
	protected:
		int getIndexFromId(size_t entity_id)
		{
			for (uint i = 0; i < entity_ids.size(); ++i)
			{
				if (entity_ids[i] == entity_id)
					return i;
			}

			return -1;
		}

	protected:
		std::vector<size_t> entity_ids {};
	};

	template <typename T> 
	class Pool : public IPool {
#ifdef _DEBUG
		const char* cmp_name_ = typeid(T).name();
#endif
	public:
		Pool(int size = 100)
		{
			static_assert(std::is_default_constructible<T>::value, 
				"Component of pool is not default constructable, more info: " FUNCTION_SIGNATURE);
			resize(size);
		}
		virtual ~Pool() = default;

		inline bool empty() const { return m_data.empty(); }

		inline uint size() const { return m_data.size(); }

		inline void resize(int n) {
			const size_t dataSize = size();
			m_data.resize(n);
			entity_ids.resize(n);
			if (size() > dataSize) {
				for (auto it = m_data.begin() + dataSize; it != m_data.end(); ++it) {
					it->first = false;
				}
			}
		}

		inline void clear() override { m_data.clear(); }

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

		inline bool active(uint index)
		{
			assert(index < size());
			return m_data[index].first;
		}

		inline T& get(uint index) {
			assert(index < size());
			return static_cast<T &>(m_data[index].second);
		}

		inline T* getById(size_t entity_id)
		{
			int index = getIndexFromId(entity_id);
			if (index == -1)
				return nullptr;

			return &m_data[index].second;
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

		inline size_t getOpenSpot()
		{
			for (uint i = 0; i < m_data.size(); ++i)
			{
				if (!m_data[i].first)
					return i;
			}

			resize(size() * 2);
			return getOpenSpot();
		}

		inline void add(size_t entity_id, const T& object) {
			size_t index = getOpenSpot();
			m_data[index].first = true;
			T& obj = m_data[index].second;
			obj = object;
			entity_ids[index] = entity_id;
		}

		inline T &operator[](uint index) { return m_data[index].second; }

		inline const T &operator[](uint index) const { return m_data[index].second; }

		inline std::vector<std::pair<bool, T>> &data() { return m_data; }

		void removeById(size_t entity_id) override
		{
			int index = getIndexFromId(entity_id);
			if (index == -1)
				return;

			m_data[index].first = false;
			entity_ids[index] = 0;
		}
	private:
		std::vector<std::pair<bool, T>> m_data{};
	};
}
