#ifndef ARTCHITECT_CODE_GENERATOR_HPP
#define ARTCHITECT_CODE_GENERATOR_HPP

#include <string>
#include <vector>

#include "Class.hpp"

std::string generateClass(const std::string& name) {
    Class generated(name);
    return generated.getCode();
}


#endif //ARTCHITECT_CODE_GENERATOR_HPP
