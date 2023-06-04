/**
 * @file algorithm.cpp
 * @author Janus
 * @brief Implementation of algorithms for boolean operations, COFACTORS, AND, OR, NOT, TAUTOLOGY.
 * 
 * @version 1.0.0
 * @date 2023-06-01
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <list>
#include <vector>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <ostream>
//#include <functional>
//#include <initializer_list>
//#include <iostream>

#include <urp/algorithm.h>

namespace pcn {

/***************************
 *   COFACTOR ALGORITHMS   *
 **************************/

/**
 * @brief The positive cofactor of a single Factor. 
 */
Factor positive_cofactor(const Factor& f) {
    // 00 -> 00 (Illegal - shouldn't occur)
    // 10 -> 00 ( x' | x=1 -> 00)
    // 01 -> 11 ( x  | x=1 -> 11)
    // 11 -> 11 ( -  | x=1 -> 11)
    return (f.low_bit() == 0 ? Factor{0b00} : Factor{0b11});
}

/**
 * @brief The negative cofactor of a single Factor. 
 */
Factor negative_cofactor(const Factor& f) {
    // 00 -> 00 (Illegal - shouldn't occur though)
    // 01 -> 00 ( x  | x=0 -> 00)
    // 10 -> 11 ( x' | x=0 -> 11)
    // 11 -> 11 ( -  | x=0 -> 11)
    return (f.high_bit() == 0 ? Factor{0b00} : Factor{0b11});
}

/**
 * @brief The positive cofactor for variable x_i (x_i=1) for the Cube F (::= Product Term). 
 * Note: Caller must check if returned_cube.is_zero().
 */
Cube positive_cofactor(const Cube& c, int idx) {
    auto cofactor = Cube(c);                              // copy entire cube list
    cofactor.at(idx) = positive_cofactor(c.at(idx));      // change only var. xi by cofactoring
    return cofactor;                                      // returned cube could be zero, caller should check!
}

/**
 * @brief The negative cofactor for variable x_i (x_i=0) for the Cube F (::= Product Term). 
 * Note: Caller must check if returned_cube.is_zero().
 */
Cube negative_cofactor(const Cube& c, int idx) {
    auto cofactor = Cube(c);
    cofactor.at(idx) = negative_cofactor(c.at(idx));
    return cofactor;
}

/**
 * @brief The positive cofactor F_xi for the variable x_i (x_i=1) on the CubeList F (::= SOP).
 * Cofactor of a Sum of Product Terms is the Sum of the cofactors of each Product Term, i.e.
 * Cofactor of a CubeList is the CubeList of the cofactors of each Cube.
 * If a returned Cube is zero, it should be excluded from the list.
 */
CubeList positive_cofactor(const CubeList& cube_list, int idx) {
    CubeList cofactor(cube_list.N());
    for (const auto& cube : cube_list) {
        cofactor.push_back_nonzero(positive_cofactor(cube, idx));
    }
    return cofactor;
}

/**
 * @brief The negative cofactor F_not_xi for the variable x_i (x_i=0) on the CubeList F (::= SOP).
 */
CubeList negative_cofactor(const CubeList& cube_list, int idx) {
    CubeList cofactor(cube_list.N());
    for (const auto& cube : cube_list) {
        cofactor.push_back_nonzero(negative_cofactor(cube, idx));
    }
    return cofactor;
}


/*****************
 *   TAUTOLOGY   *
 ****************/

/**
 * @brief Returns true if the SOP is a tautology (F==1, universally true).
 * Uses URP, F==1 iff. cofactors F_x==1 && F_x'==1.
 * Uses recursion to determine if a CubeList is a tautology, by iteratively whittling down problem size via cofactoring.
 */
