#ifndef AVDL_DYNAMIC_ARRAY_H
#define AVDL_DYNAMIC_ARRAY_H

/* a dynamic array
 * elements have a custom size (specified inside an init function)
 * new elements can be added, infinitely
 */

#include <stddef.h>

/*
 * main array struct
 */
struct avdl_dynamic_array {
	void *array;
	unsigned int elements_count;
	unsigned int array_total_count;
	size_t element_size;
	void (*clean)(struct avdl_dynamic_array *);
};

void avdl_dynamic_array_create(struct avdl_dynamic_array *da);
void avdl_dynamic_array_clean(struct avdl_dynamic_array *da);

/*
 * init function
 * this has to be run once before the array is accessed
 *
 * for each `init` call, a `free` call should be called when
 * the array is no longer needed
 */
int avdl_dynamic_array_init (struct avdl_dynamic_array *da, size_t el_size);
#define avdl_dynamic_array_initStruct(da, str) avdl_dynamic_array_init(da, sizeof(struct str))
#define avdl_dynamic_array_initStructRef(da, str) avdl_dynamic_array_init(da, sizeof(struct str *))

/*
 * add functions
 * push : adds one element to end of array
 * add  : adds `data_count` elements at `position`
 */
int avdl_dynamic_array_pushEmpty(struct avdl_dynamic_array *da);
int avdl_dynamic_array_push(struct avdl_dynamic_array *da, void *data);
int avdl_dynamic_array_add (struct avdl_dynamic_array *da, const void *data, unsigned int data_count, int position);

/* remove functions */
int avdl_dynamic_array_pop(struct avdl_dynamic_array *da);
int avdl_dynamic_array_remove(struct avdl_dynamic_array *da, unsigned int count, int position);

/* Clean
 * responsible on freeing any memory that is allocated
 * should be called once for every init function
 * a dynamic array that is cleaned is left undefined, 
 * it can be reused with another init function
 */
void avdl_dynamic_array_free(struct avdl_dynamic_array *da);

/* Get element of the array */
void *avdl_dynamic_array_get(struct avdl_dynamic_array *da, int position);
void *avdl_dynamic_array_getDeref(struct avdl_dynamic_array *da, int position);

/*
 * Get number of elements in array
 */
unsigned int avdl_dynamic_array_count(struct avdl_dynamic_array *da);

void avdl_dynamic_array_empty(struct avdl_dynamic_array *da);
void avdl_dynamic_array_copy(struct avdl_dynamic_array *dest, struct avdl_dynamic_array *src);

int avdl_dynamic_array_Swap(struct avdl_dynamic_array *da, int position1, int position2);
void avdl_dynamic_array_Shuffle(struct avdl_dynamic_array *da);

#endif
