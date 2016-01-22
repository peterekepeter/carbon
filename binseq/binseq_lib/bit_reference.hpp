#pragma once
#include "types.hpp"

namespace binseq {
	/* bit_reference allows the manipulation of any bit from memory as if it 
  were a primitive type */
	class bit_reference {

	private:
		u8* address;
		u8 offset;

	public:
		// constructor 
		inline bit_reference(const void* value, const int offset)
			:address((u8*)value + (offset >> 3)), offset(offset - ((offset >> 3) << 3)) { }

		inline void set() {
			*address |= (0x80 >> offset);
		};

		inline void clear() {
			*address &= ~(0x80 >> offset);
		};

		inline bit test() const {
			return bit((*address) & (0x80 >> offset));
		};

		inline bit_reference& operator =(const bit_reference& a) {
			if (a.test()) {
				set();
			} else {
				clear();
			}
			return *this;
		};

		inline bool operator =(const bool a) {
			if (a) {
				set();
			} else {
				clear();
			}
			return a;
		};

		inline operator bool() {
			return test();
		};
	};

};
