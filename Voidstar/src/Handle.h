#pragma once
namespace Voidstar
{
	template <class Tag>
	struct Handle {

		static constexpr uint16_t INVALID_ID = uint16_t(-1);

		uint16_t idx = INVALID_ID;

		// validity
		bool Valid() const { return idx != INVALID_ID; }

		bool operator==(const Handle& other) const {
			return idx == other.idx;
		}
		bool operator!=(const Handle& other) const {
			return idx != other.idx;
		}

	};
}
