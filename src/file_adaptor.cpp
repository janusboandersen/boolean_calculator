/**
 * @file file_adaptor.cpp
 * @author Janus
 * @brief Implementation of classes in adaptor/file_adaptor.h.
 * 
 * @version 1.0.0
 * @date 2023-06-01
 * 
 * @copyright Copyright (c) 2023
 * 
 */


#include <adaptor/file_adaptor.h>


namespace adaptor {

PcnInFileAdaptor::PcnInFileAdaptor() 
{
    filename = "NoFile";
}

PcnInFileAdaptor::PcnInFileAdaptor(std::string filename) : filename{filename} 
{
    this->file = std::ifstream {filename};
    if (this->file.good()) {
        this->file >> m_num_variables;
        this->file >> m_num_cubes;
        this->sop = std::make_unique<pcn::CubeList>(m_num_variables);
    }
    else {
        throw std::runtime_error("File not found: " + filename);
    }
}

std::string PcnInFileAdaptor::str()
{
    std::stringstream s;
    s << "File " << filename 
        << " contains M = " << m_num_cubes 
        << " cubes in N = " << m_num_variables << " variables." ;
    return s.str();
}

pcn::CubeList PcnInFileAdaptor::load()
{
    while (read_cube()) { }
    return *sop;
}

bool PcnInFileAdaptor::read_cube() 
{
    int enumerated_vars_in_cube;
    int var_encoding;
    auto product = pcn::Cube(this->m_num_variables);
    auto x = pcn::BooleanVariable {0, pcn::Factor::one()};

    if (!this->file.good()) { return false; }
    this->file >> enumerated_vars_in_cube;
    if (file.eof()) { return false; }
        
    for (int i = 0; i < enumerated_vars_in_cube; i++) {
        file >> var_encoding;
        x.pol = (var_encoding > 0 ? pcn::Factor::pos() : pcn::Factor::neg());
        x.idx = std::abs(var_encoding) - 1;
        product = pcn::bool_and(x, product);
    }
    sop->push_back(std::move(product));
    return true;
}


PcnOutFileAdaptor::PcnOutFileAdaptor()
{
    filename = "NoFile";
}

PcnOutFileAdaptor::PcnOutFileAdaptor(std::string filename) : filename{filename}
{
    this->file = std::ofstream(filename, std::ios::out);
    if (!this->file.good()) {
        throw std::runtime_error("File not found: " + filename);
    }
}

std::string PcnOutFileAdaptor::str()
{
    std::stringstream s;
    s << "File " << filename;
    return s.str();
}

bool PcnOutFileAdaptor::store(const pcn::CubeList& cube_list)
{
    return write_header(cube_list) && write_cubes(cube_list);
}

bool PcnOutFileAdaptor::write_header (const pcn::CubeList& cube_list)
{
    if (!this->file.good()) { return false; }
    file << cube_list.N() << std::endl;
    file << cube_list.size() << std::endl;
    return true;
}

CountType PcnOutFileAdaptor::count_enumerated_vars(const pcn::Cube& cube) 
{
    return std::count_if(cube.cbegin(), cube.cend(), [](pcn::Factor factor){ return factor == pcn::Factor::pos() || factor == pcn::Factor::neg(); });
}

std::string PcnOutFileAdaptor::var_repr(pcn::BooleanVariable bv) 
{
    std::stringstream s;
    if (bv.pol == pcn::Factor::pos())       {s << " " << bv.idx;}   // Positive form
    else if (bv.pol == pcn::Factor::neg())  {s << " -" << bv.idx;}  // Negated/complement form
    return s.str();
}

std::string PcnOutFileAdaptor::cube_repr(const pcn::Cube& cube) 
{
    std::stringstream s;
    IndexType i = 1;                    // Offset variable index, as variable numbering starts at x1 in PCN file format
    s << count_enumerated_vars(cube);   // State how mnay variables are mentioned in this line
    for (const auto& factor : cube) {
        s << var_repr({i++, factor});
    }
    return s.str();
}

bool PcnOutFileAdaptor::write_cubes(const pcn::CubeList& cube_list) 
{
    if (!this->file.good()) { return false; }
    for (const auto& cube : cube_list) {
        file << cube_repr(cube) << std::endl;
    }
    return true;
}

} // namespace adaptor
