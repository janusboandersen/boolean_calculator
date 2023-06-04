/**
 * @file meta.cpp
 * @author Janus
 * @brief Implementation of classes in urp/meta.h.
 * 
 * @version 1.0.0
 * @date 2023-06-01
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <urp/meta.h>

namespace pcn {

/********************************
 * METAVARIABLE IMPLEMENTATIONS *
 *******************************/

/**
 * @brief Make metadata about x_i in CubeList (SOP) and x_i's index (x_0, x_1, etc.).
 */
MetaVariable::MetaVariable(const CubeList& cube_list, IndexType idx) 
{
    this->m_idx = idx;                          // Boolean variable x_i
    auto N = (cube_list.cbegin())->size();      // N variables in each cube

    // No variables, or requested index out of bounds - make empty
    if ( (N == 0) || idx >= N) { return; }

    // Traverse terms and extract i'th factor, which represents x_i's polarity in that term.
    for (const auto& cube : cube_list) {
        this->push_back(cube.at(idx));
    }
}

/**
 * @brief Function is unate in variable x_i iff. x_i only appears in positive EXOR negative polarity.
 * Note: The complement of unate is not binate (due to the existence of don't care variables).
 */
bool MetaVariable::is_unate_in_fn() const
{
    return (this->has_pos() ^ this->has_neg());
}

/**
 * @brief Function is binate in variable x_i iff. x_i appears in BOTH positive AND negative polarity.
 * Note: The complement of biate is not unnate (due to the existence of don't care variables).
 */
bool MetaVariable::is_binate_in_fn() const 
{
    return (this->has_pos() & this->has_neg());
}

/**
 * @brief True if x_i appears in given polarity, otherwise false.
 */
bool MetaVariable::has_polarity(Factor pol) const
{
    return (std::any_of(this->cbegin(), this->cend(), [&](auto x){return x==pol;}));
}

/**
 * @brief True if x_i appears in positive polarity, otherwise false.
 */
bool MetaVariable::has_pos() const
{
    return this->has_polarity(Factor::pos());
}

/**
 * @brief True if x_i appears in negative polarity, otherwise false.
 */
bool MetaVariable::has_neg() const
{
    return this->has_polarity(Factor::neg());
}

/**
 * @brief Count of the number of times that variable x_i appears in given polarity.
 */
CountType MetaVariable::count_polarity(Factor pol) const
{
    return (std::count_if(this->cbegin(), this->cend(), [&](auto x){return x==pol;}));
}

/**
 * @brief Count of the number of times that variable x_i appears in positive polarity.
 */
CountType MetaVariable::count_pos() const
{
    return this->count_polarity(Factor::pos());
}

/**
 * @brief Count of the number of times that variable x_i appears in negative polarity.
 */
CountType MetaVariable::count_neg() const
{
    return this->count_polarity(Factor::neg());
}

CountType MetaVariable::count_terms() const
{
    return (this->count_pos() + this->count_neg());
}

/**
 * @brief Balance of the cofactor splitting tree (right-left balance) is measured by |T-C|.
 * Where T and C are the counts of the variable appearing in True and Complement form, respectively.
 */
int MetaVariable::abs_t_minus_c() const
{
    return abs(static_cast<int>(this->count_pos()) - static_cast<int>(this->count_neg()));
}

/**
 * @brief Informative text about variable x_i.
 */
std::string MetaVariable::str() const
{
    std::stringstream s;
    s << "x" << this->m_idx << ": is " 
        << (this->is_unate_in_fn() ? "unate" : (this->is_binate_in_fn() ? "binate" : "don't care")) 
        << " in function and appears in " << (count_pos() + count_neg()) << " term(s). " 
        << "As pos: " << count_pos() << ". As neg: " << count_neg() << ".";
    s << " |T-C|=" << abs_t_minus_c() << ".";
    return s.str();
}

/**
 * @brief Get the variable index (subscript) for the variable.
 */
IndexType MetaVariable::index() const
{
    return m_idx; 
}

/**
 * @brief Makes a MetaVariable printable (with insertion operator).
 */
std::ostream& operator<<(std::ostream& os, const MetaVariable& m) 
{
    return os << m.str();
}


/********************************
 * METAFUNCTION IMPLEMENTATIONS *
 *******************************/

/**
 * @brief Construct a MetaFunction (SOP+intelligence) from a CubeList.
 */
