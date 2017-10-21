#include <setjmp.h>

typedef int BOOL;

__thread BOOL __buf_fn_defined = 0, __buf_blk_defined = 0, buf_ex_defined = 0;
__thread jmp_buf __buf_fn, __buf_blk, ex_buf;

//See also CExceptions.h
