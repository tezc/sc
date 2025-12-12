#ifndef SC_DISJOINT_SET
#define SC_DISJOINT_SET

#include <stddef.h>

struct sc_disjoint_node {
    struct sc_disjoint_node* parent;
    size_t         rank;
    size_t         size;

    const void*    data;
};

void
sc_disjoint_init( struct sc_disjoint_node* node, const void* data );

struct sc_disjoint_node*
sc_disjoint_parent( struct sc_disjoint_node* node );

void
sc_disjoint_merge( struct sc_disjoint_node* a, struct sc_disjoint_node* b );

#endif
