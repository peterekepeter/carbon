#include "stdafx.h"
#include "CppUnitTest.h"
#include "../BinseqLib/bit_collection.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace binseq;

namespace UnitTestBinseqLib
{
	TEST_CLASS(BinseqUnitTest)
	{
	public:

		TEST_METHOD(BitCollectionReferenceArray) {
			u32 a=7;
			bit_collection_reference<u32> y(&a,0,4);
			Assert::IsTrue(y[0] == true);           
			Assert::IsTrue(y[1] == true);
			Assert::IsTrue(y[2] == true);
			Assert::IsTrue(y[3] == false);      
			bit_collection_reference<u32> x(&a,2,4);
			Assert::IsTrue(x[0] == true);     
			Assert::IsTrue(x[1] == false);
			Assert::IsTrue(x[2] == false);
			Assert::IsTrue(x[3] == false);
		}

		TEST_METHOD(BitCollectionReferenceTypecast)
		{
			u32 a=255;
			{ bit_collection_reference<u32> x(&a,0,4); Assert::IsTrue(x == 15); } 
			{ bit_collection_reference<u32> x(&a,1,5); Assert::IsTrue(x == 31); }      
			{ bit_collection_reference<u32> x(&a,4,4); Assert::IsTrue(x == 15); }      
			{ bit_collection_reference<u32> x(&a,5,4); Assert::IsTrue(x == 7); }  
			{ bit_collection_reference<u32> x(&a,6,4); Assert::IsTrue(x == 3); }   
			{ bit_collection_reference<u32> x(&a,7,4); Assert::IsTrue(x == 1); } 
			{ bit_collection_reference<u32> x(&a,8,4); Assert::IsTrue(x == 0); }   
			{ bit_collection_reference<u32> x(&a,0,16); Assert::IsTrue(x == 255); }
		}

		TEST_METHOD(BitCollectionReferenceAssign)
		{
			u32 a=0xffff;
			bit_collection_reference<u32> x(&a,4,8);
			x = 0;                
			Assert::IsTrue(a==0xf00f);
			x = 3;              
			Assert::IsTrue(a==0xf03f);
			x = 12;             
			Assert::IsTrue(a==0xf0cf);             
			bit_collection_reference<u32> lo(&a,0,8);
			bit_collection_reference<u32> hi(&a,8,8);
			hi = lo;
			Assert::IsTrue(a==0xcfcf);
		}             

          /*
static int bit_collection_reference_streaming(){
	u32 a = 0x5555;
	auto x = bit_collection_reference<u32>(&a,1,4);
	auto s = x.to_stream();
	s.begin();
	Assert::IsTrue(s.read() == 4);
	Assert::IsTrue(!s.next());   
	Assert::IsTrue( s.next());   
	Assert::IsTrue(!s.next());   
	Assert::IsTrue( s.next());   
	s.end();
	return 0;
}           */

	};
}
