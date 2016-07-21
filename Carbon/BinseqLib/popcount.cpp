#include "types.hpp"
#include "bit_sequence.hpp"


namespace binseq {
	static inline unsigned char bitcount(unsigned x) {
		unsigned char acc = 0;
		while (x)
			acc += x & 1 , x >>= 1;
		return acc;
	}


	static unsigned char* btc_table = nullptr;

	inline unsigned char bitc(const unsigned char x) {
		return btc_table[x];
	}

	inline unsigned char bitc(const char x) {
		return btc_table[unsigned char(x)];
	}

	inline unsigned char bitc(const unsigned short x) {
		return btc_table[x];
	}

	inline unsigned char bitc(const short x) {
		return btc_table[unsigned short(x)];
	}

	inline unsigned char bitc(const unsigned x) {
		return btc_table[x >> 16] + btc_table[x & 65535];
	}

	inline unsigned char bitc(const int x) {
		return bitc(unsigned(x));
	}

	inline unsigned char bitc(const unsigned long long x) {
		return btc_table[x & 65535] + btc_table[(x >> 16) & 65535] + btc_table[(x >> 32) & 65535] + btc_table[x >> 48];
	}

	inline unsigned char bitc(const long long x) {
		return bitc((unsigned long long) x);
	}

	static int bitcount_x86(const void* data, size_t size) {
		int acc = 0;
		auto byteptr = (unsigned char*) data;
		auto ptr = (unsigned*) data;
		auto count = size >> 2;
		while (count--) acc += bitc(*ptr++);
		switch (size & 3) {
			case 3: acc += bitc(byteptr[size - 3]);
			case 2: acc += bitc(byteptr[size - 2]);
			case 1: acc += bitc(byteptr[size - 1]);
		}
		return acc;
	}


	static int bitcount_x64(const void* data, size_t size) {
		int acc = 0, i = 0;
		auto byteptr = (unsigned char*) data;
		auto ptr = (unsigned long long*) data;
		auto count = size >> 3;
		while (count--) {
			acc += bitc(ptr[i++]);
		}
		switch (size & 7) {
			case 7: acc += bitc(byteptr[size - 7]);
			case 6: acc += bitc(byteptr[size - 6]);
			case 5: acc += bitc(byteptr[size - 5]);
			case 4: acc += bitc(byteptr[size - 4]);
			case 3: acc += bitc(byteptr[size - 3]);
			case 2: acc += bitc(byteptr[size - 2]);
			case 1: acc += bitc(byteptr[size - 1]);
		}
		return acc;
	}

	static void genTable() {
		for (int i = 0; i < 65536; i++)
			btc_table[i] = bitcount(i);
	}

	static void init() {
		if (btc_table == nullptr) {
			btc_table = new unsigned char [65536];
			genTable();
		}
	}

	u64 popcount(const bit_sequence& seq) {
		init();

		auto size = seq.size();
		auto sizeBytes = size >> 3;
		auto tailBits = size & 7;

		u64 count = bitcount_x86(seq.address(), (u32)(sizeBytes));

		auto tail = extractBitsAligned<u8>(seq, (u32)(sizeBytes), (u8)(tailBits));
		count += bitcount(tail);

		return count;
	}

	u64 popcount(const u8* bytePtr, const u32 byteCount) {
		init();
		return bitcount_x86(bytePtr, byteCount);
	}

}
