#include "binseq.hpp"
#include <cstring>
#include <stdexcept>
#include "bit_collection.hpp"

namespace binseq{
                                                          
    bool operator == ( bit_sequence&a,  bit_sequence&b)
    {
      if (a.size() != b.size()) 
        return false;
      auto bits = a.size();
      auto p64 = reinterpret_cast<u64*>(a.address());
      auto q64 = reinterpret_cast<u64*>(b.address());
      unsigned i=0;
      while (bits>64) { 
        if (p64[i]!=q64[i]) 
          return false; 
        bits-=64; 
        i++; 
      }
      auto p = bit_collection_reference<u64>(p64+i,0,u8(bits));        
      auto q = bit_collection_reference<u64>(q64+i,0,u8(bits));    
      return p==q;
    }                                        

    bool operator != (bit_sequence& a,  bit_sequence& b)
    {
      if (a.size() != b.size()) 
        return true;
      auto bits = a.size();
      auto p64 = reinterpret_cast<u64*>(a.address());
      auto q64 = reinterpret_cast<u64*>(b.address());
      unsigned i=0;
      while (bits>64) { 
        if (p64[i]!=q64[i]) 
          return true; 
        bits-=64; 
        i++; 
      }
      auto p = bit_collection_reference<u64>(p64+i,0,u8(bits));         
      auto q = bit_collection_reference<u64>(q64+i,0,u8(bits));    
      return p!=q;
    }      

    bool operator > ( bit_sequence& a,  bit_sequence& b)
    {
      if (a.size() != b.size()) 
        throw std::logic_error("can't compare sequences of different length");
      auto bits = a.size();        
      auto p64 = reinterpret_cast<u64*>(a.address());
      auto q64 = reinterpret_cast<u64*>(b.address());                
      int i = int(bits>>6);
      if (i<<6!=bits)
      {
        u64 av = bit_collection_reference<u64>(p64+i,0,u8(bits-(i<<6)));   
        u64 bv = bit_collection_reference<u64>(q64+i,0,u8(bits-(i<<6)));   
        if (av == bv){
          i--;
        } else if (av > bv) {  
          return true;
        } else { 
          return false;
        }
      }
      while (i>=0)
      {
        auto av = p64[i];
        auto bv = q64[i];
        if (av == bv) {
          i--;
        } else if (av > bv){
          return true; 
        } else {
          return false;
        }
      }
      return false;
    }

    bool operator < ( bit_sequence& a,  bit_sequence& b) 
    {
      if (a.size() != b.size()) 
        throw std::logic_error("can't compare sequences of different length");
      auto bits = a.size();        
      auto p64 = reinterpret_cast<u64*>(a.address());
      auto q64 = reinterpret_cast<u64*>(b.address());                 
      int i = int(bits>>6);
      if (i<<6!=bits)
      {
        u64 av = bit_collection_reference<u64>(p64+i,0,u8(bits-(i<<6)));    
        u64 bv = bit_collection_reference<u64>(q64+i,0,u8(bits-(i<<6)));   
        if (av == bv){
          i--;
        } else if (av < bv) {  
          return true;
        } else { 
          return false;
        }
      }
      while (i>=0)
      {
        auto av = p64[i];
        auto bv = q64[i];
        if (av == bv) {
          i--;
        } else if (av < bv){
          return true; 
        } else {
          return false;
        }
      }
      return false;
    }

    bool operator >= ( bit_sequence& a,  bit_sequence& b)
    {
      if (a.size() != b.size()) 
        throw std::logic_error("can't compare sequences of different length");
      auto bits = a.size();        
      auto p64 = reinterpret_cast<u64*>(a.address());
      auto q64 = reinterpret_cast<u64*>(b.address());            
      int i = int(bits>>6);
      if (i<<6!=bits)
      {
        u64 av = bit_collection_reference<u64>(p64+i,0,u8(bits-(i<<6)));      
        u64 bv = bit_collection_reference<u64>(q64+i,0,u8(bits-(i<<6)));   
        if (av == bv){
          i--;
        } else if (av > bv) {  
          return true;
        } else { 
          return false;
        }
      }
      while (i>=0)
      {
        auto av = p64[i];
        auto bv = q64[i];
        if (av == bv) {
          i--;
        } else if (av > bv){
          return true; 
        } else {
          return false;
        }
      }
      return true;
    }

