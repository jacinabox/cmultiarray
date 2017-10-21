#ifndef _JC_REFLECTABLE
#define _JC_REFLECTABLE

#include <stdlib.h>
#include "CExceptions.h"

typedef void* REFLECTABLE;

/*A reflection system for C. Data constructed as REFLECTABLE will have information
available including its typename, size in bytes, and information about its members.

Note: New structures defined as REFLECTABLE have to be packed. Type information
will be defined in a statically allocated REFLECT structure and referenced
by all allocated data from that structure.*/

#pragma pack(push, 1)
struct REFLECT {
	const char* typenm;
	unsigned short n_members;
	unsigned short bytessize;
	const struct REFLECT* members[10];
	const char* member_names[10];
};
#pragma pack(pop)

#define _reflect_header_p(P) ((struct REFLECT**)(P) - 1)

/*Purpose: To access ths structure containing reflection information for a structure,
using a pointer to data from that structure.*/
static const struct REFLECT* reflect_header(REFLECTABLE p) {
	const struct REFLECT* p_header = (const struct REFLECT*)*_reflect_header_p(p);

	if (!p_header) {
		fputs("reflect_header: bad pointer.", stderr);
		throw("Reflectable");
	}
	return p_header;
}

static void reflect_free(REFLECTABLE p) {
	struct REFLECT** p_header = _reflect_header_p(p);

	if (!*p_header) {
		fputs("reflect_free: double free or bad pointer.", stderr);
		throw("Reflectable");
	}
	*p_header = NULL;
	free(p_header);
}

static REFLECTABLE _alloc(const struct REFLECT* p, size_t n_elements, size_t n_sz) {
	struct REFLECT** p2 = calloc(1, n_elements*n_sz+sizeof(const struct REFLECT*));

	if(!p2) throw("Allocfailed");
	*p2 = (struct REFLECT*)p;
	return p2+1;
}

#define alloc_array(TYPE, N_ELEMENTS) _alloc(&definition_##TYPE, (N_ELEMENTS), sizeof(TYPE))
/* Accepts:
  * TYPE - the name of type to allocate.
Returns: A valid pointer to TYPE; it is a REFLECTABLE with reflection information.
Note: You MUST use reflect_free or equivalent to free the returned pointer.*/
#define alloc(TYPE) alloc_array(TYPE, 1)

static REFLECTABLE _structural_subtyping(REFLECTABLE sub, const struct REFLECT* super_p) {
	const struct REFLECT* sub_p = reflect_header(sub);

	return
		(sub_p->n_members >= super_p->n_members && !memcmp(sub_p->members, super_p->members, sizeof(struct REFLECT*) * super_p->n_members)
		? sub
		: NULL);
}

static REFLECTABLE _recast(REFLECTABLE p, const struct REFLECT* type_def) {
	return
		(!strcmp(reflect_header(p)->typenm, type_def->typenm)
		? p
		: NULL);
}

/* Accepts:
  * P - A pointer to data with reflection information.
  * TYPE - The type to cast to.
Returns: The coercion of P if it points to a structure that is structurally compatible
with TYPE; NULL otherwise.
Note: Two types T and T2 are said to be structurally compatible if the memory layout
of T2 appears at the beginning of the layout of T.*/
#define upcast(P, TYPE) ((TYPE*)_structural_subtyping((P), &definition_##TYPE))

/* The same as upcast but gets the reflection information from the structure referenced
by P2.*/
#define upcast_as(P, P2) (_structural_subtyping((P), reflect_header(P2)))

/* Accepts:
  * P - A pointer to data with reflection information.
Returns: The coercion of P if P points to a structure of typename TYPE; NULL otherwise.*/
#define recast(P, TYPE) ((TYPE*)_recast((P), &definition_##TYPE))

/* The same as recast but gets the reflection information from the structure referenced
by P2.*/
#define recast_as(P, P2) (_recast((P), reflect_header(P2)))

extern const struct REFLECT definition_int;
extern const struct REFLECT definition_long;
extern const struct REFLECT definition_short;
extern const struct REFLECT definition_char;
extern const struct REFLECT definition_float;
extern const struct REFLECT definition_double;
extern const struct REFLECT definition_time_t;

/*Purpose: Retrieves pointer to member by member name. The caller may optionally
provide a typename, in which case the result is type checked against the typename.
Returns: A pointer to member on success; NULL on failure.*/
static void* member_by_name(REFLECTABLE p, const char member_name[], const char typenm[]/*optional*/) {
	const struct REFLECT* header = reflect_header(p);
	unsigned int i;
	unsigned short bytesoff = 0;
	char* p2 = p;

	for (i=0;i<header->n_members;i++) {
		if (!strcmp(header->member_names[i], member_name)) {
			if(!typenm || !strcmp(header->members[i]->typenm, typenm)) {
				return p2;
			} else {
				throw("Typemismatch");
			}
		}
		p2 += header->members[i]->bytessize;
	}
	return NULL;
}

/*Purpose: Retrieves pointer to member by typename.
Returns: A pointer to member on success; NULL on failure.
Note: If there are multiple members with the same typename, the one with the smallest address
is selected.*/
static void* member_by_type(REFLECTABLE p, const char typenm[]) {
	const struct REFLECT* header = reflect_header(p);
	unsigned int i;
	unsigned short bytesoff = 0;
	char* p2 = (char*)p;

	for (i=0;i<header->n_members;i++) {
		if (!strcmp(header->members[i]->typenm, typenm)) return p2;
		p2 += header->members[i]->bytessize;
	}
	return NULL;
}

/*Purpose: Prints the name and members of the structure definition to stdout.*/
void reflect_print(const struct REFLECT* p);

#endif