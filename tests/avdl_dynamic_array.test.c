#include "avdl_dynamic_array.h"
#include "avdl_std.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static int avdlStdFailures = 0;

void *custom_malloc(size_t size) {
	if (avdlStdFailures > 0) {
		avdlStdFailures--;
		return 0;
	}
	return malloc(size);
}

void *custom_realloc(void *ptr, size_t size) {
	if (avdlStdFailures > 0) {
		avdlStdFailures--;
		return 0;
	}
	return realloc(ptr, size);
}

int main(int argc, char *argv[]) {

	struct dd_dynamic_array array;
	int *exampleReturn;

	// initialise array
	assert(dd_da_init(&array, sizeof(int)));

	// add example single element
	int singleElement = 4;
	assert(dd_da_push(&array, &singleElement));
	exampleReturn = dd_da_get(&array, 0);
	int expectedArray_0[1] = { 4 };
	assert(dd_da_count(&array) == 1);
	assert(exampleReturn);
	assert(*exampleReturn == expectedArray_0[0]);

	// add example array
	int arrayToAdd[4] = { 3, 5, 7, 4 };
	assert(dd_da_add(&array, arrayToAdd, 4, -1));
	assert(dd_da_count(&array) == 5);
	int expectedArray_1[5] = { 4, 3, 5, 7, 4 };
	for (int i = 0; i < 5; i++) {
		exampleReturn = dd_da_get(&array, i);
		assert(exampleReturn);
		assert(*exampleReturn == expectedArray_1[i]);
	}

	// add elements in random places
	int randomElement1 = 9;
	assert(dd_da_add(&array, &randomElement1, 1, 0));
	int randomElement2 = 13;
	assert(dd_da_add(&array, &randomElement2, 1, 2));
	int randomElement3 = 16;
	assert(dd_da_add(&array, &randomElement3, 1, -1));
	assert(dd_da_count(&array) == 8);
	int expectedArray_2[8] = { 9, 4, 13, 3, 5, 7, 4, 16 };
	for (int i = 0; i < 8; i++) {
		exampleReturn = dd_da_get(&array, i);
		assert(exampleReturn);
		assert(*exampleReturn == expectedArray_2[i]);
	}

	// add array in random place
	int arrayToAdd_2[2] = { 1, 12 };
	assert(dd_da_add(&array, arrayToAdd_2, 2, 3));
	assert(dd_da_count(&array) == 10);
	int expectedArray_3[10] = { 9, 4, 13, 1, 12, 3, 5, 7, 4, 16 };
	for (int i = 0; i < 10; i++) {
		exampleReturn = dd_da_get(&array, i);
		assert(exampleReturn);
		assert(*exampleReturn == expectedArray_3[i]);
	}

	// remove random elements from array
	assert(dd_da_remove(&array, 1, 0));
	assert(dd_da_remove(&array, 2, 3));
	assert(dd_da_pop(&array));
	assert(dd_da_count(&array) == 6);
	int expectedArray_4[6] = { 4, 13, 1, 5, 7, 4 };
	for (int i = 0; i < 6; i++) {
		exampleReturn = dd_da_get(&array, i);
		assert(exampleReturn);
		assert(*exampleReturn == expectedArray_4[i]);
	}
	exampleReturn = dd_da_get(&array, -1);
	assert(*exampleReturn == 4);

	// error calls
	exampleReturn = dd_da_get(&array, 8);
	assert(!exampleReturn);

	assert(!dd_da_remove(&array, 4, 3));
	assert(!dd_da_remove(&array, 8, -1));

	// remove rest elements from array
	assert(dd_da_remove(&array, 3, 0));
	assert(dd_da_remove(&array, 2, 1));
	assert(dd_da_pop(&array));
	assert(dd_da_count(&array) == 0);
	assert(!dd_da_pop(&array));

	// stress test #1
	int arrayToAdd_3[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	for (int i = 0; i < 100; i++) {
		dd_da_add(&array, arrayToAdd_3, 10, -1);
	}
	assert(dd_da_count(&array) == 1000);
	for (int i = 0; i < 1000; i++) {
		exampleReturn = dd_da_get(&array, i);
		assert(exampleReturn);
		assert(*exampleReturn == arrayToAdd_3[i%10]);
	}
	for (int i = 0; i < 100; i++) {
		assert(dd_da_remove(&array, 10, 0));
	}
	assert(dd_da_count(&array) == 0);

	// stress test #2
	for (int i = 0; i < 100; i++) {
		dd_da_push(&array, arrayToAdd_3 +(i%10));
	}
	assert(dd_da_count(&array) == 100);
	for (int i = 0; i < 100; i++) {
		exampleReturn = dd_da_get(&array, i);
		assert(exampleReturn);
		assert(*exampleReturn == arrayToAdd_3[i%10]);
	}
	for (int i = 0; i < 100; i++) {
		assert(dd_da_pop(&array));
	}
	assert(dd_da_count(&array) == 0);

	// free array
	dd_da_free(&array);

	// edge cases
	avdl_malloc = custom_malloc;
	avdl_realloc = custom_realloc;

	// malloc failure
	avdlStdFailures = 1;
	assert(dd_da_init(&array, sizeof(int)));
	int expectedArray_5[10] = { 2, 6, 52, 11, 32, 93, 6, 26, 84, 66 };
	assert(!dd_da_add(&array, expectedArray_5, 10, 0));
	dd_da_free(&array);

	// realloc failure
	assert(dd_da_init(&array, sizeof(int)));
	assert(dd_da_add(&array, expectedArray_5, 10, 0));
	assert(dd_da_add(&array, expectedArray_5, 10, 0));
	avdlStdFailures = 1;
	assert(!dd_da_add(&array, expectedArray_5, 10, 0));
	dd_da_free(&array);

	return 0;
}
