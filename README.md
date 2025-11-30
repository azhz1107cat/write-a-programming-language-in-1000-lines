# Kiz-lang v0.1.0
📌 **现状: 开发中...**

- 📚 文档完善
- 🪄 多范式兼容：支持OOP、FP等主流编程范式
- 🔅 语法极简：关键字集高度精简，仅包含：
```kiz
if else while break next
fn end dict import
try catch throw 
nonlocal global 
is not or in and 
True Nil False
```
- ✅ 规范友好：中文注释+统一命名规范
- ✔️ 开发者友好：低门槛快速上手
- 📃 TODO: 
    - ~~**feature** 完成list的IR生成~~
    - **frature** 完成字典的定义与使用
    - ~~**fix** user function的调用问题~~ (感谢三文鱼)
    - **fix** if, while stmt的跳转问题
    - ~~**feature** 实现getattr~~
    - **feature** 实现setattr
    - **fix** 修复Nil, False, True作为字面量出现的undefined var问题
    - **fix** 统一报错和DEBUG信息和输出信息为标准英文
    - **debug&fix** 测试注释
    - **debug&fix** 测试完整的parser
    - **debug&fix** 测试set nonlocal和set global
    - **feature** 完成 >= <= (通过编译为NOT GT, NOT LT) 这样的字节码
    - **feature** 完成 and not or in运算符(在vm中要支持判断model::Bool, 如果对象不是model::Bool, 需尝试调用Object::magic_bool魔术方法)
    - **feature** 所有报错使用util::err_reporter函数代替现在临时的assert
    - **feature** 添加对于运行时错误的报错器的TraceBack输出
    - **feature(maybe has break change)** Object->to_string改为Object的魔术方法(magic_str)
    - **feature** 添加import, 循环导入检查, std模块系统(在model::std_modules中注册)和用户模块系统
    - **feature** 完善builtins object的magic_bool, magic_getitem, magic_setitem, magic_str魔术方法, 同时支持用户定义的魔术方法
    - **feature** 完成try-catch throw语句
