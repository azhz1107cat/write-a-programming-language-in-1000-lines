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

#include "kiz.hpp" // 不能删 !!!
#include "opcode.hpp"
#include "../deps/hashmap.hpp"
#include "../deps/bigint.hpp"
#include "../deps/rational.hpp"


// 获取对象的魔法方法（参数：对象指针、魔法方法名，如add/mul/eq）
#define GET_MAGIC_METHOD(obj_ptr, method_name) \
    ([&]() -> model::Object* { \
        if (!obj_ptr) { \
            DEBUG_OUTPUT("GET_MAGIC_METHOD: obj_ptr is nullptr"); \
            return nullptr; \
        } \
        switch (obj_ptr->get_type()) { \
            case model::Object::ObjectType::OT_Int: \
                return model::Int::magic_##method_name; \
            case model::Object::ObjectType::OT_String: \
                return model::String::magic_##method_name; \
            case model::Object::ObjectType::OT_List: \
                return model::List::magic_##method_name; \
            case model::Object::ObjectType::OT_Rational: \
                return model::Rational::magic_##method_name; \
            case model::Object::ObjectType::OT_Dictionary: \
                return model::Dictionary::magic_##method_name; \
            case model::Object::ObjectType::OT_Bool: \
                return model::Bool::magic_##method_name; \
            case model::Object::ObjectType::OT_Nil: \
                return model::Nil::magic_##method_name; \
            default: \
                DEBUG_OUTPUT("GET_MAGIC_METHOD: unsupported type for " #method_name " (type: " + obj_ptr->to_string() + ")"); \
                return nullptr; \
        } \
    }())

// 检查魔法方法是否有效（避免空指针/类型错误）
#define CHECK_MAGIC_METHOD_VALID(magic_ptr, method_name, obj_type_str) \
    ([&]() -> bool { \
        if (!magic_ptr) { \
            DEBUG_OUTPUT("CHECK_MAGIC_METHOD_VALID: " #method_name " is nullptr for type: " + obj_type_str); \
            return false; \
        } \
        if (magic_ptr->get_type() != model::Object::ObjectType::OT_CppFunction) { \
            DEBUG_OUTPUT("CHECK_MAGIC_METHOD_VALID: " #method_name " is not CppFunction (type: " + magic_ptr->to_string() + ")"); \
            return false; \
        } \
        return true; \
    }())

namespace kiz {

struct Instruction {
    Opcode opc;
    std::vector<size_t> opn_list;
    size_t start_lineno;
    size_t end_lineno;
};

}

namespace model {

class Object {
    std::atomic<size_t> refc_ = 0;
public:
    static Object* magic_add;
    static Object* magic_sub;
    static Object* magic_mul;
    static Object* magic_div;
    static Object* magic_pow;
    static Object* magic_mod;
    static Object* magic_in;
    static Object* magic_bool;
    static Object* magic_eq;
    static Object* magic_lt;
    static Object* magic_gt;
    static deps::HashMap<Object*> attrs;

    // 对象类型枚举
    enum class ObjectType {
        OT_Nil, OT_Bool, OT_Int, OT_Rational, OT_String,
        OT_List, OT_Dictionary, OT_CodeObject, OT_Function,
        OT_CppFunction, OT_Module
    };

    // 获取实际类型的虚函数
    [[nodiscard]] virtual ObjectType get_type() const = 0;

    void make_ref() {
        refc_.fetch_add(1, std::memory_order_relaxed);
    }
    void del_ref() {
        const size_t old_ref = refc_.fetch_sub(1, std::memory_order_acq_rel);

        if (old_ref == 1) {
            delete this;
        }
    }
    [[nodiscard]] virtual std::string to_string() const = 0;
    virtual ~Object() {
        auto kv_list = attrs.to_vector();
        for (auto& [key, obj] : kv_list) {
            if (obj != nullptr) obj->del_ref();
        }
    }
};

class List;

class CodeObject : public Object {
public:
    std::vector<kiz::Instruction> code;
    std::vector<Object*> consts;
    std::vector<std::string> names;
    std::vector<std::tuple<size_t, size_t>> lineno_map;

    static constexpr ObjectType TYPE = ObjectType::OT_CodeObject;
    [[nodiscard]] ObjectType get_type() const override { return TYPE; }

    explicit CodeObject(const std::vector<kiz::Instruction>& code,
        const std::vector<Object*>& consts,
        const std::vector<std::string>& names,
        const std::vector<std::tuple<size_t, size_t>>& lineno_map
    ) : code(code), consts(consts), names(names), lineno_map(lineno_map) {}

    [[nodiscard]] std::string to_string() const override {
        return "<CodeObject: consts=" + std::to_string(consts.size()) + ", names=" + std::to_string(names.size()) + ">";
    }

    ~CodeObject() override {
        for (Object* const_obj : consts) {
            if (const_obj != nullptr) {
                const_obj->del_ref();
            }
        }
    }
};

class Module : public Object {
public:
    std::string name;
    CodeObject *code = nullptr;
    deps::HashMap<Object*> attrs;

    static constexpr ObjectType TYPE = ObjectType::OT_Module;
    [[nodiscard]] ObjectType get_type() const override { return TYPE; }

    explicit Module(std::string name, CodeObject *code) : name(std::move(name)), code(code) {
        code->make_ref();
    }

    [[nodiscard]] std::string to_string() const override {
        return "<Module: name=\"" + name + "\">";
    }
};

class Function : public Object {
public:
    std::string name;
    CodeObject *code = nullptr;
    size_t argc = 0;

    static constexpr ObjectType TYPE = ObjectType::OT_Function;
    [[nodiscard]] ObjectType get_type() const override { return TYPE; }

    explicit Function(std::string name, CodeObject *code, const size_t argc
    ) : name(std::move(name)), code(code), argc(argc) {
        code->make_ref();
    }

    [[nodiscard]] std::string to_string() const override {
        return "<Function: name=\"" + name + "\", argc=" + std::to_string(argc) + ">";
    }
};

class CppFunction : public Object {
public:
    std::string name="";
    std::function<Object*(Object*, List*)> func;

    static constexpr ObjectType TYPE = ObjectType::OT_CppFunction;
    [[nodiscard]] ObjectType get_type() const override { return TYPE; }

    explicit CppFunction(std::function<Object*(Object*, List*)> func) : func(std::move(func)) {}
    [[nodiscard]] std::string to_string() const override {
    return "<CppFunction" + 
           (name.empty() 
            ? "" 
            : ": name=\"" + name + "\""
            ) 
            + ">";
}
};

class List : public Object {
public:
    std::vector<Object*> val;

    static Object* magic_add;
    static Object* magic_mul;
    static Object* magic_in;
    static Object* magic_eq;

    static constexpr ObjectType TYPE = ObjectType::OT_List;
    [[nodiscard]] ObjectType get_type() const override { return TYPE; }

    explicit List(std::vector<Object*> val) : val(std::move(val)) {}
    [[nodiscard]] std::string to_string() const override {
        std::string result = "[";
        for (size_t i = 0; i < val.size(); ++i) {
            if (val[i] != nullptr) {
                result += val[i]->to_string();  // 递归调用元素的 to_string
            } else {
                result += "Nil";
            }
            if (i != val.size() - 1) {
                result += ", ";
            }
        }
        result += "]";
        return result;
    }
};

class Int : public Object {
public:
    deps::BigInt val;

    static Object* magic_add;
    static Object* magic_sub;
    static Object* magic_mul;
    static Object* magic_div;
    static Object* magic_pow;
    static Object* magic_mod;
    static Object* magic_bool;
    static Object* magic_eq;
    static Object* magic_lt;
    static Object* magic_gt;

    static constexpr ObjectType TYPE = ObjectType::OT_Int;
    [[nodiscard]] ObjectType get_type() const override { return TYPE; }

    explicit Int(deps::BigInt val) : val(std::move(val)) {}
    explicit Int() : val(deps::BigInt(0)) {}
    [[nodiscard]] std::string to_string() const override {
        return val.to_string();
    }
};

class Rational : public Object {
public:
    deps::Rational val;

    static Object* magic_add;
    static Object* magic_sub;
    static Object* magic_mul;
    static Object* magic_div;
    static Object* magic_eq;
    static Object* magic_lt;
    static Object* magic_gt;

    static constexpr ObjectType TYPE = ObjectType::OT_Rational;
    [[nodiscard]] ObjectType get_type() const override { return TYPE; }

    explicit Rational(const deps::Rational& val) : val(val) {}
    [[nodiscard]] std::string to_string() const override {
        return val.numerator.to_string() + "/" + val.denominator.to_string();
    }
};

class String : public Object {
public:
    std::string val;

    static Object* magic_add;
    static Object* magic_mul;
    static Object* magic_in;
    static Object* magic_eq;

    static constexpr ObjectType TYPE = ObjectType::OT_String;
    [[nodiscard]] ObjectType get_type() const override { return TYPE; }

    explicit String(std::string val) : val(std::move(val)) {}
    [[nodiscard]] std::string to_string() const override {
        return "\"" + val + "\"";
    }
};

class Dictionary : public Object {
public:
    deps::HashMap<Object*> attrs;

    static Object* magic_add;
    static Object* magic_in;

    static constexpr ObjectType TYPE = ObjectType::OT_Dictionary;
    [[nodiscard]] ObjectType get_type() const override { return TYPE; }

    explicit Dictionary(const deps::HashMap<Object*>& attrs) : attrs(attrs) {}
    explicit Dictionary() {
        this->attrs = deps::HashMap<Object*>{};
    }

    [[nodiscard]] std::string to_string() const override {
        std::string result = "{";
        auto kv_list = attrs.to_vector();
        for (size_t i = 0; i < kv_list.size(); ++i) {
            auto& [key, val] = kv_list[i];

            std::string val_str = (val != nullptr) ? val->to_string() : "nil";

            result += key + ": " + val_str;
            if (i != kv_list.size() - 1) {
                result += ", ";
            }
        }
        result += "}";
        return result;
    }
};

class Bool : public Object {
public:
    bool val;

    static Object* magic_eq;

    static constexpr ObjectType TYPE = ObjectType::OT_Bool;
    [[nodiscard]] ObjectType get_type() const override { return TYPE; }

    explicit Bool(const bool val) : val(val) {}
    [[nodiscard]] std::string to_string() const override {
        return val ? "True" : "False";
    }
};

class Nil : public Object {
public:
    static Object* magic_eq;

    static constexpr ObjectType TYPE = ObjectType::OT_Nil;
    [[nodiscard]] ObjectType get_type() const override { return TYPE; }

    explicit Nil() : Object() {}
    [[nodiscard]] std::string to_string() const override {
        return "Nil";
    }
};

inline deps::HashMap<Object*> std_modules;

void registering_magic_methods();
void registering_std_modules();

};