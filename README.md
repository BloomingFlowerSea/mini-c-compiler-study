# 一周冲刺：在阿里云服务器上手写简易 C 编译器

这是一个以“快速实践”为核心的学习型项目：

- 开发环境：阿里云 Linux 服务器
- 学习周期：1 周（猛猛学）
- 核心目标：从 C 语言基础练习出发，逐步完成一个简易 C 编译器（当前阶段以解释执行为主）

---

## 项目目标

1. 夯实 C 语言基础（指针、结构体、内存管理、模块拆分、Makefile）
2. 理解编译器核心流程：词法分析 -> 语法分析 -> 语义处理 -> 执行/代码生成
3. 在一周内跑通一个可执行的最小编译器/解释器原型
4. 保留学习轨迹，后续继续扩展到更完整的编译器能力

---

## 当前仓库结构

```text
.
├── main.c / message.c / message.h / Makefile
├── test.c
├── mini-c-compiler/
│   ├── include/
│   ├── src/
│   ├── docs/
│   ├── examples/
│   └── README.md
└── Teacher-compiler/
    ├── simple_c_compiler.c
    ├── test_program.c
    └── Makefile
```

说明：

- 根目录代码：用于 C 基础练习和 Makefile 练习
- `mini-c-compiler/`：主线学习项目（当前为可运行的解释器框架）
- `Teacher-compiler/`：参考实现，用来对照学习完整编译器阶段（词法/语法/语义/代码生成）

---

## 一周学习冲刺计划

### Day 1：环境与 C 基础

- 熟悉阿里云服务器开发流程（SSH、目录、编译、运行）
- 回顾 C 语法、函数、头文件拆分
- 用 Makefile 管理最小项目编译

### Day 2：词法分析（Lexer）

- 识别关键字、标识符、数字、运算符、分隔符
- 输出 token 流，验证输入文本切分是否正确

### Day 3：语法分析（Parser）

- 构建 AST（抽象语法树）
- 支持表达式优先级、变量声明与赋值语句

### Day 4：解释执行（Interpreter）

- 遍历 AST 执行表达式和语句
- 维护符号表，支持变量读写

### Day 5：错误处理与测试

- 处理语法错误、未定义变量、除零等场景
- 增加示例输入和回归测试

### Day 6：扩展语法

- 尝试加入 `if`、`while`、函数调用等特性（按精力选做）

### Day 7：总结与复盘

- 整理项目文档和模块说明
- 输出学习笔记：做对了什么、踩了什么坑、下一周怎么继续

---

## 快速开始

### 1) 运行根目录基础示例

```bash
make
make run
```

清理构建产物：

```bash
make clean
```

### 2) 运行 mini-c-compiler（解释器主线）

```bash
cd mini-c-compiler
gcc src/main.c src/lexer.c src/parser.c src/interpreter.c src/utils.c -Iinclude -o mini-c.exe
./mini-c.exe examples/test.txt
```

### 3) 运行 Teacher-compiler（参考实现）

```bash
cd Teacher-compiler
make
make test
make asm
```

---

## 阶段状态

- 项目状态：进行中（WIP）
- 当前重点：先把“可运行的最小版本”做扎实，再逐步扩展语法和代码生成能力

---

## 适合谁

如果你也想在短时间内高强度入门编译原理，这个仓库可以作为一个从 0 到 1 的实践模板：

- 有 C 基础但没写过编译器
- 学过理论但缺一个可运行项目
- 想在 Linux 服务器上边学边实战

---

## 后续计划

- 增加自动化测试脚本
- 完善文档（每一步对应一份阶段说明）
- 逐步从解释执行过渡到真正的代码生成

欢迎持续迭代，一周先跑通，后面再持续升级。