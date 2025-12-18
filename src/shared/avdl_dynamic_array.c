#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "shared/avdl_dynamic_array.h"
#include "shared/avdl_std.h"
#include "shared/avdl_log.h"
#include "shared/avdl_math.h"

void avdl_dynamic_array_create(struct avdl_dynamic_array *da) {
	da->array = 0;
	da->elements_count = 0;
	da->array_total_count = 0;
	da->element_size = 0;
	da->clean = avdl_dynamic_array_clean;
}

void avdl_dynamic_array_clean(struct avdl_dynamic_array *da) {
	/* if array exists, free it, leaves struct in undefined state */
	if (da->array) {
		avdl_free(da->array);
	}
}

static int set_array_size(struct avdl_dynamic_array *da, int count) {

	da->array_total_count = count;
	if (!da->array) {
		da->array = avdl_malloc(da->element_size *da->array_total_count);

		if (!da->array) {
			return 0;
		}
	}
	else {
		void *temp = avdl_realloc(da->array, da->element_size *da->array_total_count);

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
int avdl_dynamic_array_init(struct avdl_dynamic_array *da, size_t el_size) {
	da->element_size = el_size;
	da->elements_count = 0;
	da->array_total_count = 0;
	da->array = 0;
	return 1;
}

/*
 * Adds one element to the array
 */
int avdl_dynamic_array_pushEmpty(struct avdl_dynamic_array *da) {
	return avdl_dynamic_array_add(da, 0, 1, -1);
}

int avdl_dynamic_array_push(struct avdl_dynamic_array *da, void *data) {
	return avdl_dynamic_array_add(da, data, 1, -1);
}

int avdl_dynamic_array_add(struct avdl_dynamic_array *da, const void *data, unsigned int data_count, int position) {

	/*
	 * position of negative value means add to end of array
	 */
	if (position < 0) {
		position = da->elements_count +(position +1);

		if (position < 0) {
			avdl_log_error("failed to add data to array: position is negative: %d", position);
			return 0;
		}
	}

	/*
	 * array doesn't exist or can't hold new data - resize it
	 */
	if (!da->array
	||  da->elements_count +data_count > da->array_total_count) {
		int newSize = da->elements_count > 3 ? da->elements_count : 3;
		while (newSize < da->elements_count +data_count) {
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
	if (position < da->elements_count) {
		memmove(((char*)da->array) +(da->element_size *(position +data_count)),
			((char*)da->array) +(da->element_size * position), da->element_size *(da->elements_count -position)
		);
	}

	/* Copy element byte-by-byte (according to element_size) to array */
	if (data) {
		memcpy(((char*)da->array) +(da->element_size *position),
			data, da->element_size *data_count
		);
	}

	/* Increment elements */
	da->elements_count += data_count;

	/* Return OK */
	return 1;
}

/*
 * remove last element from array, shrink if needed
 */
int avdl_dynamic_array_pop(struct avdl_dynamic_array *da) {
	return avdl_dynamic_array_remove(da, 1, -1);
}

/*
 * remove arbitrary element from array
 */
int avdl_dynamic_array_remove(struct avdl_dynamic_array *da, unsigned int count, int position) {

	/*
	 * a negative position means remove last elements
	 */
	if (position < 0) {
		position = da->elements_count -count;
	}

	/*
	 * selected element does not exist
	 */
	if (position >= da->elements_count
	||  position < 0
	||  count == 0
	||  position +count > da->elements_count) {
		return 0;
	}

	/*
	 * move elements backwards, to override removed elements
	 * unless removing last elements of array
	 */
	if (position +count < da->elements_count) {
		memmove(avdl_dynamic_array_get(da, position), avdl_dynamic_array_get(da, position +count), da->element_size *(da->elements_count -count -position));
	}

	/* finaly, remove element */
	da->elements_count -= count;

	/*
	 * shrink array, if less than a third filled
	 */
	if (da->elements_count < da->array_total_count/3
	&&  da->array_total_count/3 >= 3) {
		set_array_size(da, da->array_total_count/3);
	}

	/* element removed succesfully */
	return 1;
}

/*
 * clean allocated memory
 */
void avdl_dynamic_array_free(struct avdl_dynamic_array *da) {
	avdl_dynamic_array_clean(da);
}

/*
 * get element in array
 */
void *avdl_dynamic_array_get(struct avdl_dynamic_array *da, int position) {

	if (position < 0) {
		position = da->elements_count +position;
	}

	// out of bounds, return null
	if (position >= da->elements_count) {
		return 0;
	}

	return ((char*)da->array) +(position *da->element_size);
}

void *avdl_dynamic_array_getDeref(struct avdl_dynamic_array *da, int position) {
	return * ((void **)avdl_dynamic_array_get(da, position));
}

unsigned int avdl_dynamic_array_count(struct avdl_dynamic_array *da) {
	return da->elements_count;
}

void avdl_dynamic_array_empty(struct avdl_dynamic_array *da) {
	while (da->elements_count > 0) avdl_dynamic_array_pop(da);
}
void avdl_dynamic_array_copy(struct avdl_dynamic_array *dest, struct avdl_dynamic_array *src) {
	for (int i = 0; i < src->elements_count; i++) {
		avdl_dynamic_array_push(dest, avdl_dynamic_array_get(src, i));
	}
}

int avdl_dynamic_array_Swap(struct avdl_dynamic_array *da, int position1, int position2) {

	// same positions, nothing to skip
	if (position1 == position2) {
		return -1;
	}

	// out of bounds position1
	if (position1 < 0 || position1 >= avdl_dynamic_array_count(da)) {
		return -1;
	}

	// out of bounds position2
	if (position2 < 0 || position2 >= avdl_dynamic_array_count(da)) {
		return -1;
	}

	// swap
	void *element1 = avdl_dynamic_array_get(da, position1);
	void *element2 = avdl_dynamic_array_get(da, position2);
	void *tempElement = avdl_malloc(da->element_size);
	if (!tempElement) {
		return -1;
	}

	memcpy(tempElement, element1, da->element_size);
	memcpy(element1, element2, da->element_size);
	memcpy(element2, tempElement, da->element_size);
	avdl_free(tempElement);
	return 0;
}

void avdl_dynamic_array_Shuffle(struct avdl_dynamic_array *da) {

	for (int i = 0; i < avdl_dynamic_array_count(da); i++) {
		int newIndex = avdl_math_RandomInt(avdl_dynamic_array_count(da));

		// skip swaps on the same index
		if (i == newIndex) {
			continue;
		}

		avdl_dynamic_array_Swap(da, i, newIndex);
	}

}
