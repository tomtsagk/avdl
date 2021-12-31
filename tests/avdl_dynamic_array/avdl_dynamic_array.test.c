#include "avdl_dynamic_array.h"
#include <assert.h>
#include <stdio.h>

int main(int argc, char *argv[]) {

	struct dd_dynamic_array array;
	int *exampleReturn;

	// initialise array
	dd_da_init(&array, sizeof(int));

	// add example element
	int singleElement = 4;
	dd_da_add(&array, &singleElement);
	exampleReturn = dd_da_get(&array, 0);
	assert(*exampleReturn == 4);
	assert(dd_da_count(&array) == 1);

	// add example array
	int arrayToAdd[2] = { 3, 5 };
	dd_da_adda(&array, arrayToAdd, 2);
	exampleReturn = dd_da_get(&array, 1);
	assert(*exampleReturn == 3);
	exampleReturn = dd_da_get(&array, 2);
	assert(*exampleReturn == 5);
	assert(dd_da_count(&array) == 3);

	// add on first place
	singleElement = 9;
	dd_da_add_first(&array, &singleElement);
	exampleReturn = dd_da_get(&array, 0);
	assert(*exampleReturn == 9);
	assert(dd_da_count(&array) == 4);

	// remove last element of array
	dd_da_pop(&array);
	assert(dd_da_count(&array) == 3);

	// remove first element of array
	dd_da_remove(&array, 0);
	exampleReturn = dd_da_get(&array, 0);
	assert(*exampleReturn == 4);
	assert(dd_da_count(&array) == 2);

	// free array
	dd_da_free(&array);

	// initialisation with array
	dd_da_inita(&array, sizeof(int), 5);

	// free array
	dd_da_free(&array);

	return 0;
}
