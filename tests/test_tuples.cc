#include "catch.hpp"
#include <tuple.h>
#include <memory>
using namespace std;

TEST_CASE( "Int datums can be multiplied", "[tuples]" ) {
    IntDatum a(11), b(12);
    DatumP ab = a.multiply(b);
    IntDatum *ab_numeric = static_cast<IntDatum *>(ab.get());
    REQUIRE ( ab_numeric->value == 132 );
}

TEST_CASE( "Int datums can be compared", "[tuples]" ) {
    DatumP a = make_unique<IntDatum>(11);
    DatumP b = make_unique<IntDatum>(12);
    REQUIRE ( *a < *b );
    REQUIRE ( !(*a < *a) );
    REQUIRE ( !(*b < *a) );
    REQUIRE ( !(*b < *b) );

    REQUIRE ( *a <= *b );
    REQUIRE ( *a == *a );
    REQUIRE ( *b == *b );
    REQUIRE ( *b >= *a );
    REQUIRE ( *b > *a );
}

TEST_CASE( "Int datums can be cloned", "[tuples]" ) {
    DatumP a = make_unique<IntDatum>(11);
    DatumP b = a->clone();
    REQUIRE ( (!(*a < *b) && !(*b < *a)) );
}

TEST_CASE( "Int datums can be stringified", "[tuples]" ) {
    DatumP a = make_unique<IntDatum>(11);
    REQUIRE ( a->toString() == "11" );
}

TEST_CASE( "Bool datums work properly", "[tuples]" ) {
    DatumP t = make_unique<BoolDatum>(true);
    DatumP f = make_unique<BoolDatum>(false);
    REQUIRE ( *t != *f );
    REQUIRE ( *f != *t );
    REQUIRE ( *t == *t );
    REQUIRE ( *f == *f );
    REQUIRE ( *(f->clone()) == *f );
    REQUIRE ( *(t->clone()) == *t );
}
