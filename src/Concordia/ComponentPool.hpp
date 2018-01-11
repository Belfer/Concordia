#pragma once
#include "Identification.hpp"
#include <vector>
#include <cassert>
#include "CommonTypes.hpp"
#include <iterator>


namespace Concordia
{
//	template<typename C>
//	class CmpPool;
//
//	class ICmpPool
//	{	
//	public:
//		const size_t component_id;
//		const size_t element_size;
//
//		byte* cmps_data;
//		std::vector<size_t> entity_ids;
//
//	public:
//		explicit ICmpPool(size_t cmp_id, size_t elem_size, unsigned int entity_amount = 100)
//			: component_id(cmp_id), element_size(elem_size), entities(0), capacity(entity_amount)
//		{
//			cmps_data = new byte[element_size * capacity];
//			for (uint i = 0; i < element_size*capacity; ++i)
//			{
//				cmps_data[i] = 0;
//			}
//			entity_ids = {};
//		}
//
//		~ICmpPool()
//		{
//			delete[] cmps_data;
//		}
//
//		bool hasComponent(size_t entity_id)
//		{
//			return std::find(entity_ids.begin(), entity_ids.end(), entity_id) != entity_ids.end();
//		}
//
//		byte* getRawComponent(size_t entity_id)
//		{
//			int index = getIndexFromId(entity_id);
//			if(index==-1)
//				return nullptr;
//
//			return cmps_data + index * element_size;
//		}
//
//		byte* addComponent(size_t entity_id, byte component[] )
//		{
//			entity_ids.push_back(entity_id);
//			insertComponent(component);
//
//			return component;
//		}
//
//		/// Simply overrides all the data of the component, effectively erasing it.
//		/// Because of this, the destructor is not called on the object,
//		/// For that look at @CmpPool<C>::removeComponent
//		void nullifyComponent(size_t entity_id)
//		{
//			int index = getIndexFromId(entity_id);
//			if (index == -1)
//				return;
//
//			entity_ids.erase(entity_ids.begin() + index);
//			unsigned cmp_begin = index * element_size;
//			for (uint i = 0; i < element_size; ++i)
//			{
//				cmps_data[cmp_begin + i] = 0;
//			}
//		}
//
//		unsigned int size() const
//		{
//			return entities;
//		}
//
//		unsigned int max_size() const
//		{
//			return capacity;
//		}
//
//		int getIndexFromId(size_t entity_id) const
//		{
//			for (uint i = 0; i < entities; ++i) {
//				if (entity_ids[i] == entity_id) {
//					return i;
//				}
//			}
//
//			return -1;
//		}
//
//		template<typename C>
//		CmpPool<C>& as();
//
//		template<typename C>
//		const CmpPool<C>& as() const;
//
//	protected:
//		unsigned int entities;
//		unsigned int capacity;
//
//	protected:
//		void insertComponent(const byte comp[])
//		{
//			checkSize();
//
//			unsigned cmp_offset = element_size * entities;
//			for (uint i = 0; i < element_size; ++i) {
//				cmps_data[cmp_offset + i] = comp[i];
//			}
//
//			++entities;
//		}
//
//		void checkSize()
//		{
//			//OLDTODO: Check this if statement:
//			if (entities >= capacity-1) {
//				byte* old_data = cmps_data;
//				size_t old_capacity = capacity;
//				capacity += 50;
//				cmps_data = new byte[element_size * capacity];
//
//				for (uint i = 0; i < old_capacity; ++i) {
//					cmps_data[i] = old_data[i];
//				}
//
//				delete[] old_data;
//			}
//		}
//	};
//
//	template<typename C>
//	class CmpPool : public ICmpPool
//	{
//#ifdef _DEBUG
//		const char* cmp_name = typeid(C).name();
//#endif
//	public:
//		explicit CmpPool(unsigned int entity_amount = 100) : ICmpPool(get_cmp_id<C>(), sizeof(C), entity_amount)
//		{
//		}
//
//		C* getComponentPtr(unsigned int index = 0)
//		{
//			return reinterpret_cast<C*>(cmps_data) + index;
//		}
//
//		const C* getComponentPtr(unsigned int index = 0) const
//		{
//			return reinterpret_cast<const C*>(cmps_data) + index;
//		}
//		
//		C* getComponent(size_t entity_id)
//		{
//			int index = getIndexFromId(entity_id);
//			if (index == -1)
//				return nullptr;
//
//			return getComponentPtr(index);
//		}
//
//		const C& addComponent(size_t entity_id, const C& component)
//		{
//			//oldtodo: insert at an open position
//			entity_ids.push_back(entity_id);
//			checkSize();
//			C* ptr = getComponentPtr(entities++);
//			//insertComponent(reinterpret_cast<const byte*>( &component ));
//			*ptr = component;
//
//			return component;
//		}
//
//		void removeComponent(size_t entity_id)
//		{
//			int index = getIndexFromId(entity_id);
//			if (index == -1)
//				return;
//
//			entity_ids.erase(entity_ids.begin() + index);
//
//			C* cmp_ptr = getComponentPtr(index);
//			//oldTODO: Does this work?
//			delete cmp_ptr;
//
//			unsigned cmp_begin = index * element_size;
//			for (int i = 0; i < element_size; ++i) {
//				cmps_data[cmp_begin + i] = 0;
//			}
//		}
//		
//		//oldTODO: Should it save the entity_id too?
//		struct const_iterator
//		{
//			using difference_type = ptrdiff_t;
//			using value_type = C;
//			using reference = C & ;
//			using pointer = C * ;
//			using iterator_category = std::random_access_iterator_tag;
//			
//			C* ptr;
//			size_t* entity_ids;
//
//			explicit const_iterator(C ptr[], size_t ent_ids[]) : ptr(ptr), entity_ids(ent_ids) {}
//
//			//oldTODO: Should this return a pair or is the seperate method good enough?
//			reference operator*() { return *ptr; }
//			pointer operator->() { return ptr; }
//			size_t entity_id() const { return *entity_ids; }
//
//			const_iterator operator+(int elements) { return {ptr + elements, entity_ids + elements}; }
//			const_iterator& operator++()
//			{
//				++ptr;
//				++entity_ids;
//				return *this;
//			}
//			const_iterator& operator+=(int elements)
//			{
//				ptr += elements;
//				entity_ids += elements;
//				return *this;
//			}
//
//			const_iterator operator-(int elements) { return {ptr - elements, entity_ids - elements}; }
//			const_iterator& operator--()
//			{
//				--ptr;
//				--entity_ids;
//				return *this;
//			}
//			const_iterator& operator-=(int elements)
//			{
//				ptr -= elements;
//				entity_ids -= elements;
//				return *this;
//			}
//		};
//		struct iterator : const_iterator{
//			explicit iterator(C ptr[], size_t ent_ids[]) : const_iterator(ptr, ent_ids){}
//		};
//
//		iterator begin()
//		{
//			return { getComponentPtr() };
//		}
//
//		const_iterator begin() const
//		{
//			return { getComponentPtr() };
//		}
//
//		iterator end()
//		{
//			return { getComponentPtr() };
//		}
//
//		const_iterator end() const
//		{
//			return { getComponentPtr() };
//		}
//	};
//
//	template <typename C>
//	CmpPool<C>& ICmpPool::as()
//	{
//		assert(get_cmp_id<C>() == this->component_id);
//		return reinterpret_cast<CmpPool<C>&>(*this);
//	}
//
//	template <typename C>
//	const CmpPool<C>& ICmpPool::as() const
//	{
//		assert(get_cmp_id<C>() == this->component_id);
//		return reinterpret_cast<const CmpPool<C>&>(*this);
//	}
}
