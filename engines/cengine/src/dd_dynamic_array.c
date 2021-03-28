#include <stdlib.h>
#include <stdio.h>
#include "dd_dynamic_array.h"
#include <string.h>
#include "dd_log.h"

/* Init empty array */
int dd_da_init(struct dd_dynamic_array *da, int el_size) {
	/* Init everything to 0, except element size to given value */
	da->element_size = el_size;
	da->elements = 0;

	da->array_size = 0;
	da->array = 0;

	/* Everything OK */
	return 0;
}

/* Init array with specific array size */
int dd_da_inita(struct dd_dynamic_array *da, int el_size, int ar_size) {
	/* Init element and array size, and allocate memory 
	 * there are no elements in initialization 
	 */
	da->element_size = el_size;
	da->elements = 0;

	/* create array based on (array_size * element_size) */
	da->array_size = ar_size;
	da->array = malloc( da->element_size *da->array_size );

	/* Check allocation */
	if (!da->array) {
		dd_log("da_inita: cannot allocate memory");
		return -1;
	}

	/* Everything OK */
	return 0;
}

/* Adds one element (of size element_size) to the array */
int dd_da_add(struct dd_dynamic_array *da, void *data) {

	/* first check if array can hold new data, if not
	 * increase array size, then just add the new element
	 */

	/* No array exists */
	if (!da->array) {
		/* Init array at 3 elements */
		da->array_size = 3;
		da->array = malloc(da->element_size *da->array_size);

		/* check allocation */
		if (!da->array) {
			dd_log("da_add: cannot allocate memory");
			return -1;
		}
	} else
	/* New element will go over array size */
	if (da->elements +1 > da->array_size) {
		/* Double array size */
		da->array_size *= 2;
		void *temp = realloc(da->array, da->element_size *da->array_size);

		/* Allocation worked */
		if (temp) {
			da->array = temp;
		}
		/* Allocation failed */
		else {
			dd_log("error: cannot re-allocate memory, abort");
			return -1;
		}
	}

	/* Copy element byte-by-byte (according to element_size) to array */
	memcpy( ((char*)da->array) +(da->element_size *da->elements),
		data, da->element_size);

	/* Increment elements */
	da->elements++;

	/* Return OK */
	return 0;
}

/* Adds an array of data to the dynamic array */
int dd_da_adda(struct dd_dynamic_array *da, void *data, unsigned int ar_size) {
	/* For each element, try to add it, return on error 
	 * (char*) is used to count 1 byte at a time, this might need fixing later on
	 */
	for (unsigned int i = 0; i < ar_size; i++) {
		if (dd_da_add(da, ((char*) data) +(da->element_size *i)) != 0) {
			dd_log("da_adda: unable to add array of data to dynamic array");
			return -1;
		}
	}
	return 0;
}

int dd_da_pop(struct dd_dynamic_array *da) {
	if (da->elements > 0) {
		da->elements--;
		if (da->elements < da->array_size/3) {
			da->array_size /= 3;
			void *ptr;
			ptr = realloc(da->array, da->array_size * da->element_size);
			if (ptr) {
				da->array = ptr;
				return 0;
			}
			return -1;
		}
		return 0;
	}
	return -1;
}

int dd_da_remove(struct dd_dynamic_array *da, unsigned int element) {

	/* selected element does not exist */
	if (element >= da->elements) {
		return -1;;
	}

	/* move elements one step backwards */
	unsigned int i = element;
	for (i = element+1; i < da->elements; i++) {
		memcpy( dd_da_get(da, i-1), dd_da_get(da, i), da->element_size );
	}

	/* finaly, remove element */
	da->elements--;

	/* element removed succesfully */
	return 0;
}

/* Clean allocated array */
void dd_da_free(struct dd_dynamic_array *da) {
	/* if array exists, free it, leaves struct in undefined state */
	if (da->array) {
		free(da->array);
	}
}

/* Get element */
void *dd_da_get(struct dd_dynamic_array *da, unsigned int element) {
	if (element >= da->elements) {
		dd_log("dd_da_get: out of bounds error: %d / %d", element, da->elements);
		exit(-1);
	}
	return ((char*)da->array) +(element *da->element_size);
}

void dd_da_empty(struct dd_dynamic_array *da) {
	while (da->elements > 0) dd_da_pop(da);
}

void dd_da_copy(struct dd_dynamic_array *dest, struct dd_dynamic_array *src) {
	dd_da_adda(dest, src->array, src->elements);
}
