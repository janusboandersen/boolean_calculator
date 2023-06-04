/**
 * @file main_test.cpp
 * @author Janus Bo Andersen
 * @brief This executable runs the full suite of tests. Will output results to console.
 * @version 1.0.0
 * @date Aug 2022
 * 
 * @copyright Copyright (c) 2022
 * See also: http://google.github.io/googletest/ 
 */

#include <gtest/gtest.h>

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}