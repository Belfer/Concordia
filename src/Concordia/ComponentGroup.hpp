#pragma once
#include "Identification.hpp"
#include <array>
#include <iostream>

namespace Concordia
{
	namespace Impl
	{
		template<typename T>
		constexpr int get_id_from()
		{
			using TClean = std::decay_t<T>;
			return get_id<TClean>();
		}

		template<typename T, typename = std::enable_if_t<!std::is_pointer_v<T>>>
		constexpr void* to_void_ptr(T& t)
		{
			return static_cast<void*>(&t);
		}

		template<typename T, typename = std::enable_if_t<std::is_pointer_v<T>>>
		constexpr void* to_void_ptr(T t)
		{
			return static_cast<void*>(t);
		}

		template<typename T, int N, typename... Ts>
		struct get_index_in_pack_impl;

		template<typename T, int N, typename T0, typename... Ts>
		struct get_index_in_pack_impl<T, N, T0, Ts...>
		{
			using TClean = std::decay_t<T>;
			using T0Clean = std::decay_t<T0>;
			static constexpr int value = std::is_same_v<TClean, T0Clean> ? N : get_index_in_pack_impl<T, (N + 1), Ts...>::value;
		};

		template<typename T, int N, typename T0>
		struct get_index_in_pack_impl<T, N, T0>
		{
			using TClean = std::decay_t<T>;
			using T0Clean = std::decay_t<T0>;
			static constexpr int value = std::is_same_v<TClean, T0Clean> ? N : -1;
		};

		template<typename T, int N>
		struct get_index_in_pack_impl<T, N>
		{
			static constexpr int value = -1;
		};

		template<typename T, typename... Ts>
		constexpr int get_index_in_pack = get_index_in_pack_impl<T, 0, Ts...>::value;
	}

	///Immutable group of components
	template<typename ...Args>
	struct ComponentGroup
	{
		static constexpr size_t size = sizeof...(Args);

		const std::array<int, size> ids;

		std::array<void*, size> cmps;

		

		/// Default constructor initialises the ids and sets the cmps to nullptr
		constexpr ComponentGroup() 
			: ids{ Impl::get_id_from<Args>()... }, cmps{}
		{ }

		constexpr ComponentGroup(Args&... args)
			: ids{ { Impl::get_id_from<Args>() }... }, cmps{ Impl::to_void_ptr(args)... }
		{
		}

		template<typename T>
		T& get()
		{
			constexpr int index = Impl::get_index_in_pack<T, Args...>;
			static_assert(index != -1, "GetNInTypelist is trying to access a type which is not available");
			if (index == -1)
				assert(false && "This should never happen because of the static_assert");

			return *static_cast<T*>(cmps[index]);
		}

		template<typename T>
		T& get() const
		{
			/*constexpr int index = Impl::get_index_in_pack<T, Args...>;
			static_assert(index != -1, "GetNInTypelist is trying to access a type which is not available");
			if (index == -1)
				assert(false && "generate an error if the static assert did not do it already");

			void* cmp = cmps[index];
			return *static_cast<const T*>(cmp);*/
			assert(false && "remove this");
			T t{};
			return t;
		}
	};
}
