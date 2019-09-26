#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "multiarray.h"




struct MD_ARRAY* transpose(struct MD_ARRAY* array) {
	struct MD_ARRAY* array2;
	unsigned i, j;
	unsigned dims[2];


	if (md_dims_n(array) != 2) {
		fputs("transpose: need two dimensional array", stderr);
		return(NULL);
	}
	dims[0] = md_dims_array(array)[1];
	dims[1] = md_dims_array(array)[0];
	array2 = _md_alloc(dims, 2, md_type_size(array));
	for (i=0;i<dims[0];i++) {
		for (j=0;j<dims[1];j++) {
			memcpy(md_2d(array2, i, j, void), md_2d(array, j, i, void), md_type_size(array));
		}
	}
	return(array2);


}

int main() {
	unsigned int dims[] = {2, 3};
	struct MD_ARRAY* array = md_alloc(dims, int);
	time_t* time;
	
	//Demo of multidimensional arrays: taking the transpose of a matrix.
	printf("%u\n", sizeof(unsigned int*));
	if (!array) return -1;
	*md_2d(array, 0, 0, int) = 3;
	*md_2d(array, 0, 1, int) = 2;
	*md_2d(array, 0, 2, int) = 4;
	*md_2d(array, 1, 0, int) = 9;
	*md_2d(array, 1, 1, int) = 2;
	*md_2d(array, 1, 2, int) = 7;
	puts("Completed writes");
	array = transpose(array);
	puts("Completed transpose");
	if (!array) { getc(NULL); return -1; }
	array = md_resize(array, 1, 100);
	array = md_resize(array, 2, 50);
	array = md_resize(array, 1, 4);
	array = md_resize(array, 2, 4);
	printf("%i %i\n", *md_2d(array, 1, 2, int), *md_2d(array, 1, 0, int));

	//Demo of exception handling.
	/*catch(
		printf("%i\n", *md_2d(array, 1, 1, int));
		,
		printf("Test exception code: %s\n", exception););*/

	md_free(array);

	getc(stdin);

	//Demo of dynamic reflection.
	//time = alloc(time_t);
	//puts(reflect_header(time)->typenm);
	//puts(reflect_header(time)->member_names[1]);
	//printf("%u %u\n", time, member_by_name(time, "tv_nsec", NULL));

	//reflect_print(&definition_time_t);



	return 0;
}
