#pragma once

#ifdef __clang__
#define FUNCTION_SIGNATURE __PRETTY_FUNCTION__
#elif __GNUC__
#define FUNCTION_SIGNATURE __PRETTY_FUNCTION__
#elif _MSC_VER
#define FUNCTION_SIGNATURE __FUNCSIG__
#endif

#ifdef __clang__
#define FUNCTION_STR ""
#elif __GNUC__
#define FUNCTION_STR ""
#elif _MSC_VER
#define FUNCTION_STR __FUNCSIG__
#endif

template<typename... T>
constexpr void StaticAssertPrint()
{
	//Only in MSVC is the FUNCTION_SIGNATURE a macro that is expanded, in gcc it is a variable,
	//Thus it can't be used to print a string literal like here
#ifdef _MSC_VER
	static_assert(false, FUNCTION_SIGNATURE);
#endif
}

inline void EmptyFunction()
{
}

void ForceEmptyFunction(volatile char = 0);
