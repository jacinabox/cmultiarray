#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "multiarray.h"
#include "reflectable.h"
#include "miscellaneous.h"
#include "CExceptions.h"

MULTIARRAY transpose(ARRAYLIKE array) {
	MULTIARRAY array2;
	unsigned i, j;
	unsigned dims[2];

	finally(
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
	,
	puts("Should always happen"));
}

int main() {
	unsigned int dims[] = {2, 3};
	MULTIARRAY array = md_alloc(dims, int);
	time_t* time;
	
	//Demo of multidimensional arrays: taking the transpose of a matrix.
	if (!array) return -1;
	*md_2d(array, 0, 0, int) = 3;
	*md_2d(array, 0, 1, int) = 2;
	*md_2d(array, 0, 2, int) = 4;
	*md_2d(array, 1, 0, int) = 9;
	*md_2d(array, 1, 1, int) = 2;
	*md_2d(array, 1, 2, int) = 7;
	array = transpose(array);
	if (!array) { getch(); return -1; }
	array = md_resize(array, 1, 100);
	array = md_resize(array, 0, 50);
	array = md_resize(array, 1, 4);
	array = md_resize(array, 0, 4);
	printf("%i %i\n", *md_2d(array, 2, 1, int), *md_2d(array, 1, 0, int));

	//Demo of exception handling.
	/*catch(
		printf("%i\n", *md_2d(array, 1, 1, int));
		,
		printf("Test exception code: %s\n", exception););*/

	md_free(array);
	getch();

	//Demo of dynamic reflection.
	time = alloc(time_t);
	puts(reflect_header(time)->typenm);
	puts(reflect_header(time)->member_names[1]);
	printf("%u %u\n", time, member_by_name(time, "tv_nsec", NULL));

	reflect_print(&definition_time_t);
	getch();


	return 0;
}
