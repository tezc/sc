#include "sc_disjoint.h"

void
sc_disjoint_init( struct sc_disjoint_node* node, const void* data )
{
    node->parent = node;
    node->rank   = 0;
    node->size   = 1;
    node->data   = data;
}

struct sc_disjoint_node*
sc_disjoint_parent( struct sc_disjoint_node* node )
{
    struct sc_disjoint_node* root = node;

    while( root != root->parent )
        root = root->parent;

    // Optimization for find:  Update EACH parent to point to root
    while( node != node->parent ){
        struct sc_disjoint_node* p = node->parent;
        node->parent = root;
        node = p;
    }

    return root;
}

void
sc_disjoint_merge( struct sc_disjoint_node* a, struct sc_disjoint_node* b )
{
    struct sc_disjoint_node* pa = sc_disjoint_parent( a );
    struct sc_disjoint_node* pb = sc_disjoint_parent( b );

    if( pa == pb )
        return;

    if( pa->rank < pb->rank ){
        struct sc_disjoint_node* temp = pa;
        pa = pb;
        pb = temp;
    }
    pb->parent = pa;

    if( pa->rank == pb->rank ){
        pa->rank ++;
    }

    pa->size += pb->size;
}
