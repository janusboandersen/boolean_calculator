/**
 * @file urp/algorithm.h
 * @author Janus
 * @brief Algorithms for boolean operations, COFACTORS, AND, OR, NOT, TAUTOLOGY.
 * 
 * @version 1.0.0
 * @date 2023-06-01
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once

#include <urp/pcn.h>
#include <urp/meta.h>

namespace pcn {

    /***************************
     *   COFACTOR ALGORITHMS   *
     **************************/
    Factor positive_cofactor(const Factor& f);
    Factor negative_cofactor(const Factor& f);

    Cube positive_cofactor(const Cube& c, int idx);
    Cube negative_cofactor(const Cube& c, int idx);
    
    CubeList positive_cofactor(const CubeList& cube_list, int idx);
    CubeList negative_cofactor(const CubeList& cube_list, int idx);


    /*****************
     *   TAUTOLOGY   *
     ****************/
    bool is_tautology(const CubeList& cube_list);


    /***************************
     *     BOOLEAN ALGEBRA     *
     **************************/
    Factor bool_and(const Factor first, const Factor second);
    Cube bool_and(const BooleanVariable var, const Cube& cube);
    CubeList bool_and(const BooleanVariable var, const CubeList& cube_list);

    Factor bool_or(const Factor first, const Factor second);
    CubeList bool_or(const CubeList& cube_list1, const CubeList& cube_list2);

    Factor bool_not(const Factor factor);
    CubeList bool_not(const Cube& cube);
    CubeList bool_not(const CubeList& cube_list);

} // namespace pcn