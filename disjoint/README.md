### Disjoint Set

### Overview

- Disjoint Set (aka Union Find)
- Amortized O(1) Find operation
- O(1) Size and Merge operations
- Each node holds optional pointer to any data type

### Notes
Data pointer is entirely ignored by the disjoint-set operations, so its purpose is flexible.

### Usage

#### **Non-intrusive usage**
- mystruct contains user data, and does not know about the disjoint-set
- User must keep track of all disjoint_nodes seperately

```c
    typedef struct {
        int val;
        char*  name;
    } mystruct;

    mystruct data[3] = {
        {1, "hello"},
        {2, "disjoint"},
        {3, "set"},
    };

    struct sc_disjoint_node nodes[3];
    for( int i = 0; i < 3; i++ ){
        sc_disjoint_init( &nodes[i], &data[i] );
    }

    sc_disjoint_merge( sc_disjoint_parent(&nodes[0]),
                       sc_disjoint_parent(&nodes[1]));

    assert( sc_disjoint_parent(&nodes[0]) ==
            sc_disjoint_parent(&nodes[1]));

    int numnodes = sc_disjoint_parent(&nodes[0])->size;
```

#### **Intrusive usage**
- mystruct is modified to contain internal node structure
- User only keeps track of mystruct (e.g. array etc)
- The data pointer of node may point to mystruct or may be NULL

```c
    typedef struct {
        int val;
        char*  name;

        struct sc_disjoint_node djn;

    } mystruct;

    mystruct data[3] = {
        {1, "hello"},
        {2, "disjoint"},
        {3, "set"},
    };

    for( int i = 0; i < 3; i++ ){
        sc_disjoint_init( &data[i].djn, &data[i] );

        // This would also be valid, depending on use case/convenience
        // sc_disjoint_init( &data[i].djn, NULL );
    }

    sc_disjoint_merge( sc_disjoint_parent(&data[0].djn),
                       sc_disjoint_parent(&data[1].djn));

    assert( sc_disjoint_parent(&data[0].djn) ==
            sc_disjoint_parent(&data[1].djn));

    int numdata = sc_disjoint_parent(&data[0].djn)->size;
```
