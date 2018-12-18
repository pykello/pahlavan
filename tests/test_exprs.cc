#include "catch.hpp"
#include <expr.h>
#include <tuple.h>
#include <memory>
#include <climits>
using namespace std;

TEST_CASE ( "ConstExpr", "[exprs]" ) {
    Tuple tuple;
    
    /* int */
    unique_ptr<Expr> e1 = make_unique<ConstExpr>(make_unique<IntDatum>(123));
    REQUIRE ( datumValue<int>(*e1->eval(tuple)) == 123 );

    /* string */
    unique_ptr<Expr> e2 = make_unique<ConstExpr>(make_unique<StringDatum>("hello"));
    REQUIRE ( datumValue<string>(*e2->eval(tuple)) == "hello" );

    /* bool */
    unique_ptr<Expr> e3 = make_unique<ConstExpr>(make_unique<BoolDatum>(false));
    REQUIRE ( datumValue<bool>(*e3->eval(tuple)) == false );

    /* double */
    unique_ptr<Expr> e4 = make_unique<ConstExpr>(make_unique<DoubleDatum>(1e10));
    REQUIRE ( datumValue<double>(*e4->eval(tuple)) == 1e10 );

    /* long long */
    unique_ptr<Expr> e5 = make_unique<ConstExpr>(make_unique<BigIntDatum>(12345678901ll));
    REQUIRE ( datumValue<long long>(*e5->eval(tuple)) == 12345678901ll );

    /* date */
    unique_ptr<Expr> e7 = make_unique<ConstExpr>(make_unique<DateDatum>(Date(2017,1,13)));
    REQUIRE ( datumValue<Date>(*e7->eval(tuple)) == Date(2017, 1, 13) );
}

TEST_CASE ( "VarExpr", "[exprs]" ) {
    Tuple tuple;
    tuple.push_back(make_unique<IntDatum>(383));
    tuple.push_back(make_unique<StringDatum>("hey there"));
    tuple.push_back(make_unique<DateDatum>(Date(1992, 05, 18)));
    tuple.push_back(make_unique<BoolDatum>(true));

    unique_ptr<Expr> e1 = make_unique<VarExpr>(0);
    REQUIRE ( datumValue<int>(*e1->eval(tuple)) == 383 );

    unique_ptr<Expr> e2 = make_unique<VarExpr>(1);
    REQUIRE ( datumValue<string>(*e2->eval(tuple)) == "hey there" );

    unique_ptr<Expr> e3 = make_unique<VarExpr>(2);
    REQUIRE ( datumValue<Date>(*e3->eval(tuple)) == Date(1992, 5, 18) );

    unique_ptr<Expr> e4 = make_unique<VarExpr>(3);
    REQUIRE ( datumValue<bool>(*e4->eval(tuple)) == true );
}

TEST_CASE ( "MultExpr", "[exprs]" ) {
    Tuple tuple;
    tuple.push_back(make_unique<IntDatum>(12));
    tuple.push_back(make_unique<IntDatum>(13));

    unique_ptr<Expr> e1 = make_unique<MultExpr>(
        make_unique<ConstExpr>(make_unique<IntDatum>(2)),
        make_unique<ConstExpr>(make_unique<IntDatum>(3))
    );
    REQUIRE ( datumValue<int>(*e1->eval(tuple)) == 6 );
    
    unique_ptr<Expr> e2 = make_unique<MultExpr>(
        make_unique<ConstExpr>(make_unique<IntDatum>(7)),
        make_unique<MultExpr>(
            make_unique<VarExpr>(0),
            make_unique<VarExpr>(1)
        )
    );

    REQUIRE ( datumValue<int>(*e2->eval(tuple)) == 1092 );
}