#pragma once
#include "types.hpp"
#include "bit_reference.hpp"

namespace binseq {
	/* a short easy to use mini array of bits */
	template <class T>
	struct bit_collection {
		T value;
		u8 size;

		bit_collection() :value(0), size(0) {}

		bit_collection(T v, int offset, u8 s)
			:value((v >> offset) & (~(T(-1) << s)))
			 ,size(s) {}

		bit_reference operator [](const u8 i)
		{
			return bit_reference(&value, i);
		}

		bool operator==(const bit_collection& other) {
			return size == other.size && value == other.value;
		}

		bool operator!=(const bit_collection& other) {
			return size != other.size || value != other.value;
		}

		operator T() {
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

		bit_collection_reference() {}

		bit_collection_reference(T* v, int offset, u8 s):value(v),size(s),offset(offset) {}

		bit_reference operator [](const u8 i) {
			return bit_reference(value, i + offset);
		}

		operator bit_collection<T>() {
			return bit_collection<T>(*value, offset, size);
		}

		operator T() {
			return T(bit_collection<T>(*value, offset, size));
		}

		T operator =(T a) {
			T mask = (T(-1) << (size + offset)) ^ (T(-1) << offset);
			*value = (*value & ~mask) | ((a << offset) & mask);
			return a;
		}

		T operator=(bit_collection_reference other) {
			return operator=(T(other));
		}

	};
};
