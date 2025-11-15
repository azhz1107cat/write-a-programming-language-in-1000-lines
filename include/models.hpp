/**
 * @file models.hpp
 * @brief 虚拟机对象模型（VM Models）核心定义
 * @author azhz1107cat
 * @date 2025-10-25
 */

#pragma once

#include <atomic>
#include <functional>
#include <utility>

#include "vm.hpp"
#include "../deps/hashmap.hpp"
#include "../deps/bigint.hpp"
#include "../deps/rational.hpp"


namespace model {

class Object {
    std::atomic<size_t> refc_ = 0;  // 原子类型支持直接初始化为 0
public:
    deps::HashMap<Object*> attrs;

    void make_ref() {
        refc_.fetch_add(1, std::memory_order_relaxed);
    }
    void del_ref() {
        const size_t old_ref = refc_.fetch_sub(1, std::memory_order_acq_rel);

        if (old_ref == 1) {
            delete this;
        }
    }
    std::string to_string() {
        return "<Object>";
    }
    ~Object() {
        auto kv_list = attrs.to_vector();
        for (auto& [key, obj] : kv_list) {
            if (obj != nullptr) obj->del_ref();
        }
    }
};

class CodeObject : public Object {
public:
    std::vector<kiz::Instruction> code;
    std::vector<Object*> consts;
    std::vector<std::string> names;
    std::vector<std::tuple<size_t, size_t>> lineno_map;
    explicit CodeObject(const std::vector<kiz::Instruction>& code,
        const std::vector<Object*>& consts,
        const std::vector<std::string>& names,
        const std::vector<std::tuple<size_t, size_t>>& lineno_map
    ) : code(code), consts(consts), names(names), lineno_map(lineno_map) {}
};

class Module : public Object {
public:
    std::string name;
    CodeObject *code = nullptr;
    deps::HashMap<Object*> attrs;
    explicit Module(std::string name, CodeObject *code) : name(std::move(name)), code(code) {
        code->make_ref();
    }
};

class Function : public Object {
public:
    std::string name;
    CodeObject *code = nullptr;
    size_t argc = 0;
    explicit Function(std::string name, CodeObject *code, const size_t argc
    ) : name(std::move(name)), code(code), argc(argc) {
        code->make_ref();
    }
};

class Int : public Object {
public:
    deps::BigInt val;
    explicit Int(deps::BigInt val) : val(std::move(val)) {}
    explicit Int() : val(deps::BigInt(0)) {}
    [[nodiscard]] std::string to_string() const {
        return val.to_string();
    }
};

class Rational : public Object {
public:
    deps::Rational val;
    explicit Rational(const deps::Rational& val) : val(val) {}
};

class String : public Object {
public:
    std::string val;
    explicit String(std::string val) : val(std::move(val)) {}
};

class List : public Object {
public:
    std::vector<Object*> val;
    explicit List(std::vector<Object*> val) : val(std::move(val)) {}
};

class CppFunction : public Object {
public:
    std::function<Object*(List*)> func;
    explicit CppFunction(std::function<Object*(List*)> func) : func(std::move(func)) {}
};

class Dictionary : public Object {
public:
    deps::HashMap<Object*> attrs;
    explicit Dictionary(const deps::HashMap<Object*>& attrs) : attrs(attrs) {}
    explicit Dictionary() {
        deps::HashMap<Object*> attrs{};
    }
};

class Bool : public Object {
public:
    bool val;
    explicit Bool(const bool val) : val(val) {}
};

class Nil : public Object {
public:
    explicit Nil() : Object() {}
};

};