    bool operator <= ( bit_sequence& a,  bit_sequence& b) 
    {
      if (a.size() != b.size()) 
        throw std::logic_error("can't compare sequences of different length");
      auto bits = a.size();        
      auto p64 = reinterpret_cast<u64*>(a.address());
      auto q64 = reinterpret_cast<u64*>(b.address());              
      int i = int(bits>>6);
      if (i<<6!=bits)
      {
        u64 av = bit_collection_reference<u64>(p64+i,0,u8(bits-(i<<6)));   
        u64 bv = bit_collection_reference<u64>(q64+i,0,u8(bits-(i<<6)));   
        if (av == bv){
          i--;
        } else if (av < bv) {  
          return true;
        } else { 
          return false;
        }
      }
      while (i>=0)
      {
        auto av = p64[i];
        auto bv = q64[i];
        if (av == bv) {
          i--;
        } else if (av < bv){
          return true; 
        } else {
          return false;
        }
      }
      return true;
    }

    bit_sequence operator + ( bit_sequence& a,  bit_sequence& b){
      bit_sequence c;
      c.reallocate(a.size() + b.size());
      auto a64 = reinterpret_cast<u64*>(a.address());
      auto b64 = reinterpret_cast<u64*>(b.address());
      auto c64 = reinterpret_cast<u64*>(c.address());
      auto cap = (a.size()+63)>>6;
      for (unsigned i = 0; i<cap; i++) {
        c64[i] = a64[i]; 
      }
      if ((a.size()&7)==0){ //if byte aligned
        auto d8 = reinterpret_cast<u8*>(c.address());
        auto dst = reinterpret_cast<u64*>(&d8[a.size()>>3]); 
        auto cap = (b.size()+63)>>6;
        for (unsigned i=0; i<cap; i++){
          dst[i] = b64[i];
        }
      } else { // not aligned ... a little bit harder    
        u8 misalign = a.size()&7;
        u8 inv_misalign = sizeof(u64)*8 - misalign;
        auto d8 = reinterpret_cast<u8*>(c.address());
        auto dst = reinterpret_cast<u64*>(&d8[a.size()>>3]); //points to the last u64
        u64 carry = dst[0];      
        unsigned i;                 
        auto cap = (b.size()+63)>>6;
        for (i=0; i<cap; i++){
          u64 loaded = b64[i];          
          dst[i] = carry | (loaded<<misalign); 
          carry = loaded>>inv_misalign; 
        }
        auto last_byte = reinterpret_cast<u8*>(&dst[i]);
        last_byte[0] = (u8)carry;
      }
      return c;
    }
                  
    bit_sequence subseq(bit_sequence& a,u64 offset, u64 size) {
      if (offset==0)
      {
        if(size == a.size())
          return bit_sequence(a); //just copy this
      }
      bit_sequence b;
      b.reallocate(size);     
      auto a64 = reinterpret_cast<u64*>(a.address());
      auto b64 = reinterpret_cast<u64*>(b.address());

      u8 misalign = offset&7;
      u8 inv_misalign = sizeof(u64)*8 - misalign;
      auto d8 = reinterpret_cast<u8*>(a.address());
      auto src = reinterpret_cast<u64*>(&d8[offset>>3]); //points to the last u64
      u64 carry = src[0]>>misalign;      
      unsigned i;
      auto cap = (b.size()+63)>>6;
      for (i=0; i<cap; i++){
        u64 loaded = src[i+1];          
        b64[i] = carry | (loaded<<inv_misalign); 
        carry = loaded>>misalign; 
      }
      b64[i-1]&=~(u64(-1)<<(size&63));
      return b;
    }

    bit_sequence head (bit_sequence& a,u64 size){
      return subseq(a,0,size);
    }
    bit_sequence tail (bit_sequence& a,u64 size){
      return subseq(a,size,a.size()-size);
    }