bool is_tautology(const CubeList& cube_list) {

    // Base case: SOP = 1. CubeList is a tautology by inspection at face value if F = ... + 1 + .... == 1.
    if (std::any_of(cube_list.cbegin(), cube_list.cend(), [](const auto& cube){return cube.is_tautology();})) {
        return true;
    }

    // Base case: SOP = 0. CubeList is a falsity (not a tautology). CubeList is zero if there are no product terms, or all terms are zero.
    if (cube_list.is_zero()) {
        return false;
    }

    // Base case: One-term SOP, and SOP is neither 1 nor 0.
    if (cube_list.size() == 1) {
        return false;
    }

    // Recursive case: Check cofactors for splitting variable. F == 1 iff F_x == 1 && F_x' == 1.
    auto split_on_idx = MetaFunction(cube_list).choose_recursion_variable();
    auto Fx  = positive_cofactor(cube_list, split_on_idx);
    auto Fx_ = negative_cofactor(cube_list, split_on_idx);
    return (is_tautology(Fx) && is_tautology(Fx_));
}


/***************************
 *     BOOLEAN ALGEBRA     *
 **************************/

/**
 * @brief And'ing variable with its own current factor representation in Cube
 * Bool Expr.      PCN         Res.
 * ------------    -------     ----
 * x  * 1  -> x    01 & 11  ->  01
 * x  * x  -> x    01 & 01  ->  01
 * x  * x' -> 0    01 & 10  ->  00
 * x  * 0  -> 0    01 & 00  ->  00
 * 
 * x' * 1  -> x'   10 & 11  ->  10
 * x' * x  -> 0    10 & 01  ->  00
 * x' * x' -> x'   10 & 10  ->  10
 * x' * 0  -> 0    10 & 00  ->  00
 */
Factor bool_and(const Factor first, const Factor second) {
    auto b1 = first.high_bit() & second.high_bit();
    auto b0 = first.low_bit() & second.low_bit();
    return Factor{b1, b0};
}

/**
 * @brief And'ing a BooleanVariable with a Cube
 * E.g. x0 * (x1 * x2)
 */
Cube bool_and(const BooleanVariable var, const Cube& cube) {
    auto product = Cube(cube);                                  // copy cube
    product.at(var.idx) = bool_and(var.pol, cube.at(var.idx));  // And'ing only affects the i'th variable representation
    return product;
}

/**
 * @brief And'ing a BooleanVariable with a CubeList
 * E.g. x0 * (x1 + x2)
 */
CubeList bool_and(const BooleanVariable var, const CubeList& cube_list) {
    auto product = CubeList(cube_list.N());
    for (const auto& cube : cube_list) {
        product.push_back_nonzero(bool_and(var, cube));
    }
    return product;
}


/**
 * @brief Or'ing variable with its own current factor representation in Cube
 * Bool Expr.      PCN         Res.
 * ------------    -------     ----
 * x  + 1  -> 1    01 | 11  ->  11
 * x  + x' -> 1    01 | 10  ->  11
 * x  + 0  -> x    01 | 00  ->  01
 * x  + x  -> x    01 | 01  ->  01
 * 
 * x' + 1  -> 1    10 | 11  ->  11
 * x' + x  -> 1    10 | 01  ->  11
 * x' + 0  -> x'   10 | 00  ->  10
 * x' + x' -> x'   10 | 10  ->  10
 
    */
Factor bool_or(const Factor first, const Factor second) {
    auto b1 = first.high_bit() | second.high_bit();
    auto b0 = first.low_bit() | second.low_bit();
    return Factor{b1, b0};
}

/**
 * @brief Or'ing a CubeList with another creates an new SOP (CubeList), which is the union of the two lists.
 * Notes: Returned value is (1) not necesarily minimal form, (2) possibly a tautology.
 * 
 * E.g. (x0 * x1) + (x1 * x2) -> <[01 01 11], [11 01 01]>
 * 
 * Note: something like x0 + x0' is a tautology, but user must manually check if returned_cube_list.is_tautology().
 */
