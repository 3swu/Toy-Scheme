# Toy-Scheme

![](https://img.shields.io/badge/platform-Linux%20%7C%20macOS-flat.svg) ![](https://img.shields.io/badge/language-C-orange.svg) ![](https://img.shields.io/badge/license-MIT-000000.svg)


这是一个Scheme语言解释器，使用C语言实现了大部分R5RS的语法特性,参考[计算机程序的构造和解释](https://mitpress.mit.edu/sites/default/files/sicp/full-text/book/book.html)中元解释器的原理。 

### Feature
---
支持的表达式：
+ `quote`
+ `define`
+ `if`
+ `lambda`
+ `begin`
+ `cond`
+ `let`
+ `...` 

支持的内部过程：
+ `+` 整数求和
+ `-` 整数求差
+ `*` 整数求积
+ `/` 整数求商
+ `quotient` 整数求商
+ `remainder` 求余数
+ `=` 判断整数是否相等
+ `<` 判断整数大小
+ `>` 判断整数大小
+ `cons` 连接两个列表或原子
+ `car` 求列表的表头
+ `cdr` 求列表的表尾
+ `set-car!` 设置列表的表头
+ `set-cdr!` 设置列表的表尾
+ `list` 构造列表
+ `eq?` 判断两个列表或原子是否相等
+ `null?` 判断是否为空表
+ `boolean?` 判断是否为布尔型
+ `symbol?` 判断是否是符号
+ `integer?` 判断是否为整数
+ `string?` 判断是否为字符串
+ `pair?` 判断是否为序对
+ `procedure?` 判断是否为内部过程或复合过程
+ `number->string` 将整数转换为字符串
+ `string->number` 将字符串转换为整数
+ `symbol->string` 将符号转换为字符串
+ `string->symbol` 将字符串转换为符号
+ `environment` 查看全局环境中绑定的变量

### Build & Install
---
`clone`项目并且在项目内执行：
``` bash
sudo chmod +x ./INSTALL
sudo ./INSTALL
```

### Usage
---
直接进入REPL环境
```bash
./Toy-Scheme
```
解释指定的源文件，使用命令`./Toy-Scheme -f [file]`
```bash
./Toy-Scheme -f hello.scm
```
### Requirement
---
为了编译项目，需要有以下的组件：
+ GCC / Clang
+ Cmake

### Author
---
Wu Lei (<a href="mailto:wuleiatso@gmail.com">wuleiatso@gmail.com</a>)
