/**
 * @file test_algebra.cpp
 * @author Janus
 * @brief Test cases for boolean algebra on the PCN/URP representation
 * @version 1.0.0
 * @date 2023-05-26
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <iostream>
#include <cstdio>

#include <urp/pcn.h>
#include <urp/algorithm.h>

using namespace testing;


/**********************************
 * BOOLEAN ALGEBRA : AND, OR, NOT * 
 *********************************/

class BooleanAlgebra: public Test {
public:
    pcn::Factor pos       = pcn::Factor{0b01};
    pcn::Factor neg       = pcn::Factor{0b10};
    pcn::Factor zero      = pcn::Factor{0b00};

    pcn::Factor one       = pcn::Factor{0b11};
    pcn::Factor dont_care = pcn::Factor{0b11};

    pcn::BooleanVariable x0  {0, pos};          // x0
    pcn::BooleanVariable x0_ {0, neg};          // x0'

    // 3-variable Product Term-Cubes.
    std::size_t dim = 3;
    pcn::Cube x0x1    {pos, pos, one};  // x0  * x1
    pcn::Cube   x1x2  {one, pos, pos};  //       x1 * x2
    pcn::Cube   x0x2_ {pos, one, neg};  // x0  *      x2'
    pcn::Cube     x2  {one, one, pos};  //            x2
    pcn::Cube     x2_ {one, one, neg};  //            x2'
    pcn::Cube x0x1x2  {pos, pos, pos};  // x0  * x1 * x2
    pcn::Cube x0_x1x2 {neg, pos, pos};  // x0' * x1 * x2

    // Product Terms for constructing F(x) SOPs -- all for 3 variables -- easier notation.
    pcn::Cube p = x1x2;
    pcn::Cube q = x2_;
    pcn::Cube s = x0_x1x2;
    pcn::Cube t = x0x2_;

    // Expected results for x0*F(x) = x0 * (p(x) + q(x)) = x0*p(x) + x*q(x).
    pcn::Cube x0p = x0x1x2;
    pcn::Cube x0q = x0x2_;

    // Test SOP vectors for H(x) = F(x) + G(x).
    // Note: 
    //  (p + q) + (s + t) = [(x1 * x2) + (x2')] + [(x0' * x1 * x2) + (x0 + x2')] -> Can be reduced to: x1*x2 + x2'.
    pcn::CubeList porq {p, q};          // (x1 * x2) + (x2')
    pcn::CubeList sort {s, t};          // (x0' * x1 * x2) + (x0 + x2')
    pcn::CubeList sorp {s, p};          // 

    // Expected results for complement (3 variables case)
    pcn::Cube not_x0 {neg, one, one};
    pcn::Cube not_x1 {one, neg, one};
    pcn::Cube not_x2 {one, one, neg};
    pcn::CubeList not_x0x1x2 {{not_x0}, {not_x1}, {not_x2}};    // not(x0x1x1) == x0' + x1' + x2'

};

// AND - Factor * Factor
TEST_F(BooleanAlgebra, PosAndPos_IsPos) {
    EXPECT_EQ(pcn::bool_and(pos, pos), pos);        // x * x == x
}

TEST_F(BooleanAlgebra, PosAndOne_IsPos) {
    EXPECT_EQ(pcn::bool_and(pos, one), pos);        // x * 1 == x
}

TEST_F(BooleanAlgebra, PosAndNeg_IsZero) {
    EXPECT_EQ(pcn::bool_and(pos, neg), zero);       // x * x' == 0
}

TEST_F(BooleanAlgebra, PosAndZero_IsZero) {
    EXPECT_EQ(pcn::bool_and(pos, zero), zero);      // x * 0 == 0
}

TEST_F(BooleanAlgebra, NegAndPos_IsZero) {
    EXPECT_EQ(pcn::bool_and(neg, pos), zero);       // x' * x == 0
}

TEST_F(BooleanAlgebra, NegAndOne_IsNeg) {
    EXPECT_EQ(pcn::bool_and(neg, one), neg);        // x' * 1 == x'
}

TEST_F(BooleanAlgebra, NegAndNeg_IsNeg) {
    EXPECT_EQ(pcn::bool_and(neg, neg), neg);        // x' * x' == x'
}

TEST_F(BooleanAlgebra, NegAndZero_IsZero) {
    EXPECT_EQ(pcn::bool_and(neg, zero), zero);      // x' * 0 == 0
}

// OR - Factor + Factor
TEST_F(BooleanAlgebra, PosOrPos_IsPos) {
    EXPECT_EQ(pcn::bool_or(pos, pos), pos);         // x + x == x
}

