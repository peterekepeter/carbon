#include "stdafx.h"
#include "CppUnitTest.h"
#include <exception>
#include "../BinseqLib/bit_sequence.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace binseq;

namespace UnitTestBinseqLib
{
	TEST_CLASS(UnitTest1)
	{
	public:

		TEST_METHOD(BitSequenceCompare){
			auto a = bit_sequence(u8(3));
			auto b = bit_sequence(u8(3));   
			auto c = bit_sequence(u8(4));
			Assert::IsTrue(a == b);              
			Assert::IsTrue(a != c);    
			Assert::IsTrue(!(a == c));   
			Assert::IsTrue(!(a != b));    
			Assert::IsTrue(b < c);    
			Assert::IsTrue(c > b);      
			Assert::IsTrue(b <= c);   
			Assert::IsTrue(c >= b);    
			Assert::IsTrue(!(b > c));  
			Assert::IsTrue(!(c < b));  
			Assert::IsTrue(!(b >= c)); 
			Assert::IsTrue(!(c <= b));
			Assert::IsTrue(a <= b); 
			Assert::IsTrue(a >= b);
		}       

		TEST_METHOD(BitSequenceAscii){
			auto a = bit_sequence("hello world!");    
			auto b = bit_sequence("hello world!");
			Assert::IsTrue(a == b);                       
			auto c = bit_sequence("hello binseq");    
			Assert::IsTrue(a != c);        
		}

		TEST_METHOD(BitSequenceSubseq)
		{
			auto a = bit_sequence(u16(0x00ff));
			Assert::IsTrue(bit_sequence(u8(0xff)) == subseq(a,0,8));   
			Assert::IsTrue(bit_sequence(u8(0x00)) == subseq(a,8,8));   
			Assert::IsTrue(bit_sequence(u8(0x0f)) == subseq(a,4,8));  
		}             

		TEST_METHOD(BitSequenceSubseq2)
		{
			auto a = bit_sequence(u16(0x0f0f));    
			Assert::IsTrue(subseq(a,0,4) == subseq(a,8,4));   
			Assert::IsTrue(subseq(a,1,4) == subseq(a,9,4));   
			Assert::IsTrue(subseq(a,2,4) == subseq(a,10,4)); 
			Assert::IsTrue(subseq(a,3,4) == subseq(a,11,4)); 
			Assert::IsTrue(subseq(a,4,4) == subseq(a,12,4));        
		}           

		TEST_METHOD(BitSequenceSubseq3)
		{
			auto a = bit_sequence(u16(0x00ff));    
			auto b = subseq(a,6,4);
			Assert::IsTrue(b[0] == true);     
			Assert::IsTrue(b[1] == true);
			Assert::IsTrue(b[2] == false);
			Assert::IsTrue(b[3] == false);   
		}       

		TEST_METHOD(BitSequenceConcat)
		{
			auto a = bit_sequence(u16(0x00ff)) + bit_sequence(u16(0xff00));    
			Assert::IsTrue(a.size()==32);
			Assert::IsTrue(bit_sequence(u32(0xff0000ff)) == a); 
		}         

		TEST_METHOD(BitSequenceConcat2)
		{
			auto b = bit_sequence("hello") + bit_sequence("world");
			auto c = bit_sequence("helloworld");
			Assert::IsTrue(b == c);
		}      

		TEST_METHOD(BitSequenceConcat3)
		{
			auto a = bit_sequence(u8(3));
			auto c = subseq(a,1,2) + subseq(a,0,4) + subseq(a,4,2) + subseq(a,1,2);
			Assert::IsTrue(c.size() == 10);
			Assert::IsTrue(c[0] == true);              
			Assert::IsTrue(c[1] == false);
			Assert::IsTrue(c[2] == true);    
			Assert::IsTrue(c[3] == true);              
			Assert::IsTrue(c[4] == false);           
			Assert::IsTrue(c[5] == false);           
			Assert::IsTrue(c[6] == false);           
			Assert::IsTrue(c[7] == false);           
			Assert::IsTrue(c[8] == true);         
			Assert::IsTrue(c[9] == false);
		}

	};
}