CubeList bool_or(const CubeList& cube_list1, const CubeList& cube_list2) {
    
    // Don't duplicate if OR'ing same object
    if (&cube_list1 == &cube_list2) {
        return CubeList(cube_list1);
    }

    auto sum = CubeList(cube_list1);            // copy list1 as base result
    for (auto cube2 : cube_list2) {
        if (!cube_list1.contains(cube2)) {
            sum.push_back(std::move(cube2));    // move the new cube copy, if it isn't already in list1
        }
    }
    return sum;
}

/**
 * @brief Not'ing a variable from its current factor representation in a Cube.
 * Bool Expr.     PCN        Res.
 * ------------   -------    ----
 * (x)'  -> x'    not(01) -> 10
 * (x')' -> x     not(10) -> 01
 * (0)'  -> 1     not(00) -> 11
 * (1)'  -> 0     not(11) -> 00
 */
Factor bool_not(const Factor factor) {
    auto b1 = ~factor.high_bit();
    auto b0 = ~factor.low_bit();
    return Factor{b1, b0};
}

/**
 * Complement of a product term -> gives a sum of complemented variables.
 * By DeMorgan's Laws: (x * y)' = x' + y', and by extension for many variables, if y = z * w, then (x * z * w)' = x' + (x * z)' = x' + z' + w'. Etc.
 * So, in general: Product{ x_i }' = Sum{ x_i' }.
 */
CubeList bool_not(const Cube& cube) {
    auto N = cube.size();                   // N is dimension of problem, x0 .. xi .. xN-1
    auto sum = CubeList(N);                 // Must assume that dimensionality can be inferred from the given cube 
    BooleanVariable xi {0, Factor::one()};

    // Factor by factor complement, each complemented factor becomes its own product term in the sum.
    for (auto xi_pol : cube) {
        xi.pol = bool_not(xi_pol);                          // let xi = (xi)'.
        sum.push_back_nonzero( bool_and(xi, Cube(N)) );     // Store product term with complemented variable as only factor.
        xi.idx++;                                           // Advance to next variable: i = {0, 1, ..., N-1}.
    }

    return sum;
}

/**
 * @brief: Complement of a full SOP.
 * Uses recursive complement expansion. F'{x} = x * (Fx)'  +  x' * (Fx')'.
 * The cofactors of F can then be used to build up F' recursively.
 * Choice of x to split on, will make a difference to final non-min. SOP form.
 * Easy-to-compute termination rules form the recursion base cases.
 */
CubeList bool_not(const CubeList& cube_list) {

    // Base case: SOP == 0 (falsity) -> SOP' = 1. 
    // I.e., F = 0 -> F' = 1.
    if (cube_list.size() == 0) {
        return CubeList{ Cube(cube_list.N()) };
    }

    // TODO: Replace this with the simple tautology check
    // Base case: SOP == 1 (tautology) -> SOP' = 0. 
    // I.e., F = ... + 1 + ... = 1 -> F' = 0
    if (is_tautology(cube_list)) {
        return CubeList(cube_list.N());     // retain dimensionality
    }

    // Base case: SOP has one product term, neither 1 nor 0, easy computation of complement of the single cube via DeMorgan's Law.
    if (cube_list.size() == 1) {
        return bool_not(cube_list.front());
    }

    // Recursive case: SOP is more complex. Compute complement of cofactors in expansion
    auto split_on_idx = MetaFunction(cube_list).choose_recursion_variable();
    auto Fx  = positive_cofactor(cube_list, split_on_idx);
    auto Fx_ = negative_cofactor(cube_list, split_on_idx);
    auto not_Fx = bool_not(Fx);     // (Fx)'  complement of positive cofactor
    auto not_Fx_ = bool_not(Fx_);   // (Fx')' complement of negative cofactor
    
    // Recombine to get F'{x} = x * (Fx)'  +  x' * (Fx')'
    BooleanVariable x  {split_on_idx, Factor::pos()};
    BooleanVariable x_ {split_on_idx, Factor::neg()};
    
    auto lhs = bool_and(x, not_Fx);
    auto rhs = bool_and(x_, not_Fx_);
    return bool_or(lhs, rhs);

}

} // namespace pcn