TEST_F(BooleanAlgebra, PosOrZero_IsPos) {
    EXPECT_EQ(pcn::bool_or(pos, zero), pos);        // x + 0 == x
}

TEST_F(BooleanAlgebra, PosOrOne_IsOne) {
    EXPECT_EQ(pcn::bool_or(pos, one), one);         // x + 1 == 1
}

TEST_F(BooleanAlgebra, PosOrNeg_IsOne) {
    EXPECT_EQ(pcn::bool_or(pos, neg), one);         // x + x' == 1
}

TEST_F(BooleanAlgebra, NegOrNeg_IsNeg) {
    EXPECT_EQ(pcn::bool_or(neg, neg), neg);         // x' + x' == x'
}

TEST_F(BooleanAlgebra, NegOrZero_IsNeg) {
    EXPECT_EQ(pcn::bool_or(neg, zero), neg);        // x' + 0 == x'
}

TEST_F(BooleanAlgebra, NegOrOne_IsOne) {
    EXPECT_EQ(pcn::bool_or(neg, one), one);         // x' + 1 == 1
}

TEST_F(BooleanAlgebra, NegOrPos_IsOne) {
    EXPECT_EQ(pcn::bool_or(neg, pos), one);         // x' + x == 1
}


// AND - Variable * Cube
TEST_F(BooleanAlgebra, PosBoolVar_And_Cube_Gives_CubeWith_PosFactor) {
    // (x0) * (x1 * x2) == x0 * x1 * x2
    EXPECT_EQ(pcn::bool_and(x0, x1x2), x0x1x2);
}

TEST_F(BooleanAlgebra, PosBoolVar_And_CubeWith_NegBoolVar_Gives_CubeWith_ZeroFactor) {
    // (x0) * (x0' * x1 * x2) == 0 * x1 * x2 -> 0
    EXPECT_EQ(pcn::bool_and(x0, x0_x1x2), pcn::Cube({zero, pos, pos})); 
}

TEST_F(BooleanAlgebra, NegBoolVar_And_Cube_Gives_CubeWith_NegFactor) {
    // (x0') * (x1 * x2) == x0' * x1 * x2
    EXPECT_EQ(pcn::bool_and(x0_, x1x2), x0_x1x2);
}

TEST_F(BooleanAlgebra, NegBoolVar_And_CubeWith_PosBoolVar_Gives_CubeWith_ZeroFactor) {
    // (x0') * (x0 * x1 * x2) == 0 * x1 * x2 -> 0
    EXPECT_EQ(pcn::bool_and(x0_, x0x1x2), pcn::Cube({zero, pos, pos}));
}

// AND - Variable * CubeList
TEST_F(BooleanAlgebra, VarAndCubeList_DistributesVarOverCubes) {
    // x*(p + q) = x*p + x*q.
    EXPECT_THAT(pcn::bool_and(x0, porq), UnorderedElementsAre(x0p, x0q));    // No guarantee on the ordering
}

// OR - CubeList + CubeList
TEST_F(BooleanAlgebra, CubeListOr_MakesUnionOfLists) {
    // (p + q) + (s + t) == p + q + s + t.
    EXPECT_EQ(pcn::bool_or(porq, sort).size(), 4);                              // 4 Product Terms in SOP
    EXPECT_THAT(pcn::bool_or(porq, sort), UnorderedElementsAre(p, q, s, t));    // No guarantee on the ordering
}

TEST_F(BooleanAlgebra, CubeListOr_OnSameCubeList_ReturnsIdenticalCubeList) {
    // (p + q) + (p + q) == p + q.
    EXPECT_EQ(pcn::bool_or(porq, porq).size(), 2);                              // 2 Product Terms in SOP
    EXPECT_THAT(pcn::bool_or(porq, porq), UnorderedElementsAre(p, q));          // No guarantee on the ordering
}

TEST_F(BooleanAlgebra, CubeListOr_OnOtherCubeList_WithRepeatedElements_OmitsDuplicatedCubes) {
    // (p + q) + (s + p) == p + q + s.
    EXPECT_EQ(pcn::bool_or(porq, sorp).size(), 3);                              // 3 Product Terms in SOP as "p" was duplicated
    EXPECT_THAT(pcn::bool_or(porq, sorp), UnorderedElementsAre(p, q, s));       // No guarantee on the ordering
}

// NOT (complement) - Factor
TEST_F(BooleanAlgebra, NotPos_IsNeg) {
    EXPECT_EQ(pcn::bool_not(pos), neg);        // (x)' == x'
}

