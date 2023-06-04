/**
 * @file urp/meta.h
 * @author Janus
 * @brief MetaVariables and MetaFunctions can analyse Boolean Variables and Boolean Functions, respectively.
 * 
 * @version 1.0.0
 * @date 2023-06-01
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once

#include <list>
#include <vector>
#include <cmath>
#include <algorithm>
#include <functional>
#include <sstream>
#include <ostream>
//#include <iostream>
//#include <initializer_list>

#include <urp/pcn.h>

namespace pcn {

// Forward declare
class Factor;
class Cube;
class CubeList;
Factor positive_cofactor(const Factor& f);
Factor negative_cofactor(const Factor& f);
Cube positive_cofactor(const Cube& c, int idx);
Cube negative_cofactor(const Cube& c, int idx);
CubeList positive_cofactor(const CubeList& cube_list, int idx);
CubeList negative_cofactor(const CubeList& cube_list, int idx);

class MetaVariable;
class MetaFunction;
using MetaVariableFilter = std::vector<const MetaVariable*>;
using CountType = std::size_t;          // alias unsigned long
using CompareType = long unsigned int;  //
using IndexType = std::uint32_t;

// forward declare
std::ostream& operator<<(std::ostream& os, const MetaVariable& m);


/**
 * @brief MetaVariable holds information about a boolean variable x_i, as it appears in a Sum-of-Products (CubeList).
 */
class MetaVariable : private Cube
{
public:
    MetaVariable(const CubeList& cube_list, IndexType idx);

    bool is_unate_in_fn() const;
    bool is_binate_in_fn() const;
    
    bool has_polarity(Factor pol) const;
    bool has_pos() const;
    bool has_neg() const;
    
    CountType count_polarity(Factor pol) const;
    CountType count_pos() const;
    CountType count_neg() const;
    CountType count_terms() const;
    int abs_t_minus_c() const;      // balance of recursion tree

    std::string str() const;
    IndexType index() const;

private:
    IndexType m_idx;

};


/**
 * @brief Metadata on a Boolean SOP.
 * Allows making decisions about recusion variables, and to determine unateness of an SOP.
 */
class MetaFunction
{
public:
    MetaFunction(const CubeList& cube_list);
    bool is_unate() const;
    std::string str() const;
    CountType max_of(MetaVariableFilter& data, std::function<CompareType(const MetaVariable)> attr_getter) const;
    CompareType min_of(MetaVariableFilter& data, std::function<CompareType(const MetaVariable)> attr_getter) const;
    IndexType choose_recursion_variable() const;

private:
    IndexType m_N;                                  // Dimensionality, x_0 ... x_N-1.
    std::vector<MetaVariable> m_vector;             // Holds analysis of each boolean variable, x_i.
    std::string m_str_repr;                         // Pretty string representation of function.

    // Keep these private to avoid dangling ptrs
    MetaVariableFilter all() const;
    MetaVariableFilter filter(MetaVariableFilter& data, std::function<bool(const MetaVariable&)> functor) const;
};

} // namespace pcn