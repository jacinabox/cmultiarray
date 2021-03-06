#ifndef _JC_MULTIARRAY
#define _JC_MULTIARRAY

#include <stdlib.h>
#include <stdio.h>
#include <exception>

/*Notes:
Multiarrays (By James Candy)

Multi-arrays are set up to act like ordinary pointers to arrays. They are equipped
with additional properties pertaining to dimensions. They have a dimensionality,
which can consist of multiple dimensions, and is retrieved with md_dims_n.
Each dimension has a size. These properties are carried in the multi-array.

Multi-arrays may be resized with the md_resize function. In that case elements
of the multi-array are moved around to add or remove space. Multi-arrays
cannot change dimensionality.

Bounds checking is disabled by default. Compile with -DMD_INDEX_CHECKS to enable it.*/


#define md_dims_n(AR) ((AR)->n_dims)
#define md_dims_array(AR) ((AR)->dims)
#define md_type_size(AR) ((AR)->type_size)

#define N_ELEMS(AR) (sizeof(AR) / sizeof((AR)[0]))

struct MULTIARRAY_EX : public std::exception {
};

#define MAX_DIMENSIONS 5


struct MD_ARRAYLIKE {
	unsigned int struct_identifier;
};

typedef struct MD_ARRAYLIKE* ARRAYLIKE; // = array or slice

struct MD_ARRAY : public MD_ARRAYLIKE {
	//struct_identifier must be AAAAA
	unsigned int n_dims;
	unsigned int type_size;

	unsigned int dims[MAX_DIMENSIONS];

	char data[1];
};

//Always stack allocated
struct MD_SLICE : public MD_ARRAYLIKE {
	//struct_identifier must be AAAAB
	struct MD_ARRAY* p_base;
	char* p_indexing_base;
	unsigned int n_dims;
	unsigned int stride;
};


struct MD_ARRAY* _md_alloc(unsigned int _md_dims[], unsigned int n_dims, unsigned int size);

/*Accepts:
  * _md_dims - A static/stack allocated array containing the dimensions.
  * type- The type of array to allocate.
Returns a pointer to the allocated array.*/
#define md_alloc(_md_dims, type) (_md_alloc((_md_dims), N_ELEMS(_md_dims), sizeof(type)))

/* Accepts:
  * ar - An array or array slice.
Returns: void.
Purpose: Frees the multi-array. If an array slice is provided, its attached array is freed in its
entirety, after which time the array slice will no longer work.
Note: Use this function to free multi-arrays. Do not use it to free plain old C arrays;
use free instead.*/
static void md_free(ARRAYLIKE ar) {
	struct MD_ARRAY* ar2;
	struct MD_SLICE* pSlice;

	if (!ar) return;
again:
	switch (ar->struct_identifier) {
	case 0xAAAAA:
		ar2 = (struct MD_ARRAY*)ar;
		//The code for md_free writes a magic number into the memory of the array before
		//freeing it, in an attempt to detect subsequent wild accesses to this memory;
		//of course the effectiveness of this technique is theoretically limited to
		//whether or not the optimizer allows the test.
		ar2->struct_identifier = 0xFEEED;
		free(ar);
		break;
	case 0xAAAAB:
		pSlice = (struct MD_SLICE*)ar;
		pSlice->struct_identifier = 0xFEEED;
		ar = (ARRAYLIKE)(pSlice->p_base);
		goto again;
	case 0xFEEED:
		fputs("md_free: already freed.", stderr);
		throw MULTIARRAY_EX();
	default:
		fputs("md_free: not an array or array slice.", stderr);
		throw MULTIARRAY_EX();
	}
}






/* md_index is used to index an array or array slice. The way of taking slices is meant to
be familiar to any C programmer. When a slice is taken on a d-dimensional array, it represents
a (d-1)-dimensional subarray, in the familiar manner from C (one crucial difference being,
that C arrays are allowed to be ragged, whereas this array concept is not). Another important
difference is that in C indexing upon a one dimensional array gives a pointer to the element
type, whereas here it gives a zero-dimensional slice containing one element. No matter the
dimensionality of a slice, its first element can be examined by using md_getptr (provided
it's non empty). */
struct MD_SLICE* md_index(ARRAYLIKE ar, unsigned int i);

/* Accepts:
  * ar - An array or array slice.
Returns: A pointer to the first element of the given array-like object.*/
static char* md_getptr(ARRAYLIKE ar) {
	struct MD_ARRAY* ar2;
	struct MD_SLICE* pSlice;

	switch (ar->struct_identifier) {
	case 0xAAAAB:
		pSlice = (struct MD_SLICE*)ar;
		return pSlice->p_indexing_base;
	case 0xAAAAA:
		ar2 = (struct MD_ARRAY*)ar;
		return ar2->data;
	case 0xFEEED:
		fputs("md_getptr: already freed.", stderr);
		throw MULTIARRAY_EX();
	default:
		fputs("md_getptr: not an array or array slice.", stderr);
		throw MULTIARRAY_EX();
	}
}

/*Accepts:
  * AR - An array or array slice.
  * I, J, K, etc. - Array indices.
  * TYPE - The type of object that the array contains. If a void pointer is desired pass 'void'.
Returns: A pointer to an array element.
Notes: If the indexing done does not exhaust the dimensions of the array, a pointer to the first
element in the remaining dimensions is returned.*/
#define md_2d(AR, I, J, TYPE) ((TYPE*)md_getptr(md_index(md_index((AR), (I)), (J))))
#define md_3d(AR, I, J, K, TYPE) ((TYPE*)md_getptr(md_index(md_index(md_index((AR), (I)), (J)), (K))))
#define md_4d(AR, I, J, K, L, TYPE) ((TYPE*)md_getptr(md_index(md_index(md_index(md_index((AR), (I)), (J)), (K)), (L))))

struct MD_ARRAY* md_resize(ARRAYLIKE ar, unsigned int dim_i, unsigned int size);

#endif