TEST_F(BooleanAlgebra, NotNeg_IsPos) {
    EXPECT_EQ(pcn::bool_not(neg), pos);        // (x')' == x
}

TEST_F(BooleanAlgebra, NotOne_IsZero) {
    EXPECT_EQ(pcn::bool_not(one), zero);       // (1)' == 0
}

TEST_F(BooleanAlgebra, NotZero_IsOne) {
    EXPECT_EQ(pcn::bool_not(zero), one);       // (0)' == 1
}

// NOT (complement) - Cube
TEST_F(BooleanAlgebra, CubeComplement_IsSumOfComplementedFactors) {
    // (x0*x1*x2)' == x0' + x1' + x2'.
    EXPECT_EQ(pcn::bool_not(x0x1x2).size(), 3);                                 // 3 Product Terms in SOP
    EXPECT_THAT(pcn::bool_not(x0x1x2), UnorderedElementsAre(not_x0, not_x1, not_x2));
}

// NOT (complement) - CubeList
TEST_F(BooleanAlgebra, ComplementOf_TautologyCubeList_IsZero) {
    auto tautology = pcn::CubeList {{0b11}};
    auto complement = pcn::bool_not(tautology);
    EXPECT_TRUE(complement.is_zero());
}

TEST_F(BooleanAlgebra, ComplementOf_ZeroCubeList_IsOne) {
    auto falsity = pcn::CubeList(dim);                // F(x0, x1, x2) = 0
    auto complement = pcn::bool_not(falsity);
    EXPECT_TRUE(pcn::is_tautology(complement));
}

TEST_F(BooleanAlgebra, ComplementOf_SingleCube_CubeList_IsIdenticalTo_ComplementOf_Cube) {
    auto cube_list = pcn::CubeList { p };               // F  = x1  * x2  ~>  <[11 01 01]>
    auto list_complement = pcn::bool_not(cube_list);    // F' = x1' + x2' ~>  <[11 10 11], [11 11 10]>
                                                        //                       not_x1      not_x2
    EXPECT_THAT(list_complement, UnorderedElementsAre(not_x1, not_x2));
    
    auto cube_complement = pcn::bool_not(p);
    EXPECT_THAT(cube_complement, UnorderedElementsAre(not_x1, not_x2));
}

TEST_F(BooleanAlgebra, ComplementOf_MultiCube_UnateCubeList_MatchesManualAlgorithm) {
    // F = x0 x1 + x2                               ~> All xi are unate and all appear in 1 term. Choose lowest index to split: x0
    // F' = x0 (F_x0)' + x0' (F_x0')'               ~> Complement expansion around x0.
    // F_x0 = x1 + x2                               ~> Positive cofactor for x0.
    // (F_x0)' = x1 (F_x0x1)' + x1' (F_x0x1')'      ~> Complement of positive cofactor. Use complement expansion around x1.
    //         = x1 * (1)' + x1' x2' = x1'x2'       ~> 
    // F_x0' = x2                                   ~> Negative cofactor for x0.
    // (F_x0')' = x2'                               ~> Complement of negative cofactor.
    // F' = x0 (x1'x2') + x0' (x2')                 ~> Fill into complement expansion around x0.
    // F' = x0 x1' x2' + x0'   x2'                  ~> Final result from manual algorithm
    //   < [01 10 10],  [10 11 10] >                ~> PCN form of manual algorithm result
    //
    // F' = x2' (x0' + x0 x1')                      ~> Manual algebraic manipulation
    //    = x2' (x0' + x1')                         ~> Using identity: (x+yz) = (x+y)(x+z) -> (x0'+x0x1') = (x0'+x0)(x0'+x1') = x0'+x1'.
    //    = x0'   x2' +    x1' x2'                  ~> By hand manipulation.
    //   < [10 11 10] , [11 10 10] >                ~> PCN form of hand manipulation.

    auto cube_list = pcn::CubeList { x0x1, x2 };        // F  = x0 * x1 + x2    ~>  <[01 01 11], [11 11 01]>
    auto list_complement = pcn::bool_not(cube_list);    //                      ~>  <[01 10 10], [10 11 10]>
    auto cube1 = pcn::Cube{0b01, 0b10, 0b10};           // x0 * x1' * x2'
    auto cube2 = pcn::Cube{0b10, 0b11, 0b10};           // x0'      * x2'
    EXPECT_THAT(list_complement, UnorderedElementsAre(cube1, cube2));
}