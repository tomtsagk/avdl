#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "shared/avdl_dynamic_array.h"
#include "shared/avdl_std.h"
#include "shared/avdl_log.h"

void avdl_dynamic_array_create(struct avdl_dynamic_array *da) {
	da->array = 0;
	da->elements = 0;
	da->array_size = 0;
	da->element_size = 0;
	da->clean = avdl_dynamic_array_clean;
}

void avdl_dynamic_array_clean(struct avdl_dynamic_array *da) {
	avdl_da_free(da);
}

static int set_array_size(struct avdl_dynamic_array *da, int count) {

	da->array_size = count;
	if (!da->array) {
		da->array = avdl_malloc(da->element_size *da->array_size);

		if (!da->array) {
			return 0;
		}
	}
	else {
		void *temp = avdl_realloc(da->array, da->element_size *da->array_size);

		/* Allocation worked */
		if (temp) {
			da->array = temp;
		}
		/* Allocation failed */
		else {
			return 0;
		}
	}

	return 1;
}

/*
 * Init empty array
 */
int avdl_da_init(struct avdl_dynamic_array *da, int el_size) {
	da->element_size = el_size;
	da->elements = 0;
	da->array_size = 0;
	da->array = 0;
	return 1;
}

/*
 * Adds one element to the array
 */
int avdl_da_pushEmpty(struct avdl_dynamic_array *da) {
	return avdl_da_add(da, 0, 1, -1);
}

int avdl_da_push(struct avdl_dynamic_array *da, void *data) {
	return avdl_da_add(da, data, 1, -1);
}

int avdl_da_add(struct avdl_dynamic_array *da, const void *data, unsigned int data_count, int position) {

	/*
	 * position of negative value means add to end of array
	 */
	if (position < 0) {
		position = da->elements +(position +1);

		if (position < 0) {
			avdl_log_error("failed to add data to array: position is negative: %d", position);
			return 0;
		}
	}

	/*
	 * array doesn't exist or can't hold new data - resize it
	 */
	if (!da->array
	||  da->elements +data_count > da->array_size) {
		int newSize = da->elements > 3 ? da->elements : 3;
		while (newSize < da->elements +data_count) {
			newSize *= 2;
		}
		if (!set_array_size(da, newSize)) {
			return 0;
		}
	}

	/*
	 * move elements to make a gap for new elements
	 * if not adding to end of array
	 */
	if (position < da->elements) {
		memmove(((char*)da->array) +(da->element_size *(position +data_count)),
			((char*)da->array) +(da->element_size * position), da->element_size *(da->elements -position)
		);
	}

	/* Copy element byte-by-byte (according to element_size) to array */
	if (data) {
		memcpy(((char*)da->array) +(da->element_size *position),
			data, da->element_size *data_count
		);
	}

	/* Increment elements */
	da->elements += data_count;

	/* Return OK */
	return 1;
}

/*
 * remove last element from array, shrink if needed
 */
int avdl_da_pop(struct avdl_dynamic_array *da) {
	return avdl_da_remove(da, 1, -1);
}

/*
 * remove arbitrary element from array
 */
int avdl_da_remove(struct avdl_dynamic_array *da, unsigned int count, int position) {

	/*
	 * a negative position means remove last elements
	 */
	if (position < 0) {
		position = da->elements -count;
	}

	/*
	 * selected element does not exist
	 */
	if (position >= da->elements
	||  position < 0
	||  count == 0
	||  position +count > da->elements) {
		return 0;
	}

	/*
	 * move elements backwards, to override removed elements
	 * unless removing last elements of array
	 */
	if (position +count < da->elements) {
		memmove(avdl_da_get(da, position), avdl_da_get(da, position +count), da->element_size *(da->elements -count -position));
	}

	/* finaly, remove element */
	da->elements -= count;

	/*
	 * shrink array, if less than a third filled
	 */
	if (da->elements < da->array_size/3
	&&  da->array_size/3 >= 3) {
		set_array_size(da, da->array_size/3);
	}

	/* element removed succesfully */
	return 1;
}

/*
 * clean allocated memory
 */
void avdl_da_free(struct avdl_dynamic_array *da) {
	/* if array exists, free it, leaves struct in undefined state */
	if (da->array) {
		avdl_free(da->array);
	}
}

/*
 * get element in array
 */
void *avdl_da_get(struct avdl_dynamic_array *da, int position) {

	if (position < 0) {
		position = da->elements +position;
	}

	// out of bounds, return null
	if (position >= da->elements) {
		return 0;
	}

	return ((char*)da->array) +(position *da->element_size);
}

void *avdl_da_getDeref(struct avdl_dynamic_array *da, int position) {
	return * ((void **)avdl_da_get(da, position));
}

unsigned int avdl_da_count(struct avdl_dynamic_array *da) {
	return da->elements;
}

void avdl_da_empty(struct avdl_dynamic_array *da) {
	while (da->elements > 0) avdl_da_pop(da);
}
void avdl_da_copy(struct avdl_dynamic_array *dest, struct avdl_dynamic_array *src) {
	for (int i = 0; i < src->elements; i++) {
		avdl_da_push(dest, avdl_da_get(src, i));
	}
}
