#pragma once
namespace util {
	template <class T>
	inline void hash_combine(std::size_t& seed, const T& v) {
		std::hash<T> h;
		seed ^= h(v) + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
	}

	template<class Enum>
	inline std::enable_if_t<std::is_enum<Enum>::value, std::size_t>
		hash_enum(Enum e) {
		using U = std::underlying_type_t<Enum>;
		return std::hash<U>{}(static_cast<U>(e));
	}
}


