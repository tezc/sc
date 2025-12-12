#include "sc_disjoint.h"
#include "unity.h"

// Requires Unity test harness (MIT License)
// https://github.com/ThrowTheSwitch/Unity

void setUp(void) {
}
void tearDown(void) {
}

typedef struct {
    int val;
} dummy;

void test_init()
{
    struct sc_disjoint_node uf;
    dummy data = { 1 };

    sc_disjoint_init( &uf, &data );

    TEST_ASSERT_TRUE( uf.parent == &uf );
    TEST_ASSERT_EQUAL( 0, uf.rank );
    TEST_ASSERT_EQUAL( 1, uf.size );
    TEST_ASSERT_EQUAL( &data, uf.data );

    TEST_ASSERT_EQUAL( 1, data.val );
}

void test_init_nodata()
{
    struct sc_disjoint_node uf;

    sc_disjoint_init( &uf, NULL );

    TEST_ASSERT_TRUE( uf.parent == &uf );
    TEST_ASSERT_EQUAL( 0, uf.rank );
    TEST_ASSERT_EQUAL( NULL, uf.data );
}

void test_trivial()
{
    struct sc_disjoint_node uf;
    dummy data = { 1 };

    sc_disjoint_init( &uf, &data );

    TEST_ASSERT_EQUAL( &uf, sc_disjoint_parent(&uf) );
}

void test_merge()
{
    struct sc_disjoint_node n1, n2, n3;

    sc_disjoint_init( &n1, NULL );
    sc_disjoint_init( &n2, NULL );
    sc_disjoint_init( &n3, NULL );

    sc_disjoint_merge( &n1, &n2 );
    TEST_ASSERT_EQUAL( sc_disjoint_parent(&n1), sc_disjoint_parent(&n2) );

    TEST_ASSERT_NOT_EQUAL( sc_disjoint_parent(&n1), sc_disjoint_parent(&n3) );
    TEST_ASSERT_NOT_EQUAL( sc_disjoint_parent(&n2), sc_disjoint_parent(&n3) );

    TEST_ASSERT_EQUAL( 1, sc_disjoint_parent(&n1)->rank );
    TEST_ASSERT_EQUAL( 1, sc_disjoint_parent(&n2)->rank );
    TEST_ASSERT_EQUAL( 0, sc_disjoint_parent(&n3)->rank );

    TEST_ASSERT_EQUAL( 2, sc_disjoint_parent(&n1)->size );
    TEST_ASSERT_EQUAL( 2, sc_disjoint_parent(&n2)->size );
    TEST_ASSERT_EQUAL( 1, sc_disjoint_parent(&n3)->size );
}

int main() {
    UNITY_BEGIN();

    RUN_TEST( test_init );
    RUN_TEST( test_init_nodata );
    RUN_TEST( test_trivial );
    RUN_TEST( test_merge );

    return UNITY_END();
}