MetaFunction::MetaFunction(const CubeList& cube_list)
{
    this->m_N = cube_list.N();          // Inherit dimensionality
    this->m_str_repr = cube_list.str(); // TODO: Make Verbose string

    for (auto i = 0UL; i < this->m_N; i++) {
        m_vector.push_back( MetaVariable(cube_list, i) );   // Analyze the i'th variable
    }
}

/**
 * @brief Boolean function (SOP) is unate iff. it is unate in all variables.
 * Note: Alternatively, check that there are no binate variables via a filter.
 */
bool MetaFunction::is_unate() const 
{
    return std::all_of(m_vector.cbegin(), m_vector.cend(), [](auto meta_x){return meta_x.is_unate_in_fn();});
}

/**
 * @brief Find the maximum value of a given attribute for the variables in the collection, by comparing values obtained using the lambda attr_getter.
 * If collection is empty, zero is returned.
*/
CompareType MetaFunction::max_of(MetaVariableFilter& data, std::function<CompareType(const MetaVariable)> attr_getter) const 
{
    auto it = std::max_element(data.begin(), data.end(), [&](auto a, auto b){ return attr_getter(*a) < attr_getter(*b);});
    return (it != data.end() ? attr_getter(**it) : 0); // don't try to dereference the past-the-end ptr, just return zero.
}

/**
 * @brief Find the minimum value of a given attribute for the variables in the collection, by comparing values obtained using the lambda attr_getter.
 * If collection is empty, zero is returned.
*/
CompareType MetaFunction::min_of(MetaVariableFilter& data, std::function<CompareType(const MetaVariable)> attr_getter) const 
{
    auto it = std::min_element(data.begin(), data.end(), [&](auto a, auto b){ return attr_getter(*a) < attr_getter(*b);});
    return (it != data.end() ? attr_getter(**it) : 0); // don't try to dereference the past-the-end ptr, just return zero.
}

/**
 * @brief Create filter (pointers) with all variables.
 */
MetaVariableFilter MetaFunction::all() const 
{
    MetaVariableFilter ptrs;
    for (const auto& meta : this->m_vector) { ptrs.push_back(&meta); }
    return ptrs;
} 

/**
 * @brief Create filter (pointers) with all variables where predicate (functor) is true.
 */
MetaVariableFilter MetaFunction::filter(MetaVariableFilter& data, std::function<bool(const MetaVariable&)> functor) const 
{
    MetaVariableFilter ptrs;
    for (const auto& meta_x_ptr : data) {
        if (functor(*meta_x_ptr)) { ptrs.push_back(meta_x_ptr); }
    }
    return ptrs;
} 

/**
 * @brief Select recursion variable based on rules 1-5.
 * Rule 1: Choose most binate variable (max # terms).
 * Rule 2: If tied, among these choose the most balanced variable (min |T-C|).
 * Rule 3: If tied, among these choose the lowest indexed variable (min idx).
 * Rule 4: If no binate variables, choose the most unate variable (max # terms).
 * Rule 5: If tied, among these choose the lowest indexed variable (min idx).
 */
IndexType MetaFunction::choose_recursion_variable() const 
{
    auto all_vars = all();
    auto binate_vars = filter(all_vars, [](auto x){ return x.is_binate_in_fn(); });
    auto get_index = [](MetaVariableFilter& one_item_container){ return one_item_container.front()->index(); };  // std::function<IndexType(MetaVariableFilter&)>

    if (binate_vars.size() > 0) // Choose among the binate variables
    {
        // Rule 1: If no tie, choose most binate
        auto most_binateness = max_of(binate_vars, [](auto x){ return x.count_terms(); });
        auto most_binate_vars = filter(binate_vars, [cond=most_binateness](auto x){ return x.count_terms() == cond; });
        if (most_binate_vars.size() == 1) {
            return get_index(most_binate_vars);
        }

        // Rule 2: If no tie, choose most balanced
        auto most_balancedness = min_of(most_binate_vars, [](auto x){ return x.abs_t_minus_c(); });
        auto most_balanced_most_binate_vars = filter(most_binate_vars, [cond=most_balancedness](auto x){ return (CountType) x.abs_t_minus_c() == cond; });
        if (most_balanced_most_binate_vars.size() == 1) {
            return get_index(most_balanced_most_binate_vars);
        }

        // Rule 3: Return lowest index
        auto lowest_index_binate = min_of(most_balanced_most_binate_vars, [](auto x){ return x.index(); });
        return lowest_index_binate;
    } 
    else // Choose among the unate variables
    {
        auto unate_vars = filter(all_vars, [](auto x){ return x.is_unate_in_fn(); });

        // Rule 4: If no tie, choose most unate
        auto most_unateness = max_of(unate_vars, [](auto x){ return x.count_terms(); });
        auto most_unate_vars = filter(unate_vars, [cond=most_unateness](auto x){ return x.count_terms() == cond; });
        if (most_unate_vars.size() == 1) {
            return get_index(most_unate_vars);
        }

        // Rule 5: Return lowest index
        auto lowest_index_unate = min_of(most_unate_vars, [](auto x){ return x.index(); });
        return lowest_index_unate;
    }
}

