#pragma once
#include "types.hpp"
#include "bit_reference.hpp"

namespace binseq {

	/* bit_seuence is a value class and it can hold a generic binary sequence */
	class bit_sequence {

	private:
		u64 _capacity; //the amount of u64s the structure can hold
		u64 _sizebits; //the amount of bits the structure is holding    
		union {
			u64* addr; //in memory address where the sequence begins
			u64 raw[2]; //sequence contained inside this structure
		};

		inline void allocate(u64 size) {
			_sizebits = size;
			if (size <= 128) {
				_capacity = 2;
				raw[0] = 0; //in structure representation    
				raw[1] = 0; //in structure representation
			} else {
				addr = new u64[(u32)(_capacity = (size + 63) >> 6)];
			};
		}

	public:
		inline u64 size() const {
			return _sizebits;
		}; //size in bits
		inline u64 capacity() const {
			return _capacity << 6;
		}; //capacity in bits
		inline void* address() {
			if (_sizebits > 128) return addr; else return (void*)&raw;
		} //sequence start address
		inline const void* address() const {
			if (_sizebits > 128) return addr; else return (void*)&raw;
		}

		inline bit_reference operator[](u64 bitIndex) {
			return bit_reference(reinterpret_cast<u8*>(address()) + (bitIndex >> 3), bitIndex & 7);
		}

		inline bit operator[](u64 bitIndex) const {
			return operator[](bitIndex);
		}

		inline void deallocate() {
			if (_sizebits > 128)
				delete []addr;
			_sizebits = 0;
		}

		inline void reallocate(u64 size) {
			deallocate();
			allocate(size);
		}

		inline bit_sequence() {
			_sizebits = 0;
			_capacity = 0;
			addr = 0;
		};

		inline bit_sequence(const bit value) {
			allocate(1);
			*reinterpret_cast<u64*>(address()) = value ? 1 : 0;
		}

		inline bit_sequence(const u8 value) {
			allocate(8);
			*reinterpret_cast<u64*>(address()) = value;
		}

		inline bit_sequence(const u16 value) {
			allocate(16);
			*reinterpret_cast<u64*>(address()) = value;
		}

		inline bit_sequence(const u32 value) {
			allocate(32);
			*reinterpret_cast<u64*>(address()) = value;
		}

		inline bit_sequence(const u64 value) {
			allocate(64);
			*reinterpret_cast<u64*>(address()) = value;
		}

		bit_sequence(const char*);

		inline bit_sequence(const bit_sequence& other) {
			_sizebits = other._sizebits;
			if (other._sizebits <= 128) {
				_capacity = other._capacity;
				raw[0] = other.raw[0]; //copy in structure representation                 
				raw[1] = other.raw[1];
			} else {
				allocate(other.size());
				for (u32 i = 0; i < _capacity; i++) addr[i] = other.addr[i]; //copy
			};
		};

		inline bit_sequence(bit_sequence&& other) {
			_sizebits = other._sizebits;
			_capacity = other._capacity;
			raw[0] = other.raw[0];
			raw[1] = other.raw[1];
			other._sizebits = 0;
		} //hostile takeover

		inline bit_sequence& operator =(bit_sequence&& other) {
			if (this != &other) {
				deallocate();
				raw[0] = other.raw[0];
				raw[1] = other.raw[1];
				_sizebits = other._sizebits;
				_capacity = other._capacity;
				other._sizebits = 0;
			}
			return *this;
		}

		inline bit_sequence& operator =(const bit_sequence& other) {
			if (this != &other) {
				deallocate();
				if (other._sizebits <= 128) {
					_sizebits = other._sizebits;
					_capacity = other._capacity;
					raw[0] = other.raw[0]; //copy in structure representation                 
					raw[1] = other.raw[1];
				} else {
					allocate(other._sizebits);
					for (u32 i = 0; i < _capacity; i++) addr[i] = other.addr[i]; //copy
				};
			}
			return *this;
		}

		inline ~bit_sequence() {
			deallocate();
		}

	};

	/* helper for implementing algorithms */

	template <class T>
	inline T extractBitsAligned(const bit_sequence& seq, const u32 byteOffset, const u8 bitCount) {
		const u8* ptr = reinterpret_cast<const u8*>(seq.address());
		const T* tptr = reinterpret_cast<const T*>(ptr + byteOffset);
		T value = *tptr;
		T mask = -1;
		mask = mask << (sizeof(T) * 8 - bitCount);
		return mask & value;
	}

	/* comparison operators for bit_sequence */

	bool operator ==(const bit_sequence&, const bit_sequence&); // equals    
	bool operator !=(const bit_sequence&, const bit_sequence&); // not equals     
	bool operator >(const bit_sequence&, const bit_sequence&);
	bool operator <(const bit_sequence&, const bit_sequence&);
	bool operator >=(const bit_sequence&, const bit_sequence&);
	bool operator <=(const bit_sequence&, const bit_sequence&);

	bit_sequence operator +(const bit_sequence&, const bit_sequence&); //concat

	/* selectors */

	bit_sequence subseq(const bit_sequence&, u64 offset, u64 size);
	bit_sequence head(const bit_sequence&, u64 size);
	bit_sequence tail(const bit_sequence&, u64 size);
	bit_sequence repeat(const bit_sequence&, u64 size);

	/* standard operators */

	bit_sequence not(const bit_sequence&);

	bit_sequence and(const bit_sequence&, const bit_sequence&);
	bit_sequence or(const bit_sequence&, const bit_sequence&);
	bit_sequence xor(const bit_sequence&, const bit_sequence&);
	bit_sequence nand(const bit_sequence&, const bit_sequence&);
	bit_sequence nor(const bit_sequence&, const bit_sequence&);
	bit_sequence nxor(const bit_sequence&, const bit_sequence&);

	bit_sequence andr(const bit_sequence&, const bit_sequence&);
	bit_sequence orr(const bit_sequence&, const bit_sequence&);
	bit_sequence xorr(const bit_sequence&, const bit_sequence&);
	bit_sequence nandr(const bit_sequence&, const bit_sequence&);
	bit_sequence norr(const bit_sequence&, const bit_sequence&);
	bit_sequence nxorr(const bit_sequence&, const bit_sequence&);

	bit_sequence andc(const bit_sequence&, const bit_sequence&);
	bit_sequence orc(const bit_sequence&, const bit_sequence&);
	bit_sequence xorc(const bit_sequence&, const bit_sequence&);
	bit_sequence nandc(const bit_sequence&, const bit_sequence&);
	bit_sequence norc(const bit_sequence&, const bit_sequence&);
	bit_sequence nxorc(const bit_sequence&, const bit_sequence&);


}
