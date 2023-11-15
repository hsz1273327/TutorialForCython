# 纯净模式语法

纯净模式语法就是python语法,不同之处是cython提供了多个装饰器和`typehints`类型用于在`cython`编译时将对应位置作为对应的Cython语法中的单元处理.

纯净模式最大好处是python解释器可以直接处理,缺点是功能并未能覆盖cython语法的全部.

本文仅介绍实现部分的纯净模式语法,声明部分建议还是使用cython语法,我们会统一介绍.

纯净模式下通过装饰器和`typehints`类型增加的内容包括:

+ C对象实现部分
    + C/C++变量类型声明
    + C自定义结构声明
    + Cython提供的运算符
    + C函数声明
    + 底层内存的调用和管理
    
+ python对象部分
    + 静态化变量和参数
    + 扩展类型

+ 导入依赖
+ 并行计算
+ 内存调用和管理

需要注意:

1. Cython基本可以覆盖C语言的所有语法,但并不能覆盖C++的语法,除了stl外基本只能支持包装外部库.
2. 本文中以`*`开头的章节在纯净模式下无法使用,以`+`开头的章节为部分功能在纯净模式下无法使用,请参考对应的cython语法部分章节

## 识别模块是否编译

纯净模式下由于python解释器可以直接处理,这就造成了我们可能无法判别使用的模块是否有被编译.可以使用cyhton库判断.

```python
import cython

if cython.compiled:
    print("Yep, I'm compiled.")
    
else:
    print("Just a lowly interpreted script.")
```


```python

```
