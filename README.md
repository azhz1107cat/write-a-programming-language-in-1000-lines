# Kiz-lang
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
    - fix list的空指针问题
    - fix 字典的定义
    - fix user function的调用问题
    - fix if, while stmt的跳转问题
    - debug 测试注释
    - debug&fix 测试set nonlocal和set global
    - feature 所有报错使用util::err_reporter函数代替现在临时的assert
    - feature 添加TraceBack
    - feature 添加import, std模块系统(在model::std_modules中注册)和用户模块系统