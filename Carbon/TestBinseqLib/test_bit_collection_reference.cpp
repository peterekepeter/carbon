                     
#include "testing.h"
#include <exception>
#include "../BinseqLib/bit_collection.hpp"

using namespace binseq;

static int bit_collection_reference_array(){
	u32 a=7;
	bit_collection_reference<u32> y(&a,0,4);
	ASSERT(y[0] == true);           
	ASSERT(y[1] == true);
	ASSERT(y[2] == true);
	ASSERT(y[3] == false);      
	bit_collection_reference<u32> x(&a,2,4);
	ASSERT(x[0] == true);     
	ASSERT(x[1] == false);
	ASSERT(x[2] == false);
	ASSERT(x[3] == false);
	return 0 ;
}

static int bit_collection_reference_typecast(){
	u32 a=255;
	{ bit_collection_reference<u32> x(&a,0,4); ASSERT(x == 15); } 
	{ bit_collection_reference<u32> x(&a,1,5); ASSERT(x == 31); }      
	{ bit_collection_reference<u32> x(&a,4,4); ASSERT(x == 15); }      
	{ bit_collection_reference<u32> x(&a,5,4); ASSERT(x == 7); }  
	{ bit_collection_reference<u32> x(&a,6,4); ASSERT(x == 3); }   
	{ bit_collection_reference<u32> x(&a,7,4); ASSERT(x == 1); } 
	{ bit_collection_reference<u32> x(&a,8,4); ASSERT(x == 0); }   
	{ bit_collection_reference<u32> x(&a,0,16); ASSERT(x == 255); }
	return 0;
}

static int bit_collection_reference_assign(){
	u32 a=0xffff;
	bit_collection_reference<u32> x(&a,4,8);
	x = 0;                
	ASSERT(a==0xf00f);
	x = 3;              
	ASSERT(a==0xf03f);
	x = 12;             
	ASSERT(a==0xf0cf);             
	bit_collection_reference<u32> lo(&a,0,8);
	bit_collection_reference<u32> hi(&a,8,8);
	hi = lo;
	ASSERT(a==0xcfcf);
	return 0;
}             
          /*
static int bit_collection_reference_streaming(){
	u32 a = 0x5555;
	auto x = bit_collection_reference<u32>(&a,1,4);
	auto s = x.to_stream();
	s.begin();
	ASSERT(s.read() == 4);
	ASSERT(!s.next());   
	ASSERT( s.next());   
	ASSERT(!s.next());   
	ASSERT( s.next());   
	s.end();
	return 0;
}           */
             
int bit_collection_reference_tests(){
	TEST_BEGIN();                         
	TEST_RUN(bit_collection_reference_array());  
	TEST_RUN(bit_collection_reference_typecast());   
	//TEST_RUN(bit_collection_reference_streaming());
	TEST_RUN(bit_collection_reference_assign());
	TEST_END("bit collection reference");  
}