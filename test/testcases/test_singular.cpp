/**
 * @file test_singular.cpp
 * @author Janus
 * @brief Test cases for when PCN/URP representation is singular, F==0 or F==1. Zero/Falsity and Tautology.
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


/*******************************
 * TAUTOLOGY AND ZERO CHECKING *
 ******************************/

class SingularityDetection: public Test {
public:
    pcn::Cube initialized_3 = pcn::Cube(3);
    pcn::Factor pos = pcn::Factor::pos();
    pcn::Factor neg = pcn::Factor::neg();
    pcn::Factor dont_care = pcn::Factor::one();
    pcn::Factor zero = pcn::Factor::zero();
};

// Cubes
TEST_F(SingularityDetection, CubeOfDontCaresIsTautology) {     
    EXPECT_TRUE(initialized_3.is_tautology());  // [11 11 11] is universally true
}

TEST_F(SingularityDetection, CubeWithZerosIsZero) {
    initialized_3.at(1) = zero;                 // set x1 = "00"
    EXPECT_TRUE(initialized_3.is_zero());       // [11 00 11] is universally false
}

TEST_F(SingularityDetection, CubeOfMixedIsNeitherZeroNorTautology) {
    auto cube = pcn::Cube{dont_care, pos, neg}; // [11 01 10] is F = x1 * x2'
    EXPECT_FALSE(cube.is_zero());
    EXPECT_FALSE(cube.is_tautology());
}

// CubeLists
TEST_F(SingularityDetection, EmptyCubeList_IsZero) {
    EXPECT_TRUE(pcn::CubeList(3).is_zero());
}

TEST_F(SingularityDetection, CubeList_withAllTautologyCubes_IsTautology) {
    auto cube_list = pcn::CubeList{{0b11, 0b11, 0b11}, {0b11, 0b11, 0b11}};     // F = 1 + 1
    EXPECT_TRUE(pcn::is_tautology(cube_list));
}

TEST_F(SingularityDetection, CubeList_withMultiCubeTautology_IsTautology) {
    auto cube_list = pcn::CubeList{{0b01}, {0b10}};                             // F = x + x' = 1.
    EXPECT_TRUE(pcn::is_tautology(cube_list));
}

TEST_F(SingularityDetection, CubeList_withMixedValues_IsNeitherZeroNorTautology) {
    auto cube_list = pcn::CubeList{{0b11, 0b01, 0b10}, {0b01, 0b11, 0b11}};     // F = (x1 * x2') + (x0)
    EXPECT_FALSE(cube_list.is_zero());
    EXPECT_FALSE(is_tautology(cube_list));
}
