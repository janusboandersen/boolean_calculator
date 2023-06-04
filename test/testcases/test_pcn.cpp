/**
 * @file test_pcn.cpp
 * @author Janus
 * @brief 
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


/*************************
 * FACTOR DATA STRUCTURE *
 ************************/ 

class FactorBooleanRepresentation: public Test {
public:
    // Polarities x and x'
    pcn::Factor pos = pcn::Factor{0b01};
    pcn::Factor neg = pcn::Factor{0b10};

    // Don't care
    pcn::Factor one = pcn::Factor{0b11};

    // Zero
    pcn::Factor zero = pcn::Factor{0b00};
};

// Equality after initialization
TEST_F(FactorBooleanRepresentation, FactorVsLiteralEqualityPositive) {
    EXPECT_EQ(one, 0b11);
}

TEST_F(FactorBooleanRepresentation, FactorVsLiteralEqualityNegative) {
    EXPECT_FALSE((one==0b10));
}

TEST_F(FactorBooleanRepresentation, FactorValueRepresentationMustMatchInitialization) {
    EXPECT_EQ(zero, 0b00);
    EXPECT_EQ(pos, 0b01);
    EXPECT_EQ(neg, 0b10);
    EXPECT_EQ(one, 0b11);
}

TEST_F(FactorBooleanRepresentation, FactorValueRepresentationMustMatchDecimal) {
    EXPECT_EQ(zero, 0);
    EXPECT_EQ(pos, 1);
    EXPECT_EQ(neg, 2);
    EXPECT_EQ(one, 3);
}


/***********************
 * CUBE DATA STRUCTURE *
 **********************/ 

class CubeBooleanRepresentation: public Test {
public:
    pcn::Cube initialized_3 = pcn::Cube(3);
    pcn::Factor pos = pcn::Factor::pos();
    pcn::Factor neg = pcn::Factor::neg();
    pcn::Factor dont_care = pcn::Factor::one();
    pcn::Factor zero = pcn::Factor::zero();
};

// Initialization
TEST_F(CubeBooleanRepresentation, CubeWithSizeInit_InitializesToDontCare) {
    EXPECT_EQ(initialized_3.at(0), dont_care);
    EXPECT_EQ(initialized_3.at(1), dont_care);
    EXPECT_EQ(initialized_3.at(2), dont_care);
}

TEST_F(CubeBooleanRepresentation, CubeWithFactorListInit_InitializesToSameFactors) {
    auto cube = pcn::Cube{dont_care, pos, neg}; // Should initialize to [11 01 10]
    EXPECT_EQ(cube.at(0), dont_care);
    EXPECT_EQ(cube.at(1), pos);
    EXPECT_EQ(cube.at(2), neg);
}

TEST_F(CubeBooleanRepresentation, CubeWithLiteralListInit_InitializesToSameFactorLiterals) {
    auto cube = pcn::Cube{0b11, 0b01, 0b10};    // Should initialize to [11 01 10]
    EXPECT_EQ(cube.at(0), dont_care);
    EXPECT_EQ(cube.at(1), pos);
    EXPECT_EQ(cube.at(2), neg);
}

// Subscripting / dereferencing to modifiable value
TEST_F(CubeBooleanRepresentation, CubeIndexationIsEditableWithLiteral) {
    initialized_3.at(1) = 0b01;                 // set x1 = "01"
    EXPECT_EQ(initialized_3.at(1), pos);
}

TEST_F(CubeBooleanRepresentation, CubeIndexationIsEditableWithObject) {
    initialized_3.at(1) = pos;                  // set x1 = "01"
    EXPECT_EQ(initialized_3.at(1), pos);
}

// Equality
TEST_F(CubeBooleanRepresentation, CubeEquality_TrueFor_EmptyCubes) {
    EXPECT_EQ(pcn::Cube(), pcn::Cube());
}

TEST_F(CubeBooleanRepresentation, CubeEquality_TrueFor_EqualCubes) {
    auto cube1 = pcn::Cube{dont_care, pos, dont_care};
    auto cube2 = cube1;                         // copy-assignment
    EXPECT_EQ(cube1, cube2);
}

TEST_F(CubeBooleanRepresentation, CubeInequality_TrueFor_DifferentSizedCubes) {
    EXPECT_NE(pcn::Cube(), pcn::Cube({pos, neg}) );
}

TEST_F(CubeBooleanRepresentation, CubeInequality_TrueFor_DifferentValuedCubes) {
    EXPECT_NE(pcn::Cube({0b01, 0b10}), pcn::Cube({0b10, 0b01}) );
}

/****************************
 * CUBE-LIST DATA STRUCTURE *
 ***************************/ 

// Initialization
TEST(TestCubeListRepresentation, CubeList_WithLiteralListInit_InitializesTo_SameCubes) {
    auto cube_list = pcn::CubeList{{0b11, 0b01, 0b10}, 
                                   {0b01, 0b10, 0b01}};     // Should initialize to <[11 01 10], [01 10 01]>
    auto it = cube_list.cbegin();

    EXPECT_EQ(cube_list.size(), 2);                         // Correct size
    EXPECT_EQ(*it,     pcn::Cube({0b11, 0b01, 0b10}));      // First cube correct
    EXPECT_EQ(*(++it), pcn::Cube({0b01, 0b10, 0b01}));      // Second cube correct
}

