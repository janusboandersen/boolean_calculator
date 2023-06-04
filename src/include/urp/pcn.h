/**
 * @file pcn.h
 * @author Janus
 * @brief Data structures for representing boolean functions in Positional Cube Notation (PCN). 
 * 
 * PCN is a sequential representation scheme. Example: 
 * - Given: F(x1, x2, x3) = x1*x2' + x3. (where <var>' is the unary negation, complement).
 * - F: boolean function in sum-of-products (SOP) form.
 * - xi: boolean variable of index i.
 * 
 * - 2-bit representation of each factor in a product term:
 *   - 01: Positive polarity, like x1 in (x1*x2').
 *   - 10: Negative polarity (complement), like x2' in (x1*x2').
 *   - 11: Don't care (not present), like x3 in (x1*x2').
 * - Cube ::= product term. Representation of a product term as an ordered/positional list of the factors in 2-bit representation.
 * - Cube-list ::= SOP, as a list (sum) of cubes (products). 
 *
 * - Putting it together: F in PCN => [01 10 11], [11 11 01].
 * 
 * @version 1.0.0
 * @date 2023-05-25
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once

#include <initializer_list>
#include <vector>
#include <list>
#include <ostream>

namespace pcn {

    using CountType = std::size_t;          // alias unsigned long
    using CompareType = long unsigned int;  //
    using IndexType = std::uint32_t;

    // Forward declare
    class Factor;
    class BooleanVariable;
    class Cube;
    class CubeList;

    // Free standing operators
    bool operator==(const Factor& lhs, const int rhs);
    bool operator==(const Factor& lhs, const Factor& rhs);
    bool operator!=(const Factor& lhs, const Factor& rhs);
    bool operator==(const Cube& lhs, const Cube& rhs);
    bool operator!=(const Cube& lhs, const Cube& rhs);
    std::ostream& operator<<(std::ostream& os, const Factor& f);
    std::ostream& operator<<(std::ostream& os, const BooleanVariable& b);
    std::ostream& operator<<(std::ostream& os, const Cube& c);
    std::ostream& operator<<(std::ostream& os, const CubeList& cl);


    /***************************
     *  FACTOR DATA STRUCTURE  *
     **************************/

    /**
     * @brief A Factor is the 2-bit representation of a boolean variable's form inside a Cube (Cube ::= Product Term).
     * 
     * b1 b0  Factor definition
     * -- --  -----------------------------------------------------------------------------------------------------------------
     * 0  0   Non-allowed. Intermediate symbol for typing. Variable will nullify product term, and its cube will be deleted (0).
     * 0  1   Variable enters product term with positive polarity (x).
     * 1  0   Variable enters product term with negative polarity (x').
     * 1  1   Variable not present in product term == Don't care (1).
     */
    class Factor {
    public:
        // Use like Factor{0b1, 0b0}
        explicit
        Factor(int b1, int b0) : b1{static_cast<unsigned char>(b1)}, 
                                 b0{static_cast<unsigned char>(b0)} {}

        // Use like Factor{0b10}
        explicit
        Factor(int b1b0) : b1{static_cast<unsigned char>((b1b0>>1) & 1)},
                           b0{static_cast<unsigned char>(b1b0 & 1)} {}

        Factor& operator=(int b1b0);

        // Convenience of notation
        static Factor pos()  { return Factor{0b01}; }       // positive polarity
        static Factor neg()  { return Factor{0b10}; }       // negative polarity (complement)
        static Factor one()  { return Factor{0b11}; }       // constant one
        static Factor zero() { return Factor{0b00}; }       // constant zero

        int val() const;
        std::string str() const;
        int low_bit() const;
        int high_bit() const;

    private:
        // 2 bits repr. in total
        unsigned char b1 : 1;    
        unsigned char b0 : 1;
    };


    /*******************************
     *  BOOLEAN VAR DATA STRUCTURE *
     ******************************/

    /**
     * @brief Represent a boolean variable by its index and polarity.
     * x0 -> BooleanVariable{0, Factor::pos()}
     * x1' -> BooleanVariable{1, Factor::neg()}
     */
    struct BooleanVariable {
        IndexType idx;
        Factor pol;

        std::string verbose_str() const;
        std::string inline_str() const;
    };


    /***************************
     *   CUBE DATA STRUCTURE   *
     **************************/

    /**
     * @brief A Cube (::= Product Term) is an indexable and fixed-length list of factors. I.e., an N-tuple of factors.
     * Index i in the Cube is the i'th Factor, representing boolean variable xi in 2-bit notation.
     * Invariant fixed-length: A function F with N variables -> all Cubes must have N Factors, each with the same relative positioning.
    */
    class Cube {
    public:

        // empty
        explicit Cube() {}

        // Copy-construction
        Cube(const Cube& other) : m_cube{ other.m_cube } {}

        // Copy-assignment
        Cube& operator=(const Cube& other) {
            if (this != &other) {
                this->m_cube.clear();
                std::copy(other.m_cube.cbegin(), other.m_cube.cend(), std::back_inserter(this->m_cube));
            }
            
            return *this;
        }

        // Initalize as Cube of N times "don't care". E.g. Cube(3).
        explicit
        Cube(int n_variables) {
            m_cube = std::vector<Factor>(n_variables, Factor(0b11));
        }

        // Specify the Factors in the Cube explicitly via braced initalizer list of Factors. E.g. Cube{Factor(0b11), Factor(0b01), Factor(0b10)}.
        explicit 
        Cube(std::initializer_list<Factor> factor_list) {
            m_cube = std::vector<Factor>(factor_list);
        }

        // Specify the Factors in the Cube implicity as a braced initalizer list of ints. E.g. Cube{0b11, 0b01, 0b10}.
        explicit
        Cube(std::initializer_list<int> literal_list) {
            for (auto literal : literal_list) {
                m_cube.push_back(Factor{literal});
            }
        }

        Factor& at(int pos);
        void push_back(Factor factor)                       { m_cube.push_back(factor); };
        const Factor& at(int pos) const;
        std::vector<Factor>::iterator begin()               { return m_cube.begin();  };
        std::vector<Factor>::iterator end()                 { return m_cube.end();    };
        std::vector<Factor>::const_iterator begin() const   { return m_cube.cbegin(); };
        std::vector<Factor>::const_iterator end() const     { return m_cube.cend();   };
        std::vector<Factor>::const_iterator cbegin() const  { return m_cube.cbegin(); };
        std::vector<Factor>::const_iterator cend() const    { return m_cube.cend();   };
        std::size_t size() const;
        std::string str() const;
        std::string inline_str() const;
        bool is_zero() const;
        bool is_tautology() const;      // TODO: Move to algorithm.h

    private:
        std::vector<Factor> m_cube;         // Choosing vector as we need random access.
    };



    /****************************
     * CUBE-LIST DATA STRUCTURE *
     ***************************/

    /**
     * @brief A CubeList (::= SOP) is a length-M list of Cubes (::= Product Terms), each Cube of length-N.
     * I.e., the same as the SOP-form function F in N variables, with M terms in the sum.
     * Zero cubes are removed from the list, new product terms can be added.
    */
    class CubeList {
    public:

        // Empty CubeList, must know dimension
        explicit 
        CubeList(IndexType dimension) : dim{dimension} {};

        // CubeList{Cube{0b11, 0b01, 0b10}, Cube{0b01, 0b10, 0b01}} <->.
        // CubeList{Cube{ one,  pos,  neg}, Cube{ pos,  neg,  neg}} <->
        // F = (x1 * x2') + (x0 * x1' * x2)
        explicit
        CubeList(std::initializer_list<Cube> cube_init_list) 
            : dim{cube_init_list.begin()->size()} // asking for Cube.size()
        {
            for (auto cube : cube_init_list) {
                this->m_list.push_back(cube);
            }
        }

        // CubeList{{0b11, 0b01, 0b10}, {0b01, 0b10, 0b01}}
        explicit
        CubeList(std::initializer_list<std::initializer_list<int>> nested_literal_init_list) 
            : dim{nested_literal_init_list.begin()->size()}     // asking for std::initializer_list<int>.size()
        {
            for (auto literal_init_list : nested_literal_init_list) {
                m_list.push_back(Cube{literal_init_list});
            }
        }

        // CubeList{{one, pos, neg}, {pos, neg, neg}}
        explicit
        CubeList(std::initializer_list<std::initializer_list<Factor>> nested_factor_init_list)
            : dim{nested_factor_init_list.begin()->size()}     // asking for std::initializer_list<Factor>.size()
        {
            for (auto factor_init_list : nested_factor_init_list) {
                m_list.push_back(Cube{factor_init_list});
            }
        }

        void push_back(Cube&& cube);
        void push_back_nonzero(Cube&& cube);
        const Cube& front() const                             { return m_list.front(); }
        CountType size() const;
        CountType N() const;
        std::string str() const;
        bool is_zero() const;
        bool contains(const Cube& cube) const;

        // Iterable
        using value_type = Cube;
        std::list<Cube>::iterator begin()                { return this->m_list.begin();  }
        std::list<Cube>::iterator end()                  { return this->m_list.end();    }
        std::list<Cube>::const_iterator begin() const    { return this->m_list.cbegin(); }
        std::list<Cube>::const_iterator end()   const    { return this->m_list.cend();   }
        std::list<Cube>::const_iterator cbegin() const   { return this->m_list.cbegin(); }
        std::list<Cube>::const_iterator cend() const     { return this->m_list.cend();   }        

    private:
        std::list<Cube> m_list;         // Choosing list as we need many insertions and removals without invalidating iterators.
        const CountType dim;            // Must retain dimension
    };

} // namespace pcn