#include <string.h>
#include <limits.h>
#include "multiarray.h"



static __thread struct MD_SLICE temporary_slice;

static inline unsigned int _the_stride(struct MD_ARRAY* ar, unsigned int dim_i, unsigned int el_sz) {
	unsigned int str = el_sz, *p_start = ar->dims+dim_i, *p_end = ar->dims+md_dims_n(ar);

	for (;p_start!=p_end;p_start++) {
#ifdef MD_INDEX_CHECKS
		if (UINT_MAX / *p_start <= str) {
			fputs("The byte size of the array overflows", stderr);
			throw MULTIARRAY_EX();
		}
#endif
		str *= *p_start;
	}
	return(str);
}


static inline unsigned int calculate_array_dimensions(unsigned int _md_dims[], unsigned int n_dims, unsigned int size) {
	unsigned int _md_i, _md_sz = size;

	for(_md_i=0;_md_i<n_dims;_md_i++) {

#ifdef MD_INDEX_CHECKS
		if (UINT_MAX/_md_dims[_md_i] <= _md_sz) {
			fputs("The byte size of the array overflows", stderr);
			throw MULTIARRAY_EX();
		}
#endif
		_md_sz *= _md_dims[_md_i];
	}


	return(_md_sz);
}

/* Accepts:
  * ar - An array or array slice.
  * i - The index for the next dimension to index.
Returns: A pointer to array slice standing for the data at the already-selected
indices. The pointer returned cannot be dereferenced; it can only be passed
to other functions.
Note: The md_#d macros demonstrate how to chain together calls to md_index
in order to index multi-dimensional arrays.*/
struct MD_SLICE* md_index(ARRAYLIKE ar, unsigned int i) {
	struct MD_SLICE* p_slice;
	struct MD_ARRAY* p_header;
	struct MD_SLICE slice;
	unsigned dim_size;

	switch (ar->struct_identifier) {
	case 0xAAAAA:
		p_header = (struct MD_ARRAY*)ar;
		dim_size = md_dims_array(p_header)[0];
#ifdef MD_INDEX_CHECKS
		if (i >= dim_size) {
			fprintf(stderr, "md_index: %u out of range %u in dimension 0\n", i, dim_size);
			throw MULTIARRAY_EX();
		}
#endif
		slice.p_base = p_header;
		slice.stride = _the_stride(p_header, 1, md_type_size(p_header));
		slice.p_indexing_base = p_header->data + i * slice.stride;
		slice.n_dims = p_header->n_dims - 1;
		break;
	case 0xAAAAB:
		p_slice = (struct MD_SLICE*)ar;
		p_header = p_slice->p_base;
#ifdef MD_INDEX_CHECKS
		if (p_slice->n_dims == 0) {
			fputs("md_index: indexed more times than there are dimensions", stderr);
			throw MULTIARRAY_EX();
		}
#endif
		dim_size = p_header->dims[md_dims_n(p_header)-p_slice->n_dims];
#ifdef MD_INDEX_CHECKS
		if (i >= dim_size) {
			fprintf(stderr, "md_index: %u out of range %u in dimension %u\n", i, dim_size, p_header->n_dims - p_slice->n_dims);
			throw MULTIARRAY_EX();
		}
#endif
		slice.p_base = p_slice->p_base;
		slice.stride = p_slice->stride / dim_size;
		slice.p_indexing_base = p_slice->p_indexing_base + i * slice.stride;
		slice.n_dims = p_slice->n_dims - 1;
		break;
	case 0xFEEED:
		fputs("md_index: already freed.", stderr);
		throw MULTIARRAY_EX();
	default:
		fputs("md_index: not an array or array slice.", stderr);
		throw MULTIARRAY_EX();
	}
	slice.struct_identifier = 0xAAAAB;
	temporary_slice = slice;
	return &temporary_slice;
}

