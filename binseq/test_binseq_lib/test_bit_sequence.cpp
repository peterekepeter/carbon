       
#include "testing.h"
#include "../binseq_lib/binseq.hpp"   
#include <exception>

using namespace binseq;

static int bit_sequence_compare(){
	auto a = bit_sequence(u8(3));
	auto b = bit_sequence(u8(3));   
	auto c = bit_sequence(u8(4));
	ASSERT(a == b);              
	ASSERT(a != c);    
	ASSERT(!(a == c));   
	ASSERT(!(a != b));    
	ASSERT(b < c);    
	ASSERT(c > b);      
	ASSERT(b <= c);   
	ASSERT(c >= b);    
	ASSERT(!(b > c));  
	ASSERT(!(c < b));  
	ASSERT(!(b >= c)); 
	ASSERT(!(c <= b));
	ASSERT(a <= b); 
	ASSERT(a >= b);
	return 0;
}       

static int bit_sequence_ascii(){
	auto a = bit_sequence("hello world!");    
	auto b = bit_sequence("hello world!");
	ASSERT(a == b);                       
	auto c = bit_sequence("hello binseq");    
	ASSERT(a != c);                       
	return 0;
}

static int bit_sequence_subseq(){
	auto a = bit_sequence(u16(0x00ff));
	ASSERT(bit_sequence(u8(0xff)) == subseq(a,0,8));   
	ASSERT(bit_sequence(u8(0x00)) == subseq(a,8,8));   
	ASSERT(bit_sequence(u8(0x0f)) == subseq(a,4,8));  
	return 0;
}             

static int bit_sequence_subseq2(){
	auto a = bit_sequence(u16(0x0f0f));    
	ASSERT(subseq(a,0,4) == subseq(a,8,4));   
	ASSERT(subseq(a,1,4) == subseq(a,9,4));   
	ASSERT(subseq(a,2,4) == subseq(a,10,4)); 
	ASSERT(subseq(a,3,4) == subseq(a,11,4)); 
	ASSERT(subseq(a,4,4) == subseq(a,12,4));        
	return 0;
}           

static int bit_sequence_subseq3(){
	auto a = bit_sequence(u16(0x00ff));    
	auto b = subseq(a,6,4);
	ASSERT(b[0] == true);     
	ASSERT(b[1] == true);
	ASSERT(b[2] == false);
	ASSERT(b[3] == false);   
	return 0;
}       

static int bit_sequence_concat(){
	auto a = bit_sequence(u16(0x00ff)) + bit_sequence(u16(0xff00));    
	ASSERT(a.size()==32);
	ASSERT(bit_sequence(u32(0xff0000ff)) == a); 
	return 0;
}         

static int bit_sequence_concat2(){                          
	auto b = bit_sequence("hello") + bit_sequence("world");
	auto c = bit_sequence("helloworld");
	ASSERT(b == c);
	return 0;
}      

static int bit_sequence_concat3(){                          
	auto a = bit_sequence(u8(3));
	auto c = subseq(a,1,2) + subseq(a,0,4) + subseq(a,4,2) + subseq(a,1,2);
	ASSERT(c.size() == 10);
	ASSERT(c[0] == true);              
	ASSERT(c[1] == false);
	ASSERT(c[2] == true);    
	ASSERT(c[3] == true);              
	ASSERT(c[4] == false);           
	ASSERT(c[5] == false);           
	ASSERT(c[6] == false);           
	ASSERT(c[7] == false);           
	ASSERT(c[8] == true);         
	ASSERT(c[9] == false);
	return 0;
}

int bit_sequence_tests(){
	TEST_BEGIN();                      
	TEST_RUN(bit_sequence_compare());   
	TEST_RUN(bit_sequence_ascii());    
	TEST_RUN(bit_sequence_subseq());   
	TEST_RUN(bit_sequence_subseq2());  
	TEST_RUN(bit_sequence_subseq3());   
	TEST_RUN(bit_sequence_concat());   
	TEST_RUN(bit_sequence_concat2());   
	TEST_RUN(bit_sequence_concat3()); 
	TEST_END("bit_sequence");   
}
