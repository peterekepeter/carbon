          
#include "testing.h"
#include <exception> 
#include "../BinseqLib/bit_reference.hpp"
using namespace binseq;

static int bit_reference_test0(){
	int a = 7;
	bit_reference x(&a,1);
	ASSERT(x.test());
	x = 0;
	ASSERT(a == 5);
	return 0;
}

static int bit_reference_test1(){
	int a = 3;
	bit_reference x(&a,1);
	bit_reference y(&a,2);
	int b = x;
	ASSERT(b);
	int c = y;
	ASSERT(!c);
	return 0;
}

static int bit_reference_test2(){   
	int a = 3;               
	bit_reference v(&a,0);
	bit_reference x(&a,1);
	bit_reference y(&a,2);
	y=x;
	ASSERT(a == 7);
	v=x=y=0;
	ASSERT(a == 0);
	return 0;
}

int bit_reference_tests(){
	TEST_BEGIN();                         
	TEST_RUN(bit_reference_test0());                                  
	TEST_RUN(bit_reference_test1());                                    
	TEST_RUN(bit_reference_test2());
	TEST_END("bit reference");
}