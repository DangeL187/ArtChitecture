#ifndef ARTCHITECT_CLASS_HPP
#define ARTCHITECT_CLASS_HPP

#include <variant>
#include "Element.hpp"

class Struct: public Element {
public:
    explicit Struct(const std::string& name): Element(name, "struct") {}

    void addSelectiveComponent(const std::string& component_name, const std::vector<std::string>& available_members) override {}

    void addParent(const std::string& parent_name, const std::string& modifier) {
        parents.push_back(modifier + " " + parent_name);
    }

    void addSelectiveParent(const std::string& parent_name, const std::vector<std::string>& available_members) {
        pre_code += "template <typename T>\n";
        pre_code += "struct _" + name + "_" + parent_name + ": private T {\n";
        for (const std::string& member: available_members) {
            if (member.find('(') != std::string::npos) {
                pre_code += "    auto _" + member + " { return T::" + member + "; }\n";
            } else {
                pre_code += "    auto& _" + member + "() { return T::" + member + "; }\n";
            }
        } pre_code += "};\n\n";
        parents.push_back("public _" + name + "_" + parent_name + "<" + parent_name + ">");
    }
};

class Class: public Element {
public:
    explicit Class(const std::string& name): Element(name, "class") {}

    void addSelectiveComponent(const std::string& component_name, const std::vector<std::string>& available_members) override {}

    void addParent(const std::string& parent_name, const std::string& modifier) {
        parents.push_back(modifier + " " + parent_name);
    }

    void addSelectiveParent(const std::string& parent_name, const std::vector<std::string>& available_members) {
        pre_code += "template <typename T>\n";
        pre_code += "struct _" + name + "_" + parent_name + ": private T {\n";
        for (const std::string& member: available_members) {
            if (member.find('(') != std::string::npos) {
                pre_code += "    auto _" + member + " { return T::" + member + "; }\n";
            } else {
                pre_code += "    auto& _" + member + "() { return T::" + member + "; }\n";
            }
        } pre_code += "};\n\n";
        parents.push_back("public _" + name + "_" + parent_name + "<" + parent_name + ">");
    }
};

class Component: public Class {
public:
    explicit Component(const std::string& name): Class(name) {
        pre_code = "template <typename T>\n";
        parents = {"public CRTP<T, Heart>"};
    }
    void addSelectiveComponent(const std::string& component_name, const std::vector<std::string>& available_members) override {}
};

class Module: public Class {
public:
    explicit Module(const std::string& name): Class(name) {}

    void addSelectiveComponent(const std::string& component_name, const std::vector<std::string>& available_members) override {
        pre_code += "template <typename T>\n";
        pre_code += "struct _" + name + "_" + component_name + ": public T {\n";
        for (const std::string& member: available_members) {
            if (member.find('(') != std::string::npos) {
                pre_code += "    auto " + name + '_' + member + " { return T::" + member + "; }\n";
            } else {
                pre_code += "    auto& " + name + '_' + member + "() { return T::" + member + "; }\n";
            }
        } pre_code += "};\n\n";
        parents.push_back("public " + component_name + "<_" + name + "_" + component_name + "<" + name + ">>");
        friends.push_back("_" + name + "_" + component_name + "<" + name + ">");
    }
};

void changeType(std::variant<Class, Struct, Component, Module>& code, const std::string& type) {
    std::string name; std::visit([&name](auto&& v){ name = v.getName(); }, code);
    if (type == "structure") code = Struct(name);
    else if (type == "class") code = Class(name);
    else if (type == "component") code = Component(name);
    else if (type == "module") code = Module(name);
}

#endif //ARTCHITECT_CLASS_HPP
