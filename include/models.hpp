/**
 * @file models.hpp
 * @brief 虚拟机对象模型（VM Models）核心定义
 * @author azhz1107cat
 * @date 2025-10-25
 */

#pragma once

namespace model {

class Object {
    deps::HashMap<std::string, Object*> attrs;
    size_t refc_;
public:
    void make_ref();
    void del_ref();
    };

class CodeObject : public Object{
    std::vector<kiz::Instruction> code;
    std::vector<Object*> consts;
    std::vector<std::string> names;
    std::vector<std::tuple<size_t, size_t>> lineno_map;
};

class Module : public Object{
    std::string name;
    CodeObject* code;
    deps::HashMap<std::string, Object*> attrs;
};

class Function : public Object{
    std::string name;
    CodeObject* code;
    size_t argc;
};

class Int : public Object{
    deps::BigInt val;
};

class Rational : public Object {
};

class String : public Object {
    std::string val;
};

class List : public Object{
    std::vector<Object*> val;
};

class Dictionary : public Object {

};

class Bool : public Object {};
class Nil : public Object{};

};