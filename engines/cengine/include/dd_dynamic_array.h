#ifndef DD_DYNAMIC_ARRAY_H
#define DD_DYNAMIC_ARRAY_H

/* a dynamic array
 * elements have a custom size (specified inside an init function)
 * new elements can be added
 */

/* Dynamic array struct */
struct dd_dynamic_array {
	void *array;
	unsigned int elements;
	unsigned int array_size;
	unsigned int element_size;
};

/* Init functions
 * init  : init empty array
 * inita : init array with fixed array size
 */
int dd_da_init (struct dd_dynamic_array *da, int el_size);
int dd_da_inita(struct dd_dynamic_array *da, int el_size, int ar_size);

/* Add functions
 * add  : adds one element to array (based on element_size)
 * adda : adds an array of elements 
 */
int dd_da_add (struct dd_dynamic_array *da, void *data);
int dd_da_adda(struct dd_dynamic_array *da, void *data, unsigned int ar_size);

/* remove functions */
int dd_da_pop(struct dd_dynamic_array *da);
int dd_da_remove(struct dd_dynamic_array *da, unsigned int element);

/* Clean
 * responsible on freeing any memory that is allocated
 * should be called once for every _init* function 
 * (when array is no longer needed)
 * a dynamic array that is cleaned is left undefined, 
 * it can be reused with another init function
 */
void dd_da_free(struct dd_dynamic_array *da);

/* Get element of the array */
void *dd_da_get(struct dd_dynamic_array *da, unsigned int element);

void dd_da_empty(struct dd_dynamic_array *da);
void dd_da_copy(struct dd_dynamic_array *dest, struct dd_dynamic_array *src);

#endif
