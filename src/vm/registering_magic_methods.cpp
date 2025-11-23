#include "../../include/models.hpp"
#include "../../include/models/bool_obj.hpp"
#include "../../include/models/nil_obj.hpp"
#include "../../include/models/int_obj.hpp"
#include "../../include/models/rational_obj.hpp"
#include "../../include/models/str_obj.hpp"
#include "../../include/models/list_obj.hpp"
#include "../../include/models/dict_obj.hpp"

namespace model {

// ========== 基类Object静态成员定义 ==========
Object* Object::magic_add = nullptr;
Object* Object::magic_sub = nullptr;
Object* Object::magic_mul = nullptr;
Object* Object::magic_div = nullptr;
Object* Object::magic_pow = nullptr;
Object* Object::magic_mod = nullptr;
Object* Object::magic_in = nullptr;
Object* Object::magic_bool = nullptr;
Object* Object::magic_eq = nullptr;
Object* Object::magic_lt = nullptr;
Object* Object::magic_gt = nullptr;
deps::HashMap<Object*> Object::attrs = deps::HashMap<Object*>();

// ========== Bool类静态成员定义 ==========
Object* Bool::magic_eq = nullptr;

// ========== Nil类静态成员定义 ==========
Object* Nil::magic_eq = nullptr;

// ========== Int类静态成员定义 ==========
Object* Int::magic_add = nullptr;
Object* Int::magic_sub = nullptr;
Object* Int::magic_mul = nullptr;
Object* Int::magic_div = nullptr;
Object* Int::magic_mod = nullptr;
Object* Int::magic_pow = nullptr;
Object* Int::magic_gt = nullptr;
Object* Int::magic_lt = nullptr;
Object* Int::magic_eq = nullptr;

// ========== Rational类静态成员定义 ==========
Object* Rational::magic_add = nullptr;
Object* Rational::magic_sub = nullptr;
Object* Rational::magic_mul = nullptr;
Object* Rational::magic_div = nullptr;
Object* Rational::magic_gt = nullptr;
Object* Rational::magic_lt = nullptr;
Object* Rational::magic_eq = nullptr;

// ========== String类静态成员定义 ==========
Object* String::magic_add = nullptr;
Object* String::magic_mul = nullptr;
Object* String::magic_in = nullptr;
Object* String::magic_eq = nullptr;

// ========== List类静态成员定义 ==========
Object* List::magic_add = nullptr;
Object* List::magic_mul = nullptr;
Object* List::magic_in = nullptr;
Object* List::magic_eq = nullptr;

// ========== Dictionary类静态成员定义 ==========
Object* Dictionary::magic_add = nullptr;
Object* Dictionary::magic_in = nullptr;

// ========== 注册魔法方法 ==========
void registering_magic_methods(){
    Bool::magic_eq = new CppFunction(bool_eq);
    Nil::magic_eq = new CppFunction(nil_eq);

    Int::magic_add = new CppFunction(int_add);
    Int::magic_sub = new CppFunction(int_sub);
    Int::magic_mul = new CppFunction(int_mul);
    Int::magic_div = new CppFunction(int_div);
    Int::magic_mod = new CppFunction(int_mod);
    Int::magic_pow = new CppFunction(int_pow);
    Int::magic_gt = new CppFunction(int_gt);
    Int::magic_lt = new CppFunction(int_lt);
    Int::magic_eq = new CppFunction(int_eq);

    Rational::magic_add = new CppFunction(rational_add);
    Rational::magic_sub = new CppFunction(rational_sub);
    Rational::magic_mul = new CppFunction(rational_mul);
    Rational::magic_div = new CppFunction(rational_div);
    Rational::magic_gt = new CppFunction(rational_gt);
    Rational::magic_lt = new CppFunction(rational_lt);
    Rational::magic_eq = new CppFunction(rational_eq);

    Dictionary::magic_add = new CppFunction(dict_add);
    Dictionary::magic_in = new CppFunction(dict_contains);

    List::magic_add = new CppFunction(list_add);
    List::magic_mul = new CppFunction(list_mul);
    List::magic_in = new CppFunction(list_contains);
    List::magic_eq = new CppFunction(list_eq);

    String::magic_add = new CppFunction(str_add);
    String::magic_mul = new CppFunction(str_mul);
    String::magic_in = new CppFunction(str_contains);
    String::magic_eq = new CppFunction(str_eq);
}

}
