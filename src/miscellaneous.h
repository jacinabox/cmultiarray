#ifndef __JC_MISCELLANEOUS
#define __JC_MISCELLANEOUS

#include "reflectable.h"

typedef REFLECTABLE FREEABLE;

//Miscellaneous utilities

//Purpose: Accepts a pointer to object; reflects against it to retrieve a free function
//(destructor); calls the destructor.
//Notes:
//  * freeable_free also takes care of freeing the memory.
//  * The free function has type void(*)(void*).
static void freeable_free(FREEABLE p) {
	void(*__free)(void*) = member_by_name(p, "__free", "void(*)(void*)");

	if (!__free) throw("Notfreeable");
	finally(
	__free(p);,
	reflect_free(p););
}

typedef REFLECTABLE CLONEABLE;

//Purpose: Clones an object.
//Notes:
//  * The clone function has type void(*)(void* fresh, void* old).
//  * Do not free the new structure in the clone function; it is freed automatically
//    upon throwing an exception.
static CLONEABLE clone(CLONEABLE p) {
	const struct REFLECT* header = reflect_header(p);
	CLONEABLE fresh;
	void(*__clone)(void*,void*) = member_by_name(p, "__clone", "void(*)(void*,void*)");

	if (!__clone) throw("Notcloneable");
	fresh = _alloc(header, 1, header->bytessize);
	catch
	(__clone(fresh,p);,
	reflect_free(fresh);throw(exception););
	return fresh;
}

#endif