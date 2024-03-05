#ifndef ARTCHITECT_ELEMENT_HPP
#define ARTCHITECT_ELEMENT_HPP

#include "join.hpp"
#include "split.hpp"

class Element {
public:
    Element(const std::string& name_, const std::string& element_type_) {
        element_type = element_type_;
        name = name_;
    }

    virtual void addSelectiveComponent(const std::string& component_name, const std::vector<std::string>& available_members) = 0;

    std::string getCode() {
        std::string code = pre_code + element_type + " " + name;
        if (!parents.empty()) {
            code += ": " + join(parents, ", ");
        }
        if (!friends.empty()) {
            code += " {\n    friend class " + join(friends, ";\n    friend class ") + ";\n";
        } else {
            code += " {";
        }
        code += "};";
        return code;
    }

    std::string getName() {
        return name;
    }
    void setName(const std::string& name_) {
        name = name_;
    }
protected:
    std::string pre_code;
    std::string element_type;
    std::string name;
    std::vector<std::string> friends;
    std::vector<std::string> parents;
};

#endif //ARTCHITECT_ELEMENT_HPP
