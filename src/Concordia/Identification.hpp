#pragma once

namespace Concordia
{
	static size_t generate_id() {
		static size_t s_cmpCounter = -1;
		return ++s_cmpCounter;
	}

	template<typename E>
	constexpr size_t get_id()
	{
		static size_t my_id = generate_id();
		return my_id;
	}
}
