# Cython

Cython语言是独立于python的一种语言,用于描述python对象和C/C++对象.python解释器无法解释cython代码,必须通过命令行工具`cython`转译成C代码后在用gcc或g++编译成可执行文件或者通过命令行工具`cythonize`编译成python可以加载的动态链接库后由python解释器加载后使用.

通常Cython并不单独作为一门编程语言直接使用,一般都是用做桥接python和C,使用范围上比较局限.

[Cython本身有文档](http://docs.cython.org/en/latest/index.html)质量也还算不错,知识点基本都有,但实话说个人认为结构比较混乱,很多知识点都是点到为止,基本仅涉及cython本身,基本没有介绍如何与python或C/C++结合的内容,因此才会有这篇攻略

这篇攻略我将尽量将知识点梳理清晰,并以我个人的最佳编码实践为基准统一风格,除了介绍Cython本身外更多的会结合实际的使用场景,通过例子来详细介绍从开发到发布的全流程工作流.

本文基于Cython 3.0+

## Cython语言的总体设计

Cython一般用做桥接python和C,因此它要做到

1. 让python解释器可以调用cython代码
2. 让C/C++代码逻辑可以调用cython代码

所以Cython定义了3种不同的部分:

+ python对象部分,源码会被翻译为用python的ABI编写的C/C++代码段,这部分用于和python解释器对接
+ C对象实现部分.源码会被翻译成单纯的C/C++代码段,这部分可以用于和C/C++对接
+ C/C++接口声明部分.用于将外部C/C++库的头文件包装为Cython对象以及给外部库提供接口信息便于模块化

cython针对实现部分(也就是python对象部分和C对象实现部分)提供了两种语法支持,即cython语法和纯净模式语法,而C/C++接口声明部分则是统一的cython语法

在同一模块中这实现部分可以相互调用,但与模块外部的交互只能分别用不同的部分进行.

Cython在语法层面上接近python,无论是cython语法还是纯净模式语法都看着和python差不多;
在代码结构上,Cython更加接近C.有实现部分和申明部分,申明部分用于对外提供接口,实现部分则只需要满足接口就可以了.

## 源文件类型

Cython可以识别`.py`,`.cpp`,`.c`,`.h`,`.pyx`,`.pxd`,`.pxi`,其中

+ `.cpp`,`.c`,`.h`是c/c++的源码和头文件,一般是包装模式下被包装的C/C++部分代码.`.h`也可以是指向的静/动态链接库.
+ `.py`是python形式的源文件,可以在python解释其中被直接执行,也满足cython纯净模式,是一种比较通用的写法,但一些比较复杂的特性写起来会比较繁琐.
+ `.pyx`, Cython语法的实现部分源文件,用于编写包含Python和C代码混合的代码.pyx文件中的代码将被转换为`C`代码.对于cython就作用来说`.py`和`.pyx`是等价的,都是实现部分的源码文件,但习惯上`.pyx`文件中只会写cython语法的源码.
+ `.pxd`, Cython语法下的的头文件,用于声明同名源文件(`.pyx`或`.py`)中的实现的接口以及包装外部C/C++库的接口,其功能类似C/C++中的头文件,.这些.pxd文件可以被.pyx文件和其他.pxd文件导入从而实现不同模块之间的代码共享和重用.

    申明可以包含如下内容:
    
    + C型变量的声明
    + extern C函数或变量声明
    + 模块实现的声明
    + 扩展类型的定义部分
    + 外部函数库的所有声明等
    
    申明文件不能包含:
    
    + 任何非外部C变量声明
    + C或Python功能的实现
    + Python类定义和Python可执行语句
    + 任何被定义为公共的声明即申明可以被其他Cython模块访问.这是没有必要的，因为它是自动的.而且公共声明只需要使外部C代码可以访问.
    
+ `.pxi`,用于填写直接嵌入其他cython代码中的代码,在使用的时候在需要插入的位置使用`include "xxx.pxi"`就可以.其原理类似C的头文件,就是单纯的将文件中的内容嵌入,算是一种古早时期代码组织方式的遗留.现在除了条件编译基本不会用了

## Cython的代码组织模式

Cython的代码组织模式可以看做是python和C的结合,它既有模块又有接口声明.通常根据实现部分的源码类型分成两种:

+ 纯净模式(Pure Python Mode),源码可以直接在python解释器中使用的模式,代码的主体还是python代码,但使用类型标注,装饰器.以`.py`作为文件后缀.在精心设计下不进行编译的情况下也可以直接被使用.

+ Cython模式,源码就是纯正的Cython语法,必须通过编译才可以被使用

这两种代码组织模式我们会在后面的文章中单独分开介绍.

## helloworld

虽然没什么带表性,惯例的我们用一个helloworld作为第一个例子.


```python
%load_ext cython
```


```cython
%%cython
# distutils: language=c++
cpdef run():
    print("Hello World")

run()
```

    Hello World


## 阅读提示

这系列文章分为如下结构

+ `工具链`,介绍cython编译相关的工具和基本用法,注意这部分不会涉及具体场景下的使用,具体场景下的使用请看正文
+ `Cython的编译补充`,正文中关于Cython编译相关内容的补充
+ `Cython语法`,由于cython支持两种语法,因此这边开始进行分叉,我们没有必要两种都学都用,这部分呢和下面的`纯净模式`部分挑一部分学即可
    + C对象部分
    + Python对象部分
    + 并行计算
    + 调用numpy
+ `纯净模式语法`,由于cython支持两种语法,因此这边开始进行分叉,我们没有必要两种都学都用,这部分呢和上面的`Cython语法`部分挑一部分学即可
    + C对象部分
    + Python对象部分
    + 并行计算
    + 调用numpy
+ `接口声明和模块化编程`,介绍如何声明接口组织代码