/* Accepts
  * ar - Pointer to an array or array slice.
  * dim_i - A dimension number (starting from 0).
  * size - The new size (number of elements) of the selected dimension.
Returns: a new MD_ARRAY on success, NULL on failure.
Purpose: Resizes a multi-array along one dimension.
Note: It can be assumed that md_resize will render invalid the parameter multi-array,
and will also render invalid any slices formerly taken on that multi-array.*/
struct MD_ARRAY* md_resize(ARRAYLIKE ar, unsigned int dim_i, unsigned int size) {
	struct MD_ARRAY* ar2;
	struct MD_SLICE* p_slice;
	char *p_read, *p_write, *p_write_end;
	unsigned int old_str, new_str, old_size, new_size, old_dim_size, n_dims;

again:
	switch (ar->struct_identifier) {
	case 0xAAAAA:
		ar2 = (struct MD_ARRAY*)ar;
		old_str = _the_stride(ar2, dim_i, md_type_size(ar2));
		old_size = _the_stride(ar2, 0, md_type_size(ar2));
		old_dim_size = md_dims_array(ar2)[dim_i];
		md_dims_array(ar2)[dim_i] = size;
		new_str = _the_stride(ar2, dim_i, md_type_size(ar2));
		new_size = _the_stride(ar2, 0, md_type_size(ar2));
		n_dims = md_dims_n(ar2);

		if (size == old_dim_size) {
			return(ar2);
		} else if (size < old_dim_size) {
			p_read = p_write = ar2->data;
			p_write_end = ar2->data + new_size;
			for (;p_write!=p_write_end;p_read+=old_str,p_write+=new_str) {
				memmove(p_write, p_read, new_str);
			}
			ar2 = (struct MD_ARRAY*)(realloc(ar2, sizeof(struct MD_ARRAY) + new_size));
			if (ar2) return (ar2);
			return((struct MD_ARRAY*)ar);
		} else {
			ar2 = (struct MD_ARRAY*)(realloc(ar2, sizeof(struct MD_ARRAY) + new_size));
			if (!ar2) {
				fputs("Array re-allocation failed", stderr);
				//...but the old array is still available.
				throw MULTIARRAY_EX();
			}

			p_read = ar2->data + old_size;
			p_write = ar2->data + new_size;
			p_write_end = ar2->data;
			do {
				p_read -= old_str;
				p_write -= new_str;
				memmove(p_write, p_read, old_str);
				memset(p_write+old_str, 0, new_str-old_str);
			} while (p_write!=p_write_end);
			return(ar2);
		}
	case 0xAAAAB:
		p_slice = (struct MD_SLICE*)ar;
		p_slice->struct_identifier = 0xFEEED;
		ar = (ARRAYLIKE)(p_slice->p_base);
		dim_i += md_dims_n(p_slice->p_base) - p_slice->n_dims;
		goto again;
	case 0xFEEED:
		fputs("md_resize: already freed.", stderr);
		throw MULTIARRAY_EX();
	default:
		fputs("md_resize: not an array or array slice.", stderr);
		throw MULTIARRAY_EX();
	}
}



struct MD_ARRAY* _md_alloc(unsigned int _md_dims[], unsigned int n_dims, unsigned int size) {
	struct MD_ARRAY* _md_p;
	struct MD_ARRAY* _md_header;
	unsigned int _md_i, _md_sz = calculate_array_dimensions(_md_dims, n_dims, size);

	if (n_dims > MAX_DIMENSIONS) {
		fputs("md_alloc: n_dims should not exceed MAX_DIMENSIONS", stderr);
		throw MULTIARRAY_EX();
	}

	_md_p = (struct MD_ARRAY*)(calloc(sizeof(struct MD_ARRAY) + _md_sz, 1));
	if (!_md_p) {
		fputs("md_alloc: array allocation failed", stderr);
		throw MULTIARRAY_EX();
	}
	memcpy(_md_p->dims, _md_dims, sizeof(unsigned int) * n_dims);
	_md_header = _md_p;
	_md_header->n_dims = n_dims;
	_md_header->type_size = size;
	_md_header->struct_identifier = 0xAAAAA;
	return (_md_p);
}
