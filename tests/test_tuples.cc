#include "catch.hpp"
#include <tuple.h>
#include <memory>
#include <climits>
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

TEST_CASE( "String datums work properly", "[tuples]" ) {
    DatumP foo = make_unique<StringDatum>("foo");
    DatumP bar = make_unique<StringDatum>("bar");
    REQUIRE ( *foo != *bar );
    REQUIRE ( *foo == *foo );
    REQUIRE ( *(foo->clone()) == *foo );
    REQUIRE ( *(bar->clone()) == *bar );
    REQUIRE ( foo->toString() == "foo" );
    REQUIRE ( bar->toString() == "bar" );
}

TEST_CASE( "Double datums work properly", "[tuples]" ) {
    DatumP a = make_unique<DoubleDatum>(-0.1);
    DatumP b = make_unique<DoubleDatum>(2.32);
    REQUIRE ( *a != *b );
    REQUIRE ( *a < *b );
    REQUIRE ( *b > *a );
    REQUIRE ( *(a->clone()) == *a );
    REQUIRE ( *(b->clone()) == *b );
}

TEST_CASE( "BigInt datums work properly", "[tuples]" ) {
    DatumP a = make_unique<BigIntDatum>(LLONG_MIN);
    DatumP b = make_unique<BigIntDatum>(LLONG_MAX);
    DatumP c = make_unique<BigIntDatum>(-2202);
    REQUIRE ( *a != *b );
    REQUIRE ( *a < *b );
    REQUIRE ( *b > *a );
    REQUIRE ( (*c > *a && *c < *b) );
    REQUIRE ( *(a->clone()) == *a );
    REQUIRE ( *(b->clone()) == *b );
}

TEST_CASE( "Date datums work properly", "[tuples]" ) {
    DatumP a = make_unique<DateDatum>(Date(2012,1,1));
    DatumP b = make_unique<DateDatum>(Date(2013,11,21));
    REQUIRE ( *a != * b );
    REQUIRE ( *a < *b );
    REQUIRE ( *b > *a );
    REQUIRE ( *(a->clone()) == *a );
    REQUIRE ( *(b->clone()) == *b );
    REQUIRE ( a->toString() == "2012-01-01" );
    REQUIRE ( b->toString() == "2013-11-21" );
}

TEST_CASE( "tupleFromString, basic test", "[tuples]" ) {
    Schema schema1 { TYPE_INT, TYPE_TEXT };
    
    TupleP tuple1 = tupleFromString("1,hey there!", schema1);
    REQUIRE ( fieldValue<int>(tuple1, 0) == 1 );
    REQUIRE ( fieldValue<string>(tuple1, 1) == "hey there!" );
    
    TupleP tuple2 = tupleFromString("2147483647, Q~~~Q ", schema1);
    REQUIRE ( fieldValue<int>(tuple2, 0) == 2147483647 );
    REQUIRE ( fieldValue<string>(tuple2, 1) == " Q~~~Q " );
}

TEST_CASE( "tupleFromString, test all data types", "[tuples]" ) {
    Schema schema1 { TYPE_INT, TYPE_TEXT, TYPE_DECIMAL, TYPE_BIGINT, TYPE_DATE,
                     TYPE_BOOL };
    TupleP tuple1 = tupleFromString("1,hey there!,1e10,12345678901,2012-04-23,true", schema1);
    REQUIRE ( fieldValue<int>(tuple1, 0) == 1 );
    REQUIRE ( fieldValue<string>(tuple1, 1) == "hey there!" );
    REQUIRE ( fieldValue<double>(tuple1, 2) == 1e10 );
    REQUIRE ( fieldValue<long long>(tuple1, 3) == 12345678901 );
    REQUIRE ( fieldValue<Date>(tuple1, 4) == Date(2012, 4, 23) );
    REQUIRE ( fieldValue<bool>(tuple1, 5) == true );
}

TEST_CASE ( "tupleToString, basic test", "[tuples]" ) {
    Schema schema { TYPE_INT, TYPE_TEXT, TYPE_DECIMAL, TYPE_BIGINT, TYPE_DATE,
                    TYPE_BOOL };
    string tupleStr = "1,hey there!,10000000000.00,12345678901,2012-04-23,1";
    TupleP tuple = tupleFromString(tupleStr, schema);
    REQUIRE ( tupleToString(*tuple) == tupleStr );
}

TEST_CASE ( "tuple (de)serialization, escaping", "[tuples]" ) {
    Schema schema { TYPE_INT, TYPE_TEXT, TYPE_INT };
    string tupleStr1 = "1,hadi\\,salam\\\\,12";
    TupleP tuple1 = tupleFromString(tupleStr1, schema);
    REQUIRE ( fieldValue<int>(tuple1, 0) == 1 );
    REQUIRE ( fieldValue<string>(tuple1, 1) == "hadi,salam\\" );
    REQUIRE ( fieldValue<int>(tuple1, 2) == 12 );
    REQUIRE ( tupleToString(*tuple1) == tupleStr1 );
    string tupleStr2 = "\\2|\\||32";
    TupleP tuple2 = tupleFromString(tupleStr2, schema, '|');
    REQUIRE ( fieldValue<int>(tuple2, 0) == 2 );
    REQUIRE ( fieldValue<string>(tuple2, 1) == "|" );
    REQUIRE ( fieldValue<int>(tuple2, 2) == 32 );
    REQUIRE ( tupleToString(*tuple2, '|') == "2|\\||32" );

}
