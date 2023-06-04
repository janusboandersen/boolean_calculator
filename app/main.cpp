/**
 * @file main.cpp
 * @author Janus Bo Andersen
 * @brief Find complement of boolean functions in input files
 * @version 1.0
 * @date Q2 2023
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <config.hpp>
#include <urp/pcn.h>
#include <urp/meta.h>
#include <urp/algorithm.h>
#include <adaptor/file_adaptor.h>


int main(void) {
    std::cout << project_name << ": " << project_version << " (" << project_build_type << ")" << std::endl;

    std::string i_path = "data/UnateRecursiveComplement/in/part";
    std::string o_path = "data/UnateRecursiveComplement/out/part";
    std::string ext = ".pcn";

    adaptor::PcnInFileAdaptor sop_file;
    adaptor::PcnOutFileAdaptor compl_file;

    for (auto file_num = 1; file_num <=5; file_num++) {
        sop_file = adaptor::PcnInFileAdaptor{i_path + std::to_string(file_num) + ext};
        compl_file = adaptor::PcnOutFileAdaptor {o_path + std::to_string(file_num) + ext};
        compl_file.store(pcn::bool_not(sop_file.load()));
    }

    return 0;
}
