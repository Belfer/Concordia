#pragma once

#ifdef __clang__
#define FUNCTION_SIGNATURE __PRETTY_FUNCTION__
#elif __GNUC__
#define FUNCTION_SIGNATURE __PRETTY_FUNCTION__
#elif _MSC_VER
#define FUNCTION_SIGNATURE __FUNCSIG__
#endif

template<typename... T>
constexpr void StaticAssertPrint()
{
	static_assert(false, FUNCTION_SIGNATURE);
}

inline void EmptyFunction()
{
}