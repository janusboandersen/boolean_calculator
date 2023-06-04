/**
 * @file test_algebra.cpp
 * @author Janus
 * @brief Test cases for cofactoring SOPs (CubeLists), Terms (Cubes) and Factors.
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


/*********************
 * SHANNON COFACTORS *
 ********************/

class CofactorComputation: public Test {
public:
    // Polarities x and x'
    pcn::Factor pos = pcn::Factor::pos();       // 01
    pcn::Factor neg = pcn::Factor::neg();       // 10

    // Don't care polarity
    pcn::Factor dont_care = pcn::Factor::one(); // 11
    pcn::Factor one = pcn::Factor::one();       // 11

    // Zero polarity
    pcn::Factor zero = pcn::Factor::zero();     // 00

    // Term for 3 variables, initialized as [11 11 11]
    pcn::Cube initialized_3 = pcn::Cube(3);
    
};

// Factors - Positive cofactors
TEST_F(CofactorComputation, PosCofactored_PosPolarity_Becomes_DontCare) {
    EXPECT_EQ(pcn::positive_cofactor(pos), one);    // x | x=1 -> 1 -> "11"
}

TEST_F(CofactorComputation, PosCofactored_NegPolarity_Becomes_Zero) {
    EXPECT_EQ(pcn::positive_cofactor(neg), zero);   // x' | x=1 -> 0 -> "00"
}

TEST_F(CofactorComputation, PosCofactored_DontCare_Is_DontCare) {
    EXPECT_EQ(pcn::positive_cofactor(one), one);    // 1 | x=1 -> 1 -> "11"
}

TEST_F(CofactorComputation, PosCofactored_Zero_Is_Zero) {
    EXPECT_EQ(pcn::positive_cofactor(zero), zero);  // 1 | x=1 -> 1 -> "11"
}

// Factors - Negative cofactors
TEST_F(CofactorComputation, NegCofactored_PosPolarity_Becomes_Zero) {
    EXPECT_EQ(pcn::negative_cofactor(pos), zero);   // x | x=0 -> 0 -> "00"
}

TEST_F(CofactorComputation, NegCofactored_NegPolarity_Becomes_DontCare) {
    EXPECT_EQ(pcn::negative_cofactor(neg), one);    // x' | x=0 -> 1 -> "11"
}

TEST_F(CofactorComputation, NegCofactored_DontCare_Is_DontCare) {
    EXPECT_EQ(pcn::negative_cofactor(one), one);    // - | x=0 -> - -> "11"
}

TEST_F(CofactorComputation, NegCofactored_Zero_Is_Zero) {
    EXPECT_EQ(pcn::negative_cofactor(zero), zero);  // 0 | x=0 -> 0 -> "00"
}

// Cubes - Positive cofactors
TEST_F(CofactorComputation, PosCofactored_CubeWithPosPolarityVar_OnlyChangesSubscriptVariable) {
    auto F = pcn::Cube{dont_care, pos, pos};            // F(x0, x1, x2)          = x1 * x2  -> [11 01 01]     
    EXPECT_EQ(pcn::positive_cofactor(F, 1), 
              pcn::Cube({dont_care, dont_care, pos}));  // F_x1 = F(x0, x1=1, x2) = 1  * x2  -> [11 11 01]
}

TEST_F(CofactorComputation, PosCofactored_CubeWithNegPolarityVar_OnlyChangesSubscriptVariable) {
    auto F = pcn::Cube{dont_care, neg, neg};            // F(x0, x1, x2)          = x1' * x2' -> [11 10 10]
    EXPECT_EQ(pcn::positive_cofactor(F, 1), 
              pcn::Cube({dont_care, zero, neg}));       // F_x1 = F(x0, x1=1, x2) =  0  * x2' -> [11 00 10] -> 0
}

// Cubes - Negative cofactors
TEST_F(CofactorComputation, NegCofactored_CubeWithPosPolarityVar_OnlyChangesSubscriptVariable) {
    auto F = pcn::Cube{dont_care, pos, pos};            // F(x0, x1, x2)          = x1 * x2 -> [11 01 01]
    EXPECT_EQ(pcn::negative_cofactor(F, 1), 
              pcn::Cube({dont_care, zero, pos}));       // F_x1 = F(x0, x1=0, x2) =  0 * x2 -> [11 00 01] -> 0
}

TEST_F(CofactorComputation, NegCofactored_CubeWithNegPolarityVar_OnlyChangesSubscriptVariable) {
    auto F = pcn::Cube{dont_care, neg, neg};            // F(x0, x1, x2)          = x1' * x2' -> [11 10 10]
    EXPECT_EQ(pcn::negative_cofactor(F, 1), 
              pcn::Cube({dont_care, dont_care, neg}));  // F_x1 = F(x0, x1=0, x2) =  1  * x2' -> [11 11 10]
}

// CubeLists - Positive cofactors
TEST_F(CofactorComputation, CubeList_PositiveCofactor) {
    auto cube_list = pcn::CubeList{{0b11, 0b01, 0b10}, {0b01, 0b10, 0b01}}; // [11 01 10], [01 10 01]
    auto cofactor = pcn::positive_cofactor(cube_list, 1);                   // [11 11 10], [01 00 01]
    auto ref_cube = pcn::Cube{0b11, 0b11, 0b10};                            // [11 11 10],  --zero--
    EXPECT_EQ(*cofactor.begin(), ref_cube);                                 // First (and only) element of the CubeList must eq. reference result
}

// CubeLists - Negative cofactors
TEST_F(CofactorComputation, CubeList_NegativeCofactor) {
    auto cube_list = pcn::CubeList{{0b11, 0b01, 0b10}, {0b01, 0b10, 0b01}}; // [11 01 10], [01 10 01]
    auto cofactor = pcn::negative_cofactor(cube_list, 1);                   // [11 00 10], [01 11 01]
    auto ref_cube = pcn::Cube{0b01, 0b11, 0b01};                            //  --zero-- , [01 11 01],  
    EXPECT_EQ(*cofactor.begin(), ref_cube);                                 // First (and only) element of the CubeList must eq. reference result
}
