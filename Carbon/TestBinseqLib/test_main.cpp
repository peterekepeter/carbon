
#include <stdio.h>
#include "tests.h"

void test_main()
{        
	int total_fail = 0;
	total_fail += bit_reference_tests();
	total_fail += bit_collection_reference_tests();
	total_fail += bit_collection_tests();    
	total_fail += bit_sequence_tests();    
	total_fail += bit_sequence_op_tests();
	if(total_fail>0){
		printf("TOTAL %d tests have failed \a\n",total_fail); 
	}
}

int main()
{          
	test_main();
	return 0;
}
