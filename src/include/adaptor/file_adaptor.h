/**
 * @file file_adaptor.h
 * @author Janus
 * @brief Input file adaptor to read .pcn format and convert to CubeList, and vice versa for Output file adapter.
 * 
 * @version 1.0
 * @date 2023-06-01
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <memory>

#include <urp/pcn.h>
#include <urp/algorithm.h>

namespace adaptor {

using CountType = std::size_t;
using IndexType = std::uint32_t;

class PcnInFileAdaptor 
{
public:
    explicit PcnInFileAdaptor();
    explicit PcnInFileAdaptor(std::string filename);
    std::string str();
    pcn::CubeList load();

private:
    std::ifstream file;                     // RAII, open for lifetime of this object
    std::string filename;
    IndexType m_num_variables {0};
    CountType m_num_cubes {0};
    std::unique_ptr<pcn::CubeList> sop;     // Indirection, as we can't know dimensionality at time of opening file
    
    bool read_cube();
};


class PcnOutFileAdaptor {
public:
    explicit PcnOutFileAdaptor();
    explicit PcnOutFileAdaptor(std::string filename);
    std::string str();
    bool store(const pcn::CubeList& cube_list);

private:
    std::ofstream file;                     // RAII, open for lifetime of this object
    std::string filename;
    
    bool write_header (const pcn::CubeList& cube_list);
    CountType count_enumerated_vars(const pcn::Cube& cube);
    std::string var_repr(pcn::BooleanVariable bv);
    std::string cube_repr(const pcn::Cube& cube);
    bool write_cubes(const pcn::CubeList& cube_list);
};

} // namespace adaptor