    bit_sequence repeat(bit_sequence& a,u64 size){
      bit_sequence b = a;
      //cheap implementation
      //TODO reimplement properly
      while (true){
        auto bs = b.size();
        if (bs<size){
          if (bs*2<size&&bs*3>size) {
            b = b+b+b;
          } else {
            b = b+b;
          }
        } else {
          break;
        }
      }
      b = head(b,size);
      return b;
    }

    bit_sequence _not (bit_sequence& a){
      bit_sequence b;
      b.reallocate(a.size());
      auto a64 = reinterpret_cast<u64*>(a.address());
      auto b64 = reinterpret_cast<u64*>(b.address()); 

      auto cap = (b.size()+63)>>6;
      for (unsigned i=0; i<cap; i++){
        b64[i] = ~a64[i];
      }
      return b;
    }

    bit_sequence _and ( bit_sequence& a,  bit_sequence& b){
      if (a.size() != b.size()) 
        throw std::logic_error("can't compare sequences of different length");
      bit_sequence c;
      c.reallocate(a.size());
      auto a64 = reinterpret_cast<u64*>(a.address());
      auto b64 = reinterpret_cast<u64*>(b.address()); 
      auto c64 = reinterpret_cast<u64*>(c.address()); 
                                   
      auto cap = (b.size()+63)>>6;
      for (unsigned i=0; i<cap; i++){
        c64[i] = a64[i] & b64[i];
      }
      return c;
    }

    bit_sequence  _or ( bit_sequence& a,  bit_sequence& b){
      if (a.size() != b.size()) 
        throw std::logic_error("can't compare sequences of different length");
      bit_sequence c;
      c.reallocate(a.size());
      auto a64 = reinterpret_cast<u64*>(a.address());
      auto b64 = reinterpret_cast<u64*>(b.address());   
      auto c64 = reinterpret_cast<u64*>(c.address()); 
                                    
      auto cap = (b.size()+63)>>6;
      for (unsigned i=0; i<cap; i++){
        c64[i] = a64[i] | b64[i];
      }
      return c;
    }

    bit_sequence _xor ( bit_sequence& a,  bit_sequence& b){
      if (a.size() != b.size()) 
        throw std::logic_error("can't compare sequences of different length");
      bit_sequence c;
      c.reallocate(a.size());
      auto a64 = reinterpret_cast<u64*>(a.address());
      auto b64 = reinterpret_cast<u64*>(b.address());   
      auto c64 = reinterpret_cast<u64*>(c.address()); 
                                   
      auto cap = (b.size()+63)>>6;
      for (unsigned i=0; i<cap; i++){
        c64[i] = a64[i] ^ b64[i];
      }
      return c;
    }
    bit_sequence nand ( bit_sequence& a,  bit_sequence& b){
      if (a.size() != b.size()) 
        throw std::logic_error("can't compare sequences of different length");
      bit_sequence c;
      c.reallocate(a.size());
      auto a64 = reinterpret_cast<u64*>(a.address());
      auto b64 = reinterpret_cast<u64*>(b.address());  
      auto c64 = reinterpret_cast<u64*>(c.address()); 
                                    
      auto cap = (b.size()+63)>>6;
      for (unsigned i=0; i<cap; i++){
        c64[i] = ~(a64[i] & b64[i]);
      }
      return c;
    }

    bit_sequence nor ( bit_sequence& a,  bit_sequence& b){
      if (a.size() != b.size()) 
        throw std::logic_error("can't compare sequences of different length");
      bit_sequence c;
      c.reallocate(a.size());
      auto a64 = reinterpret_cast<u64*>(a.address());
      auto b64 = reinterpret_cast<u64*>(b.address()); 
      auto c64 = reinterpret_cast<u64*>(c.address()); 
                                    
      auto cap = (b.size()+63)>>6;
      for (unsigned i=0; i<cap; i++){
        c64[i] = ~(a64[i] | b64[i]);
      }
      return c;
    }

