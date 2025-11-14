/**
 * @file models.hpp
 * @brief 虚拟机对象模型（VM Models）核心定义
 * @author azhz1107cat
 * @date 2025-10-25
 */

#pragma once

#include <utility>

#include "vm.hpp"
#include "../deps/hashmap.hpp"
#include "../deps/bigint.hpp"
#include "../deps/rational.hpp"


namespace model {

class Object {
    deps::HashMap<Object*> attrs;
    size_t refc_ = 0;
public:
    void make_ref();
    void del_ref();
};

class CodeObject : public Object{
    std::vector<kiz::Instruction> code;
    std::vector<Object*> consts;
    std::vector<std::string> names;
    std::vector<std::tuple<size_t, size_t>> lineno_map;
    explicit CodeObject(const std::vector<kiz::Instruction>& code);
};

class Module : public Object{
    std::string name;
    CodeObject *code = nullptr;
    deps::HashMap<Object*> attrs;
    explicit Module(std::string name, CodeObject *code) : name(std::move(name)), code(code){};
};

class Function : public Object{
    std::string name;
    CodeObject *code = nullptr;
    size_t argc = 0;
    explicit Function(std::string name, CodeObject *code) : name(std::move(name)), code(code){};
};

class Int : public Object{
    deps::BigInt val;
    explicit Int(deps::BigInt val) : val(std::move(val)) {}
};

class Rational : public Object {
    deps::Rational val;
    explicit Rational(const deps::Rational& val) : val(val) {}
};

class String : public Object {
    std::string val;
    explicit String(std::string val) : val(std::move(val)) {}
};

class List : public Object{
    std::vector<Object*> val;
    explicit List(std::vector<Object*> val) : val(std::move(val)) {}
};

class Dictionary : public Object {

};

class Bool : public Object {};
class Nil : public Object{};

};