#pragma once

#include <cstdio>

#define ASSERT(E) {if(!(E)){std::fprintf(stderr,"assert fail line %d file %s\n", __LINE__, __FILE__);return 1;}}      
#define ASSERT_EXCEPTION(ST,EX) { bool c=1;try{ST;}catch(EX e){c=0;}{if(c){std::fprintf(stderr,"assert fail line %d file %s\n", __LINE__, __FILE__);return 1;}} }
#define TEST_BEGIN() int pass=0,fail=0
#define TEST_RUN(A) {if(A){fail++;}else{pass++;}}
#define TEST_END(S) {std::printf("%s tests %d passed %d failed\n",S,pass,fail);return fail;}