    bit_sequence nxor ( bit_sequence& a,  bit_sequence& b){
      if (a.size() != b.size()) 
        throw std::logic_error("can't compare sequences of different length");
      bit_sequence c;
      c.reallocate(a.size());
      auto a64 = reinterpret_cast<u64*>(a.address());
      auto b64 = reinterpret_cast<u64*>(b.address()); 
      auto c64 = reinterpret_cast<u64*>(c.address());  
                                   
      auto cap = (b.size()+63)>>6;
      for (unsigned i=0; i<cap; i++){
        c64[i] = ~(a64[i] ^ b64[i]);
      }
      return c;
    } 

    bit_sequence andc ( bit_sequence& a,  bit_sequence& b){
      auto as = a.size();
      auto bs = b.size();
      auto minsize = as < bs ? as : bs;
      bit_sequence c;
      c.reallocate(minsize);
      auto a64 = reinterpret_cast<u64*>(a.address());
      auto b64 = reinterpret_cast<u64*>(b.address());   
      auto c64 = reinterpret_cast<u64*>(c.address());
      auto cap = (minsize+63)>>6;
      for (unsigned i=0; i<cap; i++){
        c64[i] = a64[i] & b64[i];
      }
      return c;
    }
    
    bit_sequence  orc ( bit_sequence& a,  bit_sequence& b){
      auto as = a.size();
      auto bs = b.size();
      auto minsize = as < bs ? as : bs;
      bit_sequence c;
      c.reallocate(minsize);
      auto a64 = reinterpret_cast<u64*>(a.address());
      auto b64 = reinterpret_cast<u64*>(b.address()); 
      auto c64 = reinterpret_cast<u64*>(c.address());
      auto cap = (minsize+63)>>6;
      for (unsigned i=0; i<cap; i++){
        c64[i] = a64[i] | b64[i];
      }
      return c;
    }

    bit_sequence xorc ( bit_sequence& a,  bit_sequence& b){
      auto as = a.size();
      auto bs = b.size();
      auto minsize = as < bs ? as : bs;
      bit_sequence c;
      c.reallocate(minsize);
      auto a64 = reinterpret_cast<u64*>(a.address());
      auto b64 = reinterpret_cast<u64*>(b.address());  
      auto c64 = reinterpret_cast<u64*>(c.address());
      auto cap = (minsize+63)>>6;
      for (unsigned i=0; i<cap; i++){
        c64[i] = a64[i] ^ b64[i];
      }
      return c;
    }
 
    bit_sequence nandc ( bit_sequence& a,  bit_sequence& b){
      auto as = a.size();
      auto bs = b.size();
      auto minsize = as < bs ? as : bs;
      bit_sequence c;
      c.reallocate(minsize);
      auto a64 = reinterpret_cast<u64*>(a.address());
      auto b64 = reinterpret_cast<u64*>(b.address()); 
      auto c64 = reinterpret_cast<u64*>(c.address());
      auto cap = (minsize+63)>>6;
      for (unsigned i=0; i<cap; i++){
        c64[i] = ~(a64[i] & b64[i]);
      }
      return c;
    }
        
    bit_sequence norc ( bit_sequence& a,  bit_sequence& b){
      auto as = a.size();
      auto bs = b.size();
      auto minsize = as < bs ? as : bs;
      bit_sequence c;
      c.reallocate(minsize);
      auto a64 = reinterpret_cast<u64*>(a.address());
      auto b64 = reinterpret_cast<u64*>(b.address());  
      auto c64 = reinterpret_cast<u64*>(c.address());
      auto cap = (minsize+63)>>6;
      for (unsigned i=0; i<cap; i++){
        c64[i] = ~(a64[i] | b64[i]);
      }
      return c;
    }
     
    bit_sequence nxorc ( bit_sequence& a,  bit_sequence& b){
      auto as = a.size();
      auto bs = b.size();
      auto minsize = as < bs ? as : bs;
      bit_sequence c;
      c.reallocate(minsize);
      auto a64 = reinterpret_cast<u64*>(a.address());
      auto b64 = reinterpret_cast<u64*>(b.address());   
      auto c64 = reinterpret_cast<u64*>(c.address());
      auto cap = (minsize+63)>>6;
      for (unsigned i=0; i<cap; i++){
        c64[i] = ~(a64[i] ^ b64[i]);
      }
      return c;
    }
 
