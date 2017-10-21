#ifndef __JC_EXCEPTIONS
#define __JC_EXCEPTIONS

#include <stdio.h>
#include <setjmp.h>

typedef int BOOL;

extern __thread BOOL __buf_fn_defined;
extern __thread BOOL __buf_blk_defined;
extern __thread BOOL buf_ex_defined;
extern __thread jmp_buf __buf_fn;
extern __thread jmp_buf __buf_blk;
extern __thread jmp_buf ex_buf;

typedef int EXCEPTION;
enum ACTION_TO_TAKE { NOTHING, RETURN, BREAK };

//The preliminary __finally performs a finally action upon returning
//from a procedure or local block, but is bypassed by an exception.
//Each form of control receives a local longjmp buffer for managing
//that form's runtime behavior. This possibility arises because
//block defined variables can shadow one another, making a kind of
//lexical stack; such a lexical stack reflects the lexical relationship
//of flow of control.
#define __finally(DO, FINALLY) {enum ACTION_TO_TAKE action; \
	int __ex_ret; \
	{ \
	jmp_buf __ex_buf_fn, __ex_buf_blk; \
	const BOOL __buf_fn_defined = 1; \
	if(__ex_ret=setjmp(__buf_fn)) { \
		action = RETURN; \
	} else if(setjmp(__buf_blk)) { \
		action = BREAK; \
	} else { \
		DO; \
		action = NOTHING; \
	}} \
	FINALLY; \
	if(action == NOTHING) { \
	} else if(action == RETURN) { \
		return(__ex_ret); \
	} else { \
		longjmp(__buf_blk, 1); \
	}}

//A catch form; the CATCH block is of course bypassed by
//other forms of control. There is a statically allocated
//longjmp buffer which persists between function calls
//(but it is managed with the lexical stack trick).
#define catch(DO, CATCH) {EXCEPTION exception = 0; \
	BOOL __temp_buf_ex_defined = buf_ex_defined; \
	jmp_buf __temp_ex_buf; \
	memcpy(__temp_ex_buf, ex_buf, sizeof(jmp_buf)); \
	buf_ex_defined = 1; \
	if (!(exception=setjmp(ex_buf))) {DO} \
	buf_ex_defined = __temp_buf_ex_defined; \
	memcpy(ex_buf, __temp_ex_buf, sizeof(jmp_buf)); \
	if (exception) {CATCH}} \

#define finally(DO, FINALLY) catch(__finally(DO, FINALLY), FINALLY; throw(exception))

//Return has to be non-zero. The macro checks a flag and determines
//whether to use a block local longjmp buffer or the native control.
//A sufficiently smart compiler could read the flag value (which
//is constant for any binding), and select the correct form of
//control statically.
#define return(RET) { if(__buf_fn_defined) { \
		longjmp(__buf_fn, (int)(RET)); \
	} else { \
		return(RET); \
	}}

#define break { if(__buf_blk_defined) { \
		longjmp(__buf_blk, 1); \
	} else { \
		break; \
	}}

//Exception has to be non-zero.
#define throw(EX) { if(buf_ex_defined) { \
		longjmp(ex_buf, (int)(EX)); \
	} else { \
		fprintf(stderr, "Exceptions: Terminated with exception %s", (EX)); \
		exit(-1); \
	}}

//The following keywords interfere with the whole scheme and should not be used:
//continue, goto.

//These macros add extra runtime behavior to the existing flow of control.
#define loop_form(KEYWORD) { const BOOL __buf_blk_defined = 1; \
	jmp_buf __buf_blk; \
	if (!setjmp(__buf_blk)) { \
	KEYWORD

#define for_ loop_form(for)
#define while_ loop_form(while)
#define do_ loop_form(do)
#define switch_ loop_form(switch)

//Ending syntax form for "for" and "while_" loops and "switch" forms
#define end }}}
//Ending syntax form for "do-while" loops
#define end_do(CONDITION) }while(CONDITION)}}

#endif