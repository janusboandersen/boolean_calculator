/**
 * @file pcn.cpp
 * @author Janus
 * @brief Implementation of data structures for representing boolean functions in Positional Cube Notation (PCN).
 * 
 * @version 1.0.0
 * @date 2023-05-25
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include <algorithm>
#include <sstream>
#include <urp/pcn.h>

namespace pcn {

    /***************************
     *  FACTOR DATA STRUCTURE  *
     **************************/

    /**
     * @brief Construct factor by specifying each bit of the 2-bit representation.
     */
    Factor::Factor(int b1, int b0) 
        : b1{static_cast<unsigned char>(b1)}, 
          b0{static_cast<unsigned char>(b0)}
    {
    }

    /**
     * @brief Construct a factor using a 2-bit literal, like Factor{0b01}.
     */
    Factor::Factor(int b1b0) 
        : b1{static_cast<unsigned char>((b1b0>>1) & 1)},
          b0{static_cast<unsigned char>(b1b0 & 1)} 
    {

    }

    /**
     * @brief Assign literal value to a Factor, like f = 0b01.
     */
    Factor& Factor::operator=(int b1b0) {
        this->b1 = static_cast<unsigned char>((b1b0>>1) & 1);
        this->b0 = static_cast<unsigned char>(b1b0 & 1);
        return *this;
    }

    /**
     * @brief Value (int) representation of factor.
     */
    int Factor::val() const {
        int bit_value = this->high_bit() * 2 + this->low_bit();     // Decimal value: b1 * 2^1 + b0
        return bit_value;
    }

    /**
     * @brief Textual (string) representation of factor.
     */
    std::string Factor::str() const {
        std::stringstream s;
        s << this->high_bit() << this->low_bit();
        return s.str();
    }

    /**
     * @brief Low bit (b0) in a factor represented like b1b0, returned as int.
     */
    int Factor::low_bit() const {
        return static_cast<int>(this->b0);
    }
    
    /**
     * @brief High bit (b1) in a factor represented like b1b0, returned as int.
     */
    int Factor::high_bit() const {
        return static_cast<int>(this->b1);
    }

    /**
     * @brief Equality comparison of Factor to a literal value (int).
     */
    bool operator==(const Factor& lhs, const int rhs) {
        return (lhs.val() == rhs);
    }

    /**
     * @brief Equality comparison of Factor to another Factor.
     */
    bool operator==(const Factor& lhs, const Factor& rhs) {
        return (lhs.val() == rhs.val());
    }

    /**
     * @brief Inequality comparison of Factor to another Factor.
     */
    bool operator!=(const Factor& lhs, const Factor& rhs) {
        return !(lhs == rhs);
    }

    /**
     * @brief Makes a Factor printable (with insertion operator).
     */
    std::ostream& operator<<(std::ostream& os, const Factor& f) {
        return os << f.str();
    }


    /*******************************
     *  BOOLEAN VAR DATA STRUCTURE *
     ******************************/
    
    /**
     * Text representation of a boolean variable as a verbose expression, like x1 = 01.
     */
    std::string BooleanVariable::verbose_str() const {
        std::stringstream s;
        s << "x" << idx << " = " << this->pol.str();
        return s.str();
    }

    /**
     * @brief Text representation of boolean variable as part of an inline formula, like x1 + x2'.
     */
    std::string BooleanVariable::inline_str() const {
        std::stringstream s;

        if (this->pol == Factor::zero()) {
            s << "0";
        } else if (this->pol == Factor::one()) {
            s << "1";
        } else {
            s << "x" << idx << (this->pol == Factor::neg() ? "'" : "");
        }
        return s.str();
    }

    /**
     * @brief Insertion operator for a Boolean variable.
     */
    std::ostream& operator<<(std::ostream& os, const BooleanVariable& b) {
        return os << b.inline_str();
    }


    /***************************
     *   CUBE DATA STRUCTURE   *
     **************************/

    /**
     * @brief Construct empty cube.
     */
    Cube::Cube()
    {
    }

    /**
     * @brief Initalize as Cube of N times "don't care". E.g. Cube(3).
     */
    Cube::Cube(int n_variables)
    {
        m_cube = std::vector<Factor>(n_variables, Factor(0b11));
    }

    /**
     * @brief Specify the Factors in the Cube explicitly via braced initalizer list of Factors. E.g. Cube{Factor(0b11), Factor(0b01), Factor(0b10)}.
     */
    Cube::Cube(std::initializer_list<Factor> factor_list)
    {
        m_cube = std::vector<Factor>(factor_list);
    }

    /**
     * @brief Specify the Factors in the Cube implicity as a braced initalizer list of ints. E.g. Cube{0b11, 0b01, 0b10}.
     */
    Cube::Cube(std::initializer_list<int> literal_list)
    {
        for (auto literal : literal_list) {
            m_cube.push_back(Factor{literal});
        }
    }

    /**
     * @brief Copy-construction
     */
    Cube::Cube(const Cube& other) : m_cube{ other.m_cube }
    {

    }

    /**
     * @brief Copy-assignment.
     */
    Cube& Cube::operator=(const Cube& other)
    {
        if (this != &other) {
            this->m_cube.clear();
            std::copy(other.m_cube.cbegin(), other.m_cube.cend(), std::back_inserter(this->m_cube));
        }
        
        return *this;
    }

    /**
     * @brief Dereferencing a Cube, with bounds-checking.
     */ 
    Factor& Cube::at(int pos) 
    {
        return m_cube.at(pos);
    }

    /**
     * @brief Dereferencing a const Cube, with bounds-checking.
     */
    const Factor& Cube::at(int pos) const
    {
        return m_cube.at(pos);
    }

    /**
     * @brief Non-const interator.
     */
    std::vector<Factor>::iterator Cube::begin()
    {
        return m_cube.begin();
    };

    /**
     * @brief Non-const interator.
     */
    std::vector<Factor>::iterator Cube::end()
    {
        return m_cube.end();
    };

    /**
     * @brief Const interator.
     */
    std::vector<Factor>::const_iterator Cube::begin() const
    {
        return m_cube.cbegin();
    };
    
    /**
     * @brief Const interator.
     */
    std::vector<Factor>::const_iterator Cube::end() const
    {
        return m_cube.cend();
    };
    
    /**
     * @brief Const interator.
     */
    std::vector<Factor>::const_iterator Cube::cbegin() const
    {
        return m_cube.cbegin();
    };
    
    /**
     * @brief Const interator.
     */
    std::vector<Factor>::const_iterator Cube::cend() const
    {
        return m_cube.cend();
    };

    /**
     * @brief Insert a Factor at the back of the Cube.
     */
    void Cube::push_back(Factor factor) 
    { 
        m_cube.push_back(factor); 
    }

    /**
     * @brief Number of variables in the cube (all states: 00, 01, 10, 11).
    */
    std::size_t Cube::size() const 
    {
        return m_cube.size();
    }

    /**
     * @brief Textual (string) representation of a Cube.
     */
    std::string Cube::str() const 
    {
        std::stringstream s;
        s << "[ ";
        for(const auto f : this->m_cube) {
            s << f << " ";
        }
        s << "]";
        return s.str();
    }

    /**
     * @brief String representation for inline printing, like x3'.
     */
    std::string Cube::inline_str() const 
    {
        std::stringstream s;
        std::uint32_t i = 0;
        for(const auto factor : this->m_cube) {
            s << (BooleanVariable{i++, factor}).inline_str() << "*";
        }
        auto str = s.str(); str.pop_back(); // remove last multiplication char
        return str;
    }

    /**
     * @brief If any of the factors in a Cube are 0, the whole product is 0 (universally false).
     * 
     */
    bool Cube::is_zero() const 
    {
        return (std::any_of(m_cube.begin(), m_cube.end(), [](auto x){return x==0b00;}));
    }

    /**
     * @brief If all of the factors in a Cube are "don't care"s, then the whole product is 1 (universally true).
     * TODO: MOVE THIS TO ALGORITHM
     */
    bool Cube::is_tautology() const 
    {
        return (std::all_of(m_cube.begin(), m_cube.end(), [](auto x){return x==0b11;}));
    }

    /**
     * @brief Makes a Cube printable (with insertion operator).
     */
    std::ostream& operator<<(std::ostream& os, const Cube& c) 
    {
        return os << c.str();
    }

    /**
     * @brief Equality comparison of one Cube to another Cube.
     */
    bool operator==(const Cube& lhs, const Cube& rhs) 
    {
        if ((lhs.size() == 0) && (rhs.size() == 0)) { return true; }
        if (lhs.size() != rhs.size()) { return false; }

        // Cubes are same non-zero size. 
        // Cubes are always ordered, x_0, ..., x_N-1, so we will iterate and compare lhs_0 != rhs_0, ..., lhs_N-1 != rhs_N-1.
        auto it_lhs = lhs.cbegin();
        auto it_rhs = rhs.cbegin();

        while (it_lhs != lhs.cend()) {
            if (*(it_lhs++) != *(it_rhs++)) { return false; }  // compare and advance
        }

        return true; // they must all be equal as we got this far...
    }

    /**
     * @brief Inquality comparison of one Cube to another Cube.
     */
    bool operator!=(const Cube& lhs, const Cube& rhs) 
    {
        return !(lhs == rhs);
    }


    /****************************
     * CUBE-LIST DATA STRUCTURE *
     ***************************/

    /**
     * @brief Construct an empty CubeList, must specify dimension of variables, e.g. dimension=2 -> x0, x1.
     */
    CubeList::CubeList(IndexType dimension) : dim{dimension}
    {
    };

    /**
     * @brief Construct a CubeList by specifying contained Cubes.
     * CubeList{Cube{0b11, 0b01, 0b10}, Cube{0b01, 0b10, 0b01}} <->.
     * CubeList{Cube{ one,  pos,  neg}, Cube{ pos,  neg,  neg}} <->
     * F = (x1 * x2') + (x0 * x1' * x2)
     */
    CubeList::CubeList(std::initializer_list<Cube> cube_init_list) 
        : dim{cube_init_list.begin()->size()} // asking for Cube.size()
    {
        for (auto cube : cube_init_list) {
            this->m_list.push_back(cube);
        }
    }

    /**
     * @brief Construct a CubeList by a nested list of literals.
     * CubeList{{0b11, 0b01, 0b10}, {0b01, 0b10, 0b01}}
     */
    CubeList::CubeList(std::initializer_list<std::initializer_list<int>> nested_literal_init_list) 
        : dim{nested_literal_init_list.begin()->size()}     // asking for std::initializer_list<int>.size()
    {
        for (auto literal_init_list : nested_literal_init_list) {
            m_list.push_back(Cube{literal_init_list});
        }
    }

    /**
     * @brief Construct a CubeList by a nested list of Factors.
     * CubeList{{one, pos, neg}, {pos, neg, neg}}
     */
    CubeList::CubeList(std::initializer_list<std::initializer_list<Factor>> nested_factor_init_list)
        : dim{nested_factor_init_list.begin()->size()}     // asking for std::initializer_list<Factor>.size()
    {
        for (auto factor_init_list : nested_factor_init_list) {
            m_list.push_back(Cube{factor_init_list});
        }
    }

    /**
     * @brief Push a Cube in to the back of the CubeList.
     */
    void CubeList::push_back(Cube&& cube) 
    {
        this->m_list.push_back(cube);
    };

    /**
     * @brief Push non-zero Cubes in to the back of the CubeList. Zero-Cubes are ignored.
     * Will move rvalue references, and copy lvalue references.
     */
    void CubeList::push_back_nonzero(Cube&& cube) 
    {
        if (!cube.is_zero()) {
            this->m_list.push_back(cube);
        }
    };

    /**
     * @brief Get const access to first cube in the list.
     */
    const Cube& CubeList::front() const
    {
        return m_list.front();
    }

    /**
     * @brief Number terms in the SOP. I.e., number of Cubes in the CubeList.
    */
    CountType CubeList::size() const 
    {
        return m_list.size();
    }

    /**
     * @brief Dimensionality of SOP function, F(x_0 ... x_N-1).
     */
    CountType CubeList::N() const 
    {
        return this->dim;   // Inferred dimensionality from the first cube, set at creation
    }

    /**
     * @brief Textual (string) representation of the SOP.
     */
    std::string CubeList::str() const 
    {
        if (this->is_zero()) {return "< 0 >"; }

        std::stringstream s;
        s << "< ";
        for (const auto& cube : this->m_list) {
            s << cube << ", ";
        }
        auto str = s.str(); 
        str.pop_back(); str.pop_back(); // remove last comma char
        str.append(" >");
        return str;
    }

    /**
     * @brief If the SOP is empty _OR_ ALL of the Terms in the SOP are 0, the whole sum is 0 (universally false).
     */
    bool CubeList::is_zero() const 
    {
        bool size_result = (this->size() == 0);
        bool search_result = std::all_of(m_list.begin(), m_list.end(), [](const auto& cube){return cube.is_zero();});
        return (size_result || search_result);
    }

    /**
     * @brief Check is CubeList contains a specified cube.
     * Repeated O(N) linear search will become "expensive". Consider using sets instead, or storing in a hashmap.
     */
    bool CubeList::contains(const Cube& cube) const 
    {
        return (std::find(this->cbegin(), this->cend(), cube) != this->cend());
    }

    /**
     * @brief Non-const interator.
     */
    std::list<Cube>::iterator CubeList::begin()
    {
        return this->m_list.begin();
    }

    /**
     * @brief Non-const interator.
     */
    std::list<Cube>::iterator CubeList::end()
    {
        return this->m_list.end();
    }
    
    /**
     * @brief Const interator.
     */
    std::list<Cube>::const_iterator CubeList::begin() const
    {
        return this->m_list.cbegin();
    }

    /**
     * @brief Const interator.
     */
    std::list<Cube>::const_iterator CubeList::end() const
    {
        return this->m_list.cend();
    }
    
    /**
     * @brief Const interator.
     */
    std::list<Cube>::const_iterator CubeList::cbegin() const
    {
        return this->m_list.cbegin();
    }
    
    /**
     * @brief Const interator.
     */
    std::list<Cube>::const_iterator CubeList::cend() const {
        return this->m_list.cend();
    } 

    /**
     * @brief Makes a CubeList printable (with insertion operator).
     */
    std::ostream& operator<<(std::ostream& os, const CubeList& cl) 
    {
        return os << cl.str();
    }

} // namespace pcn