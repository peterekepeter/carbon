           
#include "testing.h"
#include "../binseq_lib/binseq.hpp"   
#include "../binseq_lib/bit_collection.hpp"
#include <exception>

using namespace binseq;

static int bit_collection_compare(){
	u32 a = 0x0ff0;
	auto x = bit_collection<u32>(a,4,4);
	auto y = bit_collection<u32>(a,8,4);
	ASSERT(x == y);                  
	y = bit_collection<u32>(a,9,4);
	ASSERT(x > y);                     
	x=y;
	y = bit_collection<u32>(a,10,4);
	ASSERT(x > y);                  
	x=y;
	y = bit_collection<u32>(a,11,4);
	ASSERT(x > y);                 
	x=y;
	y = bit_collection<u32>(a,12,4);
	ASSERT(x > y);
	x=y; 
	y = bit_collection<u32>(a,13,4);
	ASSERT(x == y);
	return 0;
}


static int bit_collection_typecast(){
	u32 a=255;
	{ bit_collection<u32> x(a,0,4); ASSERT(x == 15); } 
	{ bit_collection<u64> x(a,1,5); ASSERT(x == 31); }      
	{ bit_collection<u64> x(a,4,4); ASSERT(x == 15); }      
	{ bit_collection<u16> x(a,5,4); ASSERT(x == 7); }  
	{ bit_collection<u16> x(a,6,4); ASSERT(x == 3); }   
	{ bit_collection<u32> x(a,7,4); ASSERT(x == 1); } 
	{ bit_collection<u32> x(a,8,4); ASSERT(x == 0); }   
	{ bit_collection<u32> x(a,0,16); ASSERT(x == 255); }
	return 0;
}


static int bit_collection_semantics(){
	bit_collection<u32> a = bit_collection<u32>(3,0,2);
	bit_collection<u32> b = a;
	ASSERT(b.size == 2);     //make sure size info is not lost
	ASSERT(a == b); //make sure they are equal
	a[0] = false;
	ASSERT(a != b); //they should not be equal
	bit_collection<u32> c = bit_collection<u32>(3,0,3);
	ASSERT(b != c); // different size;
	return 0;
}

int bit_collection_tests(){
	TEST_BEGIN();                       
	TEST_RUN(bit_collection_compare());      
	TEST_RUN(bit_collection_semantics());     
	TEST_RUN(bit_collection_typecast());
	TEST_END("bit collection");  
}
            