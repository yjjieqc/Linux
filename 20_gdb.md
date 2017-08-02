# 20  gdb调试工具

程序中除了一目了然的Bug之外都需要一定的调试手段来分析到底错在哪。到目前为止我们的调试手段只有一种：根据程序执行时的出错现象假设错误原因，然后在代码中适当的位置插入printf，执行程序并分析打印结果，如果结果和预期的一样，就基本上证明了自己假设的错误原因，就可以动手修正Bug了，如果结果和预期的不一样，就根据结果做进一步的假设和分析。本章我们介绍一种非常强大的调试工具gdb，可以完全操控程序的运行，使得程序就像你手里的玩具一样，叫它走就走，叫它停就停，并且随时可以查看程序中所有的内部状态，比如各变量的值、传给函数的参数、当前执行的语句位置等。掌握了gdb的用法以后，调试的手段就更加丰富了。但要注意，即使调试的手段非常丰富了，其基本思想仍然是“分析现象一＞假设错误原因一＞产生新的现象去验证假设”这样一个循环，根据现象如何假设错误原因，以及如何设计新的现象去验证假设，这都需要非常严密的分析和思考，如果因为手里有了强大的工具就滥用，而忽视了严谨的思维，往往会治标不治本地修正Bug，导致一个错误现象消失了但Bug仍然存在，甚至是把程序越改越错。本章通过几个初学者易犯的错误实例来讲解如何使用gdb调试程序，在每个实例后面总结一部分常用gdb命令。

```
        gcc -g main.c -o main
```

常用命令

命令|简写 |作用
-------|-----|-----
help |h|按模块列出命令类
help <class\>| |查看某一类型的具体命令
list|l|查看代码，可跟行号函数名
quit|q|退出gdb
run|r|全速运行程序
start| |单步执行，运行程序，停在第一执行语句
next|n|逐过程执行
step|s|逐语句执行，遇到函数，调到函数内执行
backtrace|bt|查看函数的调用的栈帧和层级的关系
info|i|查看函数内部局部变量的数值
frame|f|切换函数的栈帧
finish| | 结束当前函数，返回到函数调用点
set| |设置当前变量的值
run argv[1] argv[2]| |调试时命令行传参
print|p|打印变量和地址
break|b|设置断点，可根据行号和函数名
delete|d|删除断点，用法：d breakpoint NUM
display| | 设置观察变量
undisplay| |取消观察变量
continue|c|继续全速运行剩下的代码
enable breakpoint| |启用断点
disable breakpoint| |禁用断点
x| |查看内存 x /20xw 显示20个单元，16进制，4字节每个单元
watch| |被设置观察点的变量发生变化时，打印显示
i watch| |显示观察点
core| |ulimit -c 1024 开启core文件，调试时 gdb a.out 若出错生成core

## 20.1  gdb调试模式

run 全速运行

start   单步调试

set follow-fork-mode child  # Makefile项目管理

## 20.2  用途

+ 项目代码编译管理
+ 节省编译项目时间
+ 一次编写终身受益
+ 操作示例文件：add.c sub.c mul.c dive.c main.c

## 20.3  基本规则

### 20.3.1  三要素

* 目标
* 条件
* 命令

## 20.4  Makefile工作原理

* 分析各个目标和依校之问的关系
* 根据依赖关系自底向上执行命令
* 根据修改时间比目标新，确定更新
* 如果目标不依核任何条件，则执行对应命令以示更新

## 20.4  clean

* 用途：清除编译生成的中问.o文件和最终目标文件
* makeclean如果当前目录下有同名clean文件，则不执行clean对应的命令
* 伪目标声明：.PHONY:clean
* clean命令中的特殊符号
- "-" 此条命令出错，make也会继续执行后续的命令。如："-rm main.o"
- "@" 不显示命令本身，只显示结果。如： "@echo "clean done""
* 其他：
- make默认执行第一个出现的目标，可通过make dest指定要执行的目标
- distclean目标
- isntall目标
- make - C 路径名  表示进入指定目录，调用里面的makefile



注：

makefile中有一些特殊变量

**$@**表示目标，**$^**表示所有依赖，**$<**表示依赖的第一个

基于字符串的內建函数

函数用法|用途
-----|-----
$(wildcard *.c)| 查找符合条件的文件名
$(subst ee, EE, feet)|字符串替换
$(patsubst %.c, %.o, a.c)|模式查找替换
$(stirp a b c)|去除参数中开头和结尾的空格
$(findstring a, a b c)|在参数2中查找参数1，若找到返回参数1，否则返回空
$(filter %.c, s, $(source))|过滤出符合模式后缀的文件名
$(filter-out a b, a b c d)|反向过滤，过滤出c和d
$(sort d a b c)|生序排序

基于文件名的常用函数

函数用法|作用
----|----
$(dir src/foo.c hack.c)|返回目录名 src/ ./
$(notdir src/foo.c hack.c)|返回文件名foo.c hack.c
$(suffix src/foo.c src-1/bar.s hacks)|返回后缀.c .s
$(basename src/foo.c src-1/bar.c hacks)|返回前缀src/foo src-1/bar hacks
$(addsuffix .c, foo bar)|返回添加后缀的文件名 foo.c bar.c
$(addperfix src/, foo bar)|返回添加前缀后的文件名 src/foo src/bar
$(jion aaa bbb, 111 222 333)|返回链接后值：aaa111 bbb222 333





















