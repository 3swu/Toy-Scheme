# Toy-Scheme

![](https://img.shields.io/badge/platform-Linux%20%7C%20macOS-flat.svg) ![](https://img.shields.io/badge/language-C-orange.svg) ![](https://img.shields.io/badge/license-MIT-000000.svg)


这是一个Scheme语言解释器，使用C语言实现了大部分R5RS的语法特性,参考[计算机程序的构造和解释](https://mitpress.mit.edu/sites/default/files/sicp/full-text/book/book.html)中元解释器的原理。 

### Feature
---
支持的表达式：
+ `quote`
+ `'` (`quote` 简写)
+ `quasiquote` / `` ` ``
+ `unquote` / `,`
+ `unquote-splicing` / `,@`
+ `define`
+ `if`
+ `lambda`
+ `begin`
+ `cond`
+ `let`
+ `let*`
+ `letrec`
+ `define-syntax` + `syntax-rules` (常用模式)
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
+ `eqv?` / `equal?` 等价判断
+ `null?` 判断是否为空表
+ `boolean?` 判断是否为布尔型
+ `symbol?` 判断是否是符号
+ `integer?` 判断是否为整数
+ `char?` 判断是否为字符
+ `string?` 判断是否为字符串
+ `pair?` 判断是否为序对
+ `vector?` / `vector` / `vector-length` / `vector-ref` / `vector-set!`
+ `vector->list` / `list->vector`
+ `procedure?` 判断是否为内部过程或复合过程
+ `apply` / `map` / `for-each`
+ `call/cc`
+ `number->string` 将整数转换为字符串
+ `string->number` 将字符串转换为整数
+ `symbol->string` 将符号转换为字符串
+ `string->symbol` 将字符串转换为符号
+ `string-length` / `string-ref` / `substring` / `string-append`
+ `char->integer` / `integer->char`
+ `environment` 查看全局环境中绑定的变量
+ `read` / `write` / `display` / `newline`
+ `read-line` / `eof-object?`
+ `open-input-file` / `open-output-file`
+ `close-input-port` / `close-output-port`
+ `current-input-port` / `current-output-port`
+ `load` 加载并执行指定 Scheme 文件

### Build & Install
---
`clone` 项目后在项目根目录执行：
``` bash
chmod +x ./COMPILE ./INSTALL ./TEST ./scripts/*.sh

# 编译（输出在 ./build）
./COMPILE
# 同时会同步根目录可执行文件 ./Toy-Scheme

# 安装到 /usr/local/bin（需要管理员权限）
USE_SUDO=1 ./INSTALL
```

### Usage
---
直接进入REPL环境
```bash
./Toy-Scheme
```
在交互终端中支持方向键编辑、历史记录和 `(` 自动补 `)`（依赖系统 `readline/libedit`，若缺失会自动降级为原始输入模式）。

解释指定的源文件，使用命令`./build/Toy-Scheme -f [file]`
```bash
./build/Toy-Scheme -f hello.scm
```

### Test
---
运行完整测试集：
```bash
./TEST
```
测试中间产物会放在项目目录下的 `./test-artifacts/`。

### Examples
---
- 元解释器：`./Toy-Scheme -f examples/meta-scheme/demo.scm`
- 线性回归（梯度下降）：`./Toy-Scheme -f examples/ml-linear-regression/linear_regression.scm`
- CSV + SQL 子集引擎：`./Toy-Scheme -f examples/sql-engine/demo.scm`

### Requirement
---
为了编译项目，需要有以下的组件：
+ GCC / Clang
+ Cmake

### Author
---
Wu Lei (<a href="mailto:wuleiatso@gmail.com">wuleiatso@gmail.com</a>)