    bit_sequence andr ( bit_sequence& _a,  bit_sequence& _b){     
      auto _as = _a.size();
      auto _bs = _b.size();
      auto _cond = _as<_bs;
      auto& a = _cond?_b:_a;  
      auto& b = _cond?_a:_b;
      auto as = _cond?_bs:_as;  
      auto bs = _cond?_as:_bs;

      if (64 % bs == 0){
        u64 word = *reinterpret_cast<u64*>(b.address());
        word &= ~(u64(-1)<<bs);
        if (bs==1) word |= word<<1, bs <<=1;
        if (bs==2) word |= word<<2, bs <<=1;
        if (bs==4) word |= word<<4, bs <<=1;   
        if (bs==8) word |= word<<8, bs <<=1;   
        if (bs==16) word |= word<<16, bs <<=1; 
        if (bs==32) word |= word<<32, bs <<=1;
        bit_sequence c;
        c.reallocate(as);
                                      
        auto a64 = reinterpret_cast<u64*>(a.address());
        auto c64 = reinterpret_cast<u64*>(c.address());
        u32 cap = (as+63)>>6;
        for (unsigned i=0; i<cap; i++){
          c64[i] = a64[i] & word;
        }
        return c;
      } else {
        return _and(a,repeat(b,as));
      }
    }

    bit_sequence  orr ( bit_sequence& _a,  bit_sequence& _b){     
      auto _as = _a.size();
      auto _bs = _b.size();
      auto _cond = _as<_bs;
      auto& a = _cond?_b:_a;  
      auto& b = _cond?_a:_b;
      auto as = _cond?_bs:_as;  
      auto bs = _cond?_as:_bs;

      if (64 % bs == 0){
        u64 word = *reinterpret_cast<u64*>(b.address());
        word &= ~(u64(-1)<<bs);
        if (bs==1) word |= word<<1, bs <<=1;
        if (bs==2) word |= word<<2, bs <<=1;
        if (bs==4) word |= word<<4, bs <<=1;   
        if (bs==8) word |= word<<8, bs <<=1;   
        if (bs==16) word |= word<<16, bs <<=1; 
        if (bs==32) word |= word<<32, bs <<=1;
        bit_sequence c;
        c.reallocate(as);
                                      
        auto a64 = reinterpret_cast<u64*>(a.address());
        auto c64 = reinterpret_cast<u64*>(c.address());
        u32 cap = (as+63)>>6;
        for (unsigned i=0; i<cap; i++){
          c64[i] = a64[i] | word;
        }
        return c;
      } else {
        return _or(a,repeat(b,as));
      }
    }
      
    bit_sequence xorr ( bit_sequence& _a,  bit_sequence& _b){     
      auto _as = _a.size();
      auto _bs = _b.size();
      auto _cond = _as<_bs;
      auto& a = _cond?_b:_a;  
      auto& b = _cond?_a:_b;
      auto as = _cond?_bs:_as;  
      auto bs = _cond?_as:_bs;

      if (64 % bs == 0){
        u64 word = *reinterpret_cast<u64*>(b.address());
        word &= ~(u64(-1)<<bs);
        if (bs==1) word |= word<<1, bs <<=1;
        if (bs==2) word |= word<<2, bs <<=1;
        if (bs==4) word |= word<<4, bs <<=1;   
        if (bs==8) word |= word<<8, bs <<=1;   
        if (bs==16) word |= word<<16, bs <<=1; 
        if (bs==32) word |= word<<32, bs <<=1;
        bit_sequence c;
        c.reallocate(as);
                                      
        auto a64 = reinterpret_cast<u64*>(a.address());
        auto c64 = reinterpret_cast<u64*>(c.address());
        u32 cap = (as+63)>>6;
        for (unsigned i=0; i<cap; i++){
          c64[i] = a64[i] ^ word;
        }
        return c;
      } else {
        return _xor(a,repeat(b,as));
      }
    }

