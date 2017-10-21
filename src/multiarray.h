#ifndef _JC_MULTIARRAY
#define _JC_MULTIARRAY

#include <stdlib.h>
#include <stdio.h>
#include "CExceptions.h"

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

typedef void* ARRAYLIKE;
typedef void* MULTIARRAY;

#define md_dims_n(AR) (((unsigned int*)(AR))[-3])
#define md_dims_array(AR) ((unsigned int*)(AR)-3-md_dims_n(AR))
#define md_type_size(AR) (((unsigned int*)(AR))[-2])

#define N_ELEMS(AR) (sizeof(AR) / sizeof((AR)[0]))

extern const char* MULTIARRAY_EX;

#pragma pack(push, 1)
struct MD_ARRAY {
	//Preceding words are dimensions.
	unsigned int n_dims;
	unsigned int type_size;
	unsigned int struct_identifier; //Must be AAAAA
	//Data follows.
};

//Always stack allocated
struct MD_SLICE {
	unsigned int* p_base;
	char* p_indexing_base;
	unsigned int n_dims;
	unsigned int stride;
	unsigned int struct_identifier; //Must be AAAAB
};
#pragma pack(pop)

static MULTIARRAY _md_alloc(unsigned int _md_dims[], unsigned int n_dims, unsigned int size) {
	unsigned int* _md_p;
	struct MD_ARRAY* _md_header;
	unsigned int _md_i = 1, _md_sz = size;

	for(_md_i=0;_md_i<n_dims;_md_i++) _md_sz *= _md_dims[_md_i];
	_md_p = calloc(sizeof(unsigned int) * n_dims + sizeof(struct MD_ARRAY) + _md_sz, 1);
	if (!_md_p) {
		fputs("Array allocation failed", stderr);
		throw(MULTIARRAY_EX);
	}
	memcpy(_md_p, _md_dims, sizeof(unsigned int) * n_dims);
	_md_header = (struct MD_ARRAY*)(_md_p + n_dims);
	_md_header->n_dims = n_dims;
	_md_header->type_size = size;
	_md_header->struct_identifier = 0xAAAAA;
	return(_md_p + n_dims + 3);
}

/*Accepts:
  * _md_dims - A static/stack allocated array containing the dimensions.
  * type- The type of array to allocate.
Returns a pointer to the allocated array.*/
#define md_alloc(_md_dims, type) ((type*)_md_alloc((_md_dims), N_ELEMS(_md_dims), sizeof(type)))

/* Accepts:
  * ar - An array or array slice.
Returns: void.
Purpose: Frees the multi-array. If an array slice is provided, its attached array is freed in its
entirety, after which time the array slice will no longer work.
Note: Use this function to free multi-arrays. Do not use it to free plain old C arrays;
use free instead.*/
static void md_free(ARRAYLIKE ar) {
	if (!ar) return(1);
again:
	switch (((unsigned int*)ar)[-1]) {
	case 0xAAAAA:
		((unsigned int*)ar)[-1] = 0xFEEED;
		free(md_dims_array(ar));
		break;
	case 0xAAAAB:
		((unsigned int*)ar)[-1] = 0xFEEED;
		ar = (ARRAYLIKE)(((unsigned int*)ar)[-5]);
		goto again;
	case 0xFEEED:
		fputs("md_free: already freed.", stderr);
		throw(MULTIARRAY_EX);
	default:
		fputs("md_free: not an array or array slice.", stderr);
		throw(MULTIARRAY_EX);
	}
}

//Purpose: Converts the result of ar_index to a dereferenceable pointer to SLICE.
#define adjust_slice_p(S) ((struct MD_SLICE*)((unsigned int*)(S) + 5))
//Purpose: Converts back to something that is acceptable for other functions in this header.
#define unadjust_slice_p(S) ((struct MD_SLICE*)((unsigned int*)(S) - 5))

struct MD_SLICE* md_index(ARRAYLIKE ar, unsigned int i);

/* Accepts:
  * ar - An array or array slice.
Returns: A pointer to the first element of the given array-like object.*/
static void* md_getptr(ARRAYLIKE ar) {
	switch (((unsigned int*)ar)[-1]) {
	case 0xAAAAB:
		return((void*)(((unsigned int*)ar)[-4]));
	case 0xAAAAA:
		return(ar);
	case 0xFEEED:
		fputs("md_getptr: already freed.", stderr);
		throw(MULTIARRAY_EX);
	default:
		fputs("md_getptr: not an array or array slice.", stderr);
		throw(MULTIARRAY_EX);
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

MULTIARRAY md_resize(ARRAYLIKE ar, unsigned int dim_i, unsigned int size);

#endif