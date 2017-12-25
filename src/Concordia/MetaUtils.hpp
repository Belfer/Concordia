#pragma once
#include <type_traits>

namespace Concordia
{
	namespace Impl
	{
		using uint = unsigned int;

		template<uint I, typename... TypeList>
		struct GetStruct{};

		template<typename T, typename... Types>
		struct GetStruct<0, T, Types...>
		{
			using type = T;
		};

		template<uint I, typename T, typename... Types>
		struct GetStruct<I, T, Types...>
		{
			using type = typename GetStruct<I-1, Types...>::type;
		};
	}

	template<unsigned int I, typename... TypeList>
	using GetNInTypelist = typename Impl::GetStruct<I, TypeList...>::type;
}
