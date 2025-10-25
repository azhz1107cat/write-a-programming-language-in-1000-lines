/**
 * @file models.hpp
 * @brief 虚拟机对象模型（VM Models）核心定义
 * @author azhz1107cat
 * @date 2025-10-25
 */

#pragma once

namespace model {

class Object {
    size_t refc_;
public:
    void make_ref();
    void del_ref();
    };

class CodeObject : public Object{
    std::vector<kiz::Introduction> code;
    std::vector<std::shared_ptr<Object>> consts;
};

class Module : public Object{
    std::string name;
    std::shared_ptr<CodeObject> code;
    std::unordered_map<std::string, std::shared_ptr<Object>> attrs;
};

class Function : public Object{
    std::string name;
    std::shared_ptr<CodeObject> code;
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
    std::vector<std::shared_ptr<Object>> val;
};

class Dictionary : public Object {

};

class Bool : public Object {};
class Nil : public Object{};

};