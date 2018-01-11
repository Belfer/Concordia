#pragma once

namespace Concordia
{
	inline size_t generate_cmp_id() {
		static size_t s_cmpCounter = -1;
		return ++s_cmpCounter;
	}

	template<typename E>
	size_t get_cmp_id()
	{
		static size_t my_id = generate_cmp_id();
		return my_id;
	}
}
