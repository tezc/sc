#include "sc_disjoint.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int example(void)
{

    struct dummy {
        const char* val;
    };

  struct dummy data[] = {
         {"zero"},
         {"one"},
         {"two"},
         {"three"},
         {"four"}};

    struct sc_disjoint_node nodes[5];

  for (int i = 0; i < 5; i++) {
    sc_disjoint_init( &nodes[i], &data[i] );
  }

    // Set0: contains 2 and 3
    // Set1: contains 0 and 1
    // Set2: contains 4
    sc_disjoint_merge( &nodes[2], &nodes[3] );
    sc_disjoint_merge( &nodes[0], &nodes[1] );

    struct sc_disjoint_node *parent0, *parent1, *parent2;

    // All nodes in the same set share the same parent
    // Compare parents of two nodes to determine if they are in the same set
    parent0 = sc_disjoint_parent( &nodes[2] );
    parent1 = sc_disjoint_parent( &nodes[0] );
    parent2 = sc_disjoint_parent( &nodes[4] );

    // Set0: contains 2 and 3
    assert( parent0 == sc_disjoint_parent( &nodes[2]) );
    assert( parent0 == sc_disjoint_parent( &nodes[3]) );

    // Set1: contains 0 and 1
    assert( parent1 == sc_disjoint_parent( &nodes[0]) );
    assert( parent1 == sc_disjoint_parent( &nodes[1]) );

    // Set2: contains 4
    assert( parent2 == sc_disjoint_parent( &nodes[4]) );

    // Sets 0, 1 and 2 are disjoint
    assert( parent0 != parent1 );
    assert( parent0 != parent2 );
    assert( parent1 != parent2 );

    // Rank is upper-bound on max tree depth
    // When 2 nodes merge, the lower-rank tree joins the higher-rank tree
    // Multiple 'find' operations eventually flatten the tree to height=1
    assert( parent0->rank == 1 );
    assert( parent1->rank == 1 );
    assert( parent2->rank == 0 );

    // Parent always contains exact number of all nodes in set
    assert( parent0->size == 2 );
    assert( parent1->size == 2 );
    assert( parent2->size == 1 );

  return 0;
}


int main(void)
{
  example();

  return 0;
}
