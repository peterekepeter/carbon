#include "stdafx.h"
#include "CppUnitTest.h"
#include <exception> 
#include "../BinseqLib/bit_reference.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace binseq;

namespace UnitTestBinseqLib
{
	TEST_CLASS(UnitTest1)
	{
	public:

		TEST_METHOD(BitReferenceTest0)
		{
			int a = 7;
			bit_reference x(&a,1);
			Assert::IsTrue(x.test());
			x = 0;
			Assert::IsTrue(a == 5);
		}

		TEST_METHOD(BitReferenceTest1)
		{
			int a = 3;
			bit_reference x(&a,1);
			bit_reference y(&a,2);
			int b = x;
			Assert::IsTrue(b);
			int c = y;
			Assert::IsTrue(!c);
		}

		TEST_METHOD(BitReferenceTest2)
		{
			int a = 3;               
			bit_reference v(&a,0);
			bit_reference x(&a,1);
			bit_reference y(&a,2);
			y=x;
			Assert::IsTrue(a == 7);
			v=x=y=0;
			Assert::IsTrue(a == 0);
		}

	};
}