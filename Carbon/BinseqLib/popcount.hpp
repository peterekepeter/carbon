#pragma once
#include "types.hpp"
#include "bit_sequence.hpp"

namespace binseq {

	u64 popcount(const bit_sequence&);
	u64 popcount(const u8* bytePtr, const u32 byteCount);

}