/**
 * @brief Very verbose analysis of the SOP function for debugging algorithms.
 */
std::string MetaFunction::str() const 
{
    auto all_vars = all();

    // Rules 1-3
    auto binate_vars = filter(all_vars, [](auto x){ return x.is_binate_in_fn(); });
    auto most_binateness = max_of(binate_vars, [](auto x){ return x.count_terms(); });
    auto most_binate_vars = filter(binate_vars, [cond=most_binateness](auto x){ return x.count_terms() == cond; });
    auto most_balancedness = min_of(most_binate_vars, [](auto x){ return x.abs_t_minus_c(); });
    auto most_balanced_most_binate_vars = filter(most_binate_vars, [cond=most_balancedness](auto x){ return (CountType) x.abs_t_minus_c() == cond; });
    auto lowest_index_binate = min_of(most_balanced_most_binate_vars, [](auto x){ return x.index(); });

    // Rules 4-5
    auto unate_vars = filter(all_vars, [](auto x){ return x.is_unate_in_fn(); });
    auto most_unateness = max_of(unate_vars, [](auto x){ return x.count_terms(); });
    auto most_unate_vars = filter(unate_vars, [cond=most_unateness](auto x){ return x.count_terms() == cond; });
    auto lowest_index_unate = min_of(most_unate_vars, [](auto x){ return x.index(); });

    std::stringstream s;

    s << "PCN: " << this->m_str_repr << "." << '\n';

    s << (this->is_unate() ? "Unate" : "Binate") << " function in " << this->m_N << " variable(s)." << '\n';

    s << '\n' 
        << "Rule-based choice of recursion variable: " << "x" << choose_recursion_variable() << "." << '\n';

    s << '\n' 
        << "RULE 1: Choose among BINATE variable(s). " << '\n'
        << "Candidate(s):";
    for (auto bvar : binate_vars) {
        s << " x" << bvar->index();
    }
    s << "." << '\n';

    s << "Of these, choose the one appearing in most terms." << '\n'
        << "Choose with max(#terms)=" 
        << most_binateness << "." << '\n'
        << "Count of possible candidate(s): " 
        << most_binate_vars.size() << "." << '\n' 
        << "Candidate(s):";
    for (auto bvar : most_binate_vars) {
        s << " x" << bvar->index();
    }
    s << "." << '\n';

    s << '\n' 
        << "RULE 2: If tied, choose among these the best balanced variable." << '\n'
        << "Choose with min|T-C|=" 
        << most_balancedness << "." << '\n'
        << "Count of possible candidate(s): "
        << most_balanced_most_binate_vars.size() << "." << '\n'
        << "Candidate(s):";
    for (auto bvar : most_balanced_most_binate_vars) {
        s << " x" << bvar->index();
    }
    s << "." << '\n';

    s << '\n' 
        << "RULE 3: If tied, choose among these the lowest-indexed binate variable: " 
        << "x" << lowest_index_binate << "." << '\n';

    s << '\n' 
        << "RULE 4: If no binate variables, choose among UNATE variable(s)." << '\n'
        << "Candidate(s):";
    for (auto uvar : unate_vars) {
        s << " x" << uvar->index();
    }
    s << "." << '\n';

    s << "Of these, choose the one appearing in most terms." << '\n'
        << "Choose with max(#terms)="
        << most_unateness << "." << '\n'
        << "Count of possible candidates: " 
        << most_unate_vars.size() << "." << '\n'
        << "Candidate(s):";
    for (auto uvar : most_unate_vars) {
        s << " x" << uvar->index();
    }
    s << "." << '\n';

    s << '\n' 
        << "RULE 5: If tied, choose among these the lowest-indexed unate variable: "
        << "x" << lowest_index_unate << "." << '\n';

    s << '\n' << '\n'
        << "Variable enumeration:" << '\n';
    for (const auto& meta_x : m_vector) {
        s << meta_x << '\n';
    }

    return s.str();
}

} // namespace pcn