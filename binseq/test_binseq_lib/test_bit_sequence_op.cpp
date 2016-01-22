       
#include "testing.h"
#include "../binseq_lib/binseq.hpp"   
#include <exception>

using namespace binseq;
    
static int bit_sequence_op_not(){
	auto a = bit_sequence(u16(0xf0f0));
	auto b = not(a);
	ASSERT(b == bit_sequence(u16(0x0f0f)));
	auto c = bit_sequence("this is what marcellus wallace looks like");
	auto d = not(c);
	auto e = not(d);
	ASSERT(c == e);
	return 0;
}

static int bit_sequence_op_basic(){
	auto a = bit_sequence(u16(0x0101));
	auto b = bit_sequence(u16(0x0011));
	ASSERT(and(a,b) == bit_sequence(u16(0x0001)));           
	ASSERT( or(a,b) == bit_sequence(u16(0x0111)));          
	ASSERT(xor(a,b) == bit_sequence(u16(0x0110)));    
	ASSERT(nand(a,b) == bit_sequence(u16(0xfffe)));           
	ASSERT( nor(a,b) == bit_sequence(u16(0xfeee)));          
	ASSERT(nxor(a,b) == bit_sequence(u16(0xfeef)));
	return 0;
}


static int bit_sequence_op_incompatible(){
	auto a = bit_sequence(u8(0xff));
	auto b = bit_sequence(u16(0xffff));
	ASSERT_EXCEPTION(and(a,b),std::exception);
	return 0;
}          

static int bit_sequence_op_clip(){
	auto a = bit_sequence(u32(0xffff0101));
	auto b = bit_sequence(u16(0x0011));
	ASSERT(andc(a,b) == bit_sequence(u16(0x0001)));           
	ASSERT( orc(a,b) == bit_sequence(u16(0x0111)));          
	ASSERT(xorc(a,b) == bit_sequence(u16(0x0110)));    
	ASSERT(nandc(a,b) == bit_sequence(u16(0xfffe)));           
	ASSERT( norc(a,b) == bit_sequence(u16(0xfeee)));          
	ASSERT(nxorc(a,b) == bit_sequence(u16(0xfeef)));
	return 0;
}

static int bit_sequence_op_repeat(){
	auto a = bit_sequence(u32(0x00110011));
	auto b = bit_sequence(u8(0x01));
	ASSERT(andr(a,b) == bit_sequence(u32(0x00010001)));           
	ASSERT( orr(a,b) == bit_sequence(u32(0x01110111)));          
	ASSERT(xorr(a,b) == bit_sequence(u32(0x01100110)));    
	ASSERT(nandr(a,b) == bit_sequence(u32(0xfffefffe)));           
	ASSERT( norr(a,b) == bit_sequence(u32(0xfeeefeee)));          
	ASSERT(nxorr(a,b) == bit_sequence(u32(0xfeeffeef)));
	return 0;
}

static int bit_sequence_op_repeat2(){
	auto a = bit_sequence(u16(0xffff));
	auto b = bit_sequence(u8(0x01));
	auto c = head(b,3);
	auto d = andr(c,a);
	ASSERT(d == bit_sequence(u16(0x9249)));
	return 0;
}

static int bit_sequence_op_repeat3(){
	auto a = bit_sequence(u8(0x0f));
	auto b = repeat(repeat(a,4),8);
	ASSERT(b == bit_sequence(u8(0xff)));
	return 0;
}
       
int bit_sequence_op_tests(){
	TEST_BEGIN();                        
	TEST_RUN(bit_sequence_op_not());     
	TEST_RUN(bit_sequence_op_basic());   
	TEST_RUN(bit_sequence_op_incompatible());   
	TEST_RUN(bit_sequence_op_clip());      
	TEST_RUN(bit_sequence_op_repeat());      
	TEST_RUN(bit_sequence_op_repeat2());     
	TEST_RUN(bit_sequence_op_repeat3()); 
	TEST_END("bit_sequence operations");   
}
