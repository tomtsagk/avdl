#ifndef DD_DYNAMIC_ARRAY_H
#define DD_DYNAMIC_ARRAY_H

/* a dynamic array
 * elements have a custom size (specified inside an init function)
 * new elements can be added, infinitely
 */

/*
 * main array struct
 */
struct avdl_dynamic_array {
	void *array;
	unsigned int elements;
	unsigned int array_size;
	unsigned int element_size;
};

/*
 * init function
 * this has to be run once before the array is accessed
 *
 * for each `init` call, a `free` call should be called when
 * the array is no longer needed
 */
int avdl_da_init (struct avdl_dynamic_array *da, int el_size);

/*
 * add functions
 * push : adds one element to end of array
 * add  : adds `data_count` elements at `position`
 */
int avdl_da_push(struct avdl_dynamic_array *da, void *data);
int avdl_da_add (struct avdl_dynamic_array *da, const void *data, unsigned int data_count, int position);

/* remove functions */
int avdl_da_pop(struct avdl_dynamic_array *da);
int avdl_da_remove(struct avdl_dynamic_array *da, unsigned int count, int position);

/* Clean
 * responsible on freeing any memory that is allocated
 * should be called once for every init function
 * a dynamic array that is cleaned is left undefined, 
 * it can be reused with another init function
 */
void avdl_da_free(struct avdl_dynamic_array *da);

/* Get element of the array */
void *avdl_da_get(struct avdl_dynamic_array *da, int position);

/*
 * Get number of elements in array
 */
unsigned int avdl_da_count(struct avdl_dynamic_array *da);

#endif
