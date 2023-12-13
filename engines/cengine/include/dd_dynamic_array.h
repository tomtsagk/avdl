#ifndef DD_DYNAMIC_ARRAY_H
#define DD_DYNAMIC_ARRAY_H

/* a dynamic array
 * elements have a custom size (specified inside an init function)
 * new elements can be added, infinitely
 */

/*
 * main array struct
 */
struct dd_dynamic_array {
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
int dd_da_init (struct dd_dynamic_array *da, int el_size);

/*
 * add functions
 * push : adds one element to end of array
 * add  : adds `data_count` elements at `position`
 */
int dd_da_push(struct dd_dynamic_array *da, void *data);
int dd_da_add (struct dd_dynamic_array *da, const void *data, unsigned int data_count, int position);

/* remove functions */
int dd_da_pop(struct dd_dynamic_array *da);
int dd_da_remove(struct dd_dynamic_array *da, unsigned int count, int position);

/* Clean
 * responsible on freeing any memory that is allocated
 * should be called once for every init function
 * a dynamic array that is cleaned is left undefined, 
 * it can be reused with another init function
 */
void dd_da_free(struct dd_dynamic_array *da);

/* Get element of the array */
void *dd_da_get(struct dd_dynamic_array *da, int position);

/*
 * Get number of elements in array
 */
unsigned int dd_da_count(struct dd_dynamic_array *da);

void dd_da_empty(struct dd_dynamic_array *da);
void dd_da_copy(struct dd_dynamic_array *dest, struct dd_dynamic_array *src);

#endif
