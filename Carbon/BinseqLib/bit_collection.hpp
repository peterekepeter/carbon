#pragma once
#include "types.hpp"

namespace binseq {
	/* a short easy to use mini array of bits */
	template <class T>
	struct bit_collection {
		T value;
		u8 size;

		inline bit_collection() {}

		inline bit_collection(T v, int offset, u8 s)
			:value((v >> offset) & (~(T(-1) << s)))
			 ,size(s) {}

		inline bit_reference operator [](const u8 i) {
			return bit_reference(&value, i);
		}

		inline bool operator==(const bit_collection& other) {
			return size == other.size && value == other.value;
		}

		inline bool operator!=(const bit_collection& other) {
			return size != other.size || value != other.value;
		}

		inline operator T() {
			return value;
		}
	};

	/* a short, easy to pass by value bit sequence, the data is stored inside
  the value attribute and count represents the number of bits used */
	template <class T>
	struct bit_collection_reference {
		T* value;
		u8 offset;
		u8 size;

		inline bit_collection_reference() {}

		inline bit_collection_reference(T* v, int offset, u8 s):value(v),size(s),offset(offset) {}

		inline bit_reference operator [](const u8 i) {
			return bit_reference(value, i + offset);
		}

		inline operator bit_collection<T>() {
			return bit_collection<T>(*value, offset, size);
		}

		inline operator T() {
			return T(bit_collection<T>(*value, offset, size));
		}

		inline T operator =(T a) {
			T mask = (T(-1) << (size + offset)) ^ (T(-1) << offset);
			*value = (*value & ~mask) | ((a << offset) & mask);
			return a;
		}

		inline T operator=(bit_collection_reference other) {
			return operator=(T(other));
		}

	};
};
