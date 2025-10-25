/**
 * @file models.hpp
 * @brief 虚拟机对象模型（VM Models）核心定义
 * @author azhz1107cat
 * @date 2025-10-25
 */

#pragma once

namespace model {

class Model {
    size_t refc_;
public:
    void make_ref();
    void del_ref();
    };

class CodeObject : public Model{
    std::vector<my-lang::Introduction> code;
    std::vector<std::shared_ptr<Model>> consts;
};

class Module : public Model{
    std::string name;
    std::shared_ptr<CodeObject> code;
    std::unordered_map<std::string, std::shared_ptr<Model>> attrs;
};

class Function : public Model{
    std::string name;
    std::shared_ptr<CodeObject> code;
    size_t argc;
};

class Class : public Model{
    std::string name;
    std::vector<std::shared_ptr<Class>> parents;
    std::unordered_map<std::string, std::shared_ptr<Model>> attrs;
};

class Instance : public Model{
    std::shared_ptr<Class> ref_class;
    std::unordered_map<std::string, std::shared_ptr<Model>> attrs;
};

class Int : public Model{};
class Dec : public Model {};
class String : public Model {
    std::string val;
};
class List : public Model{};
class Dictionary : public Model {};
class Bool : public Model {};
class Nil : public Model{};
};