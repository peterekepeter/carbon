#include "stdafx.h"
#include "CppUnitTest.h"
#include "../BinseqLib/bit_sequence.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace binseq;

namespace UnitTestBinseqLib
{
	TEST_CLASS(UnitTest1)
	{
	public:
    
		TEST_METHOD(BitSequenceOpNot){
			auto a = bit_sequence(u16(0xf0f0));
			auto b = not(a);
			Assert::IsTrue(b == bit_sequence(u16(0x0f0f)));
			auto c = bit_sequence("this is what marcellus wallace looks like");
			auto d = not(c);
			auto e = not(d);
			Assert::IsTrue(c == e);
		}

		TEST_METHOD(BitSequenceOpBasic){
			auto a = bit_sequence(u16(0x0101));
			auto b = bit_sequence(u16(0x0011));
			Assert::IsTrue(and(a,b) == bit_sequence(u16(0x0001)));           
			Assert::IsTrue( or(a,b) == bit_sequence(u16(0x0111)));          
			Assert::IsTrue(xor(a,b) == bit_sequence(u16(0x0110)));    
			Assert::IsTrue(nand(a,b) == bit_sequence(u16(0xfffe)));           
			Assert::IsTrue( nor(a,b) == bit_sequence(u16(0xfeee)));          
			Assert::IsTrue(nxor(a,b) == bit_sequence(u16(0xfeef)));
		}


		TEST_METHOD(BitSequenceOpIncompatible){
			auto a = bit_sequence(u8(0xff));
			auto b = bit_sequence(u16(0xffff));
			Assert::ExpectException<std::exception>([&]() { and (a, b); });
		}          

		TEST_METHOD(BitSequenceOpClip){
			auto a = bit_sequence(u32(0xffff0101));
			auto b = bit_sequence(u16(0x0011));
			Assert::IsTrue(andc(a,b) == bit_sequence(u16(0x0001)));           
			Assert::IsTrue( orc(a,b) == bit_sequence(u16(0x0111)));          
			Assert::IsTrue(xorc(a,b) == bit_sequence(u16(0x0110)));    
			Assert::IsTrue(nandc(a,b) == bit_sequence(u16(0xfffe)));           
			Assert::IsTrue( norc(a,b) == bit_sequence(u16(0xfeee)));          
			Assert::IsTrue(nxorc(a,b) == bit_sequence(u16(0xfeef)));
		}

		TEST_METHOD(BitSequenceOpRepeat){
			auto a = bit_sequence(u32(0x00110011));
			auto b = bit_sequence(u8(0x01));
			Assert::IsTrue(andr(a,b) == bit_sequence(u32(0x00010001)));           
			Assert::IsTrue( orr(a,b) == bit_sequence(u32(0x01110111)));          
			Assert::IsTrue(xorr(a,b) == bit_sequence(u32(0x01100110)));    
			Assert::IsTrue(nandr(a,b) == bit_sequence(u32(0xfffefffe)));           
			Assert::IsTrue( norr(a,b) == bit_sequence(u32(0xfeeefeee)));          
			Assert::IsTrue(nxorr(a,b) == bit_sequence(u32(0xfeeffeef)));
		}

		TEST_METHOD(BitSequenceOpRepeat2){
			auto a = bit_sequence(u16(0xffff));
			auto b = bit_sequence(u8(0x01));
			auto c = head(b,3);
			auto d = andr(c,a);
			Assert::IsTrue(d == bit_sequence(u16(0x9249)));
		}

		TEST_METHOD(BitSequenceOpRepeat3){
			auto a = bit_sequence(u8(0x0f));
			auto b = repeat(repeat(a,4),8);
			Assert::IsTrue(b == bit_sequence(u8(0xff)));
		}

	};
}