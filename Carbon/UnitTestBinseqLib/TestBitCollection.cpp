
#include "stdafx.h"
#include "CppUnitTest.h"
#include "../BinseqLib/bit_collection.hpp"

using namespace binseq;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTestBinseqLib
{
	TEST_CLASS(BinseqUnitTest)
	{
	public:

		TEST_METHOD(BitCollectionCompare) 
		{
			u32 a = 0x0ff0;
			auto x = bit_collection<u32>(a,4,4);
			auto y = bit_collection<u32>(a,8,4);
			Assert::IsTrue(x == y);                  
			y = bit_collection<u32>(a,9,4);
			Assert::IsTrue(x > y);                     
			x=y;
			y = bit_collection<u32>(a,10,4);
			Assert::IsTrue(x > y);                  
			x=y;
			y = bit_collection<u32>(a,11,4);
			Assert::IsTrue(x > y);                 
			x=y;
			y = bit_collection<u32>(a,12,4);
			Assert::IsTrue(x > y);
			x=y; 
			y = bit_collection<u32>(a,13,4);
			Assert::IsTrue(x == y);
		}


		TEST_METHOD(BitCollectionTypecast)
		{
			u32 a=255;
			{ bit_collection<u32> x(a,0,4); Assert::IsTrue(x == 15); } 
			{ bit_collection<u64> x(a,1,5); Assert::IsTrue(x == 31); }      
			{ bit_collection<u64> x(a,4,4); Assert::IsTrue(x == 15); }      
			{ bit_collection<u16> x(a,5,4); Assert::IsTrue(x == 7); }  
			{ bit_collection<u16> x(a,6,4); Assert::IsTrue(x == 3); }   
			{ bit_collection<u32> x(a,7,4); Assert::IsTrue(x == 1); } 
			{ bit_collection<u32> x(a,8,4); Assert::IsTrue(x == 0); }   
			{ bit_collection<u32> x(a,0,16); Assert::IsTrue(x == 255); }
		}


		TEST_METHOD(BitCollectionSemantics)
		{
			bit_collection<u32> a = bit_collection<u32>(3,0,2);
			bit_collection<u32> b = a;
			Assert::IsTrue(b.size == 2);     //make sure size info is not lost
			Assert::IsTrue(a == b); //make sure they are equal
			a[0] = false;
			Assert::IsTrue(a != b); //they should not be equal
			bit_collection<u32> c = bit_collection<u32>(3,0,3);
			Assert::IsTrue(b != c); // different size;
		}

	};
}