    bit_sequence nandr ( bit_sequence& _a,  bit_sequence& _b){     
      auto _as = _a.size();
      auto _bs = _b.size();
      auto _cond = _as<_bs;
      auto& a = _cond?_b:_a;  
      auto& b = _cond?_a:_b;
      auto as = _cond?_bs:_as;  
      auto bs = _cond?_as:_bs;

      if (64 % bs == 0){
        u64 word = *reinterpret_cast<u64*>(b.address());
        word &= ~(u64(-1)<<bs);
        if (bs==1) word |= word<<1, bs <<=1;
        if (bs==2) word |= word<<2, bs <<=1;
        if (bs==4) word |= word<<4, bs <<=1;   
        if (bs==8) word |= word<<8, bs <<=1;   
        if (bs==16) word |= word<<16, bs <<=1; 
        if (bs==32) word |= word<<32, bs <<=1;
        bit_sequence c;
        c.reallocate(as);
                                      
        auto a64 = reinterpret_cast<u64*>(a.address());
        auto c64 = reinterpret_cast<u64*>(c.address());
        u32 cap = (as+63)>>6;
        for (unsigned i=0; i<cap; i++){
          c64[i] = ~(a64[i] & word);
        }
        return c;
      } else {
        return nand(a,repeat(b,as));
      }
    }
         
    bit_sequence norr ( bit_sequence& _a,  bit_sequence& _b){     
      auto _as = _a.size();
      auto _bs = _b.size();
      auto _cond = _as<_bs;
      auto& a = _cond?_b:_a;  
      auto& b = _cond?_a:_b;
      auto as = _cond?_bs:_as;  
      auto bs = _cond?_as:_bs;

      if (64 % bs == 0){
        u64 word = *reinterpret_cast<u64*>(b.address());
        word &= ~(u64(-1)<<bs);
        if (bs==1) word |= word<<1, bs <<=1;
        if (bs==2) word |= word<<2, bs <<=1;
        if (bs==4) word |= word<<4, bs <<=1;   
        if (bs==8) word |= word<<8, bs <<=1;   
        if (bs==16) word |= word<<16, bs <<=1; 
        if (bs==32) word |= word<<32, bs <<=1;
        bit_sequence c;
        c.reallocate(as);
                                      
        auto a64 = reinterpret_cast<u64*>(a.address());
        auto c64 = reinterpret_cast<u64*>(c.address());
        u32 cap = (as+63)>>6;
        for (unsigned i=0; i<cap; i++){
          c64[i] = ~(a64[i] | word);
        }
        return c;
      } else {
        return nor(a,repeat(b,as));
      }
    }
       
    bit_sequence nxorr ( bit_sequence& _a,  bit_sequence& _b){     
      auto _as = _a.size();
      auto _bs = _b.size();
      auto _cond = _as<_bs;
      auto& a = _cond?_b:_a;  
      auto& b = _cond?_a:_b;
      auto as = _cond?_bs:_as;  
      auto bs = _cond?_as:_bs;

      if (64 % bs == 0){
        u64 word = *reinterpret_cast<u64*>(b.address());
        word &= ~(u64(-1)<<bs);
        if (bs==1) word |= word<<1, bs <<=1;
        if (bs==2) word |= word<<2, bs <<=1;
        if (bs==4) word |= word<<4, bs <<=1;   
        if (bs==8) word |= word<<8, bs <<=1;   
        if (bs==16) word |= word<<16, bs <<=1; 
        if (bs==32) word |= word<<32, bs <<=1;
        bit_sequence c;
        c.reallocate(as);
                                      
        auto a64 = reinterpret_cast<u64*>(a.address());
        auto c64 = reinterpret_cast<u64*>(c.address());
        u32 cap = (as+63)>>6;
        for (unsigned i=0; i<cap; i++){
          c64[i] = ~(a64[i] ^ word);
        }
        return c;
      } else {
        return nxor(a,repeat(b,as));
      }
    }

}

