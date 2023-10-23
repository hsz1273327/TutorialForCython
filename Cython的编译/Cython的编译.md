# Cython的编译

Cython本身只是一个转译工具,它的作用是将python/cython源码转译成C/C++代码.所谓的编译本质上是将转译好的C/C++源码使用C/C++编译器编译成需要的东西.

C/C++源码本质上没有任何使用价值,只有后续C/C++编译器编译出来的东西(可执行程序,动态链接库...)才有,因此我们通常还是会称呼这整个过程为编译.需要注意这是一种很容易混淆的说法,我个人并不提倡这种模糊的说法.

我们大体可以将整个Cython的编译过程分为

1. 转译,即将Cython代码转译为C/C++代码
2. C/C++编译,即将C/C++代码编译为所需的动态链接库,可执行程序等等.

一个特殊的就是编译成python可以调用的动态链接库模块,由于这个过程都是使用`Cython.Build.Cythonize`实现的,我称其为`Cythonize`编译

本部分是针对正文中cython编译使用方面的补充,主要编译使用方法在正文部分会有介绍.

## 转译Cython代码

虽然英文都是`compile`,但明显从Cython到C/C++的过程和从Cython到动态链接库的过程是两回事,这边我统一称呼从Cython到C/C++的过程为`转译`.

Cython无论用在哪里,怎么用,都必须先将Cython源码转译为C/C++源码.而控制转译行为的被称为转译指令(`Compiler directives`)

### 转译指令

转译指令指的是发送给底层编译器的指令,用于控制一些转译的行为,包括如下:

+ `binding`,bool,默认为`True`,用于控制自由函数的行为是否更像Python的自带的C实现函数(例如`len()`),或者当设置为True时函数将在作为类属性查找时绑定到实例,并将模拟Python函数的属性,包括参数名称和注释等内省.

+ `boundscheck`,bool,默认为`True`,用于控制是否执行边界检查,如果设置为False,Cython可以自由地在代码中进行索引操作(也就是`[i]`操作)而将不会导致任何IndexErrors被抛出.只有当索引为非负数(或者转译指令`wraparound`为False)时列表,元组和字符串才会受影响该选项影响.不建议设置为False,容易造成存储器区段错误.

+ `wraparound`,bool,默认为`True`,用于控制是否处理负索引.Python支持负索引但C并不支持,如果设置为False则Cython既不检查也不正确处理负索引,这容易造成存储器区段错误.但需要注意设置为True并不意味着Cython支持负索引,它仅仅是会进行检查并不代表它可以让C部分支持负索引.

+ `initializedcheck`,bool,默认`True`,用于控制元素访问或分配内存视图时是否检查它是否被初始化.如果转译指令`cpp_locals`处于打开状态,则也会控制C++类是否初始化.

+ `nonecheck`,bool,默认`False`,用于控制是否检查None值,如果设置为`False`,Cython则认为对变量类型的本地字段访问为扩展类型,或者当变量被设为None时对缓冲区变量的缓冲区访问永远不会发.。否则插入一个检查并引发适当的异常

+ `overflowcheck`,bool,默认为`False`,用于控制是否执行溢出检查.如果设置为True,当溢出的C整数算术运算上引发了异常时会执行适度的运行时惩罚.

+ `overflowcheck.fold`,bool,默认为`True`,在转译指令`overflowcheck`为True时生效,如果设置为True则会仅检查嵌套的溢出位和有副作用的算术表达式而不是每个步骤都检查.依赖于不同的编译器,体系结构和优化设置,这项选项可能有助于提高性能也可能降低性能.

+ `embedsignature`,bool,默认`False`,用于控制是否将cython函数签名嵌入编译好的python对象的docstring中.

+ `embedsignature.format`,枚举(`c`/`python`/`clinic`),默认`c`,仅在转译指令`embedsignature`为True时生效,用于设置签名样式,其中
    + `c`,保留C类型声明和Python类型注释的签名
    + `python`,尽量在签名中使用纯python类型的注释,对于没有Python类型注释的参数,C类型映射到最接近的Python类型(比如short会映射为python中的int)
    + `clinic`,与[Argument clinic](https://github.com/DOsinga/argument-clinic)工具所理解的签名兼容的类型

+ `cdivision`,bool,默认`False`,用于控制是否检不测除0错误.如果设置为`False`,Cython将调整余数和商值运算符C类型以匹配Python的int类型并且当右操作数为0时产生ZeroDivisionError.这将会有超过35％的性能损失;如果设置为`True`则不执行任何检查。
        
+ `cdivision_warnings`,bool,默认`False`,在转译指令`cdivision`为True时生效,控制出现除0情况时是否发出运行时警告
        
+ `cpow`,bool,默认`False`,用于控制`a**b`的行为是否使用c语言行为.下表是开关不同状态下的行为描述

    `a`的类型|`b`的类型|`cpow==True`|`cpow==False`
    ---|---|---|---
    C中的整型|负整型|返回C中的double型|返回C中的double型
    C中的整型|>=0的C中整型|返回C中的整型|返回C中的整型
    C中的整型|C中的整型|返回C中的整型|返回C中的double型
    C中的浮点类型|C中的整型|返回C中的浮点类型|返回C中的浮点类型
    C中的浮点类型或整型|C中的浮点类型|返回C中的浮点类型,当结果为复数时返回结果为NaN|返回C中的浮点类型或复数

+ `always_allow_keywords`,bool,默认`True`,控制在构造零或一个参数的函数或方法时`METH_NOARGS`和`METH_O`是否置空.这是一个优化项,对具有多个参数的特殊方法和函数没有影响.
`METH_NOARGS`和`METH_O`签名提供了更快的调用约定但不允许使用关键字参数.

+ `c_api_binop_methods`,bool,默认`False`,用于控制二进制操作比如`__add__`等的行为是否根据低级C-API槽的语义执行(即只有一个方法实现正常运算符和反向运算符).

+ `profile`,bool,默认`False`,用于控制是否在转译为C代码的时候带上pyhton性能分析的钩子

+ `linetrace`,bool,默认`False`,用于控制是否在转译为C代码的时候带上分析器或覆盖率报告的跟踪钩子.注意,除非将C宏定义`CYTHON_TRACE=1`额外传递给C编译器,否则生成的模块实际上不会使用行追踪.定义CYTHON_TRACE_NOGIL=1也包括NOGIL函数和部分

+ `infer_types`,bool,默认`None`,用于控制是否对函数体中未声明类型的的变量进行类型推断.默认值表示只允许安全(语义上不变的)推断.类型推断并不安全因此不推荐设置为`True`


+ `language_level`,枚举(`2`/`3`/`3str`),用于全局设置使用的python语言级别.默认为Python3和Python2的兼容类型,推荐如果不打算支持python2则设置为`3`,打算支持可以设置为`3str`.`3str`选项和`3`的区别在于`3str`会启用Python3语义,但当编译的代码在Python 2.x中运行时不会将str类型和未固定的字符串文字更改为unicode.

+ `c_string_type`,枚举(`bytes`/`str`/`unicode`),默认为`bytes`,用于控制与`char *`或`std :: string`隐式转换的python类型
    
+ `c_string_encoding`,str,(`ascii`,`utf-8`,...),默认为`utf-8`.用于控制与`char *`或`std :: string`隐式转换时的文本编码
               
+ `type_version_tag`,bool,默认`True`,用于控制是否开启自定义扩展类型的属性缓存.通过设置类型标志`Py_TPFLAGS_HAVE_VERSION_TAG`可以在CPython中启用扩展类型的属性缓存.在类型需要在内部与其`tp_dict`进行协调而不关注缓存一致性的罕见情况下需要禁用该选项

+ `unraisable_tracebacks`,bool,默认`False`,用于控制是否在抑制不可取消的异常时打印回溯


+ `iterable_coroutine`,bool,默认`True`.用于控制是否遵循[PEP492](https://peps.python.org/pep-0492/)规定异步定义协同程序必须不可迭代的规范.PEP492规定异步定义协同程序必须不可迭代,以防止在非异步上下文中意外误用.然而这使得编写向后兼容的代码变得困难且效率低下,这些代码在Cython中使用`async def`定义,但需要与使用旧语法`yield`的异步Python代码交互.例如Python 3.5之前的asyncio.该指令可以应用于模块中也可以选择性地作为装饰器应用于异步定义协同程序,以使受影响的协同程序可迭代从而直接与`yield from`互操作.

+ `annotation_typing`,bool,默认`True`.控制是否允许使用`typehints`声明类型.

+ `emit_code_comments`,bool,默认`True`.控制是否将cython源码逐行复制到生成的代码文件中的C代码注释中以帮助理解输出.这也是覆盖率分析所必需的.

+ `cpp_locals`,bool,默认`False`,控制让C++变量的行为更像Python变量,允许它们"未绑定",而不是总是默认在函数开始时构造它们.`cpp_cals`是一个实验性功能,它使C++变量的行为类似于普通的Python对象变量.有了这个指令它们只在第一次赋值时初始化,因此不再需要堆栈分配null构造函数.尝试访问未初始化的C++变量将以与Python变量相同的方式生成UnboundLocalError(或类似的东西),比如:

    ```cython
    def function(dont_write):
        cdef SomeCppClass c  # not initialized
        if dont_write:
            return c.some_cpp_function()  # UnboundLocalError
        else:
            c = SomeCppClass(...)  # initialized
            return c.some_cpp_function()  # OK
    ```

    此外该编译器选项避免在分配临时C++对象之前初始化这些对象,因为Cython需要在自己的代码生成中使用这些对象(通常用于可能引发异常的函数的返回值).为了提高速度,`initializedcheck`指令禁用了对未绑定本地的检查.启用此指令后访问未初始化的变量将触发未定义的行为,用户需要自行避免此类访问.

    `cpp_cals`目前使用`std:：optional`实现,因此需要一个兼容C++17的编译器.定义`CYTON_USE_BOOST_OPTIONAL`可以让C++编译器改用`boost::optional`(但更具实验性和未经测试).由于需要存储和检查跟踪变量是否初始化的布尔值,该指令可能会带来内存和性能成本,但C++编译器在大多数情况下应该能够消除检查

### 如何设置转译指令

转译指令的设置有2种方式:

1. 在转译或编译时通过参数传入,比如命令行工具`cython`/`cythonize`调用时的`-X`参数,比如:
    `cython -X boundscheck=False xxx.pyx`
2. 在注释中声明转译指令,使用`# cython: 转译指令`的方式,比如:
    ```cython
    # cython: boundscheck=False
    for i in range(100):
        # 这里的数组访问不会进行边界检查
        my_list[i] = i
    ```
        
需要注意这两者并不是重复功能,而是有分工的下面是3条规则

+ `参数设置中的转译指令`是全局影响的,也就是说只要设置了就全局有效
+ `注释设置中的转译指令`根据注释位置的不同可以影响局部行为也可以影响全局行为
    + 在源文件顶部的代表该转译指令影响全局
    + 在函数上面代表转译指令影响该函数;在代码块上面代表转译指令影响该代码块在表达式上面代表仅表达式有影响.
+ 参数设置中的转译指令和注释设置中的转译指令同时存在时全局设置会以`参数设置中的转译指令`>`源文件顶部的转译指令`>默认值的顺序进行覆盖,而局部设置不会受影响

习惯上我们会在第一个文件的头部注释好全局转译指令,避免参数设置转译指令造成行为混乱.

## `Cythonize`编译

`Cythonize`编译本质上就是`cython`转译后再使用C/C++编译器编译,只是cythonize将这个过程封装了成了一个步骤.`Cythonize`编译会在如下场景中被用到:

1. 使用`cythonize`工具编译动态链接库作为python模块
2. 使用`setuptools`在`setup.py`中编译动态链接库并作为python模块发布
3. 在python中使用`pyximport`直接加载cython源码文件作为模块,它没有用到`cythonize`工具而是单独实现了一个简化版,不过原理相似因此统一归为`Cythonize`编译介绍
4. 在python中直接使用`cython.inline(code)`执行内嵌的cython源码文本,它没有用到`cythonize`工具而是单独实现了一个简化版,不过原理相似因此统一归为`Cythonize`编译介绍
5. 在python中使用装饰器`@cython.compile`装饰一个python函数使其作为cython函数被处理,它没有用到`cythonize`工具而是单独实现了一个简化版,不过原理相似因此统一归为`Cythonize`编译介绍
6. 在jupyter中使用魔法命令`%%cython`执行cython源码,它没有用到`cythonize`工具而是单独实现了一个简化版,不过原理相似因此统一归为`Cythonize`编译介绍

需要注意`Cythonize`编译是将`.pyx`或`.py`源文件转换成**python模块**.因此它也遵从模块的规范.在Cython中模块的范围可以是单个的`.pyx`或`.py`源文件也可以是包含`__init__.pxd`的文件夹,因此以`cythonize`工具编译为例,我们可以指定用于编译的目标有两种:

+ `.pyx`或`.py`为后缀的源文件
+ 包含`__init__.pxd`的文件夹

Cython的模块化编程我们会在后面的章节中详细介绍.

`Cythonize`编译除了有上面的转译指令用于控制转译时的行为外,还可以通过编译链接指令来控制编译链接时的行为.这些行为是实际是[setuptools.Extension](https://setuptools.pypa.io/en/latest/userguide/ext_modules.html#extension-api-reference)在处理.Cythonize编译时会将这些编译链接指令传递给`setuptools.Extension`处理进行编译操作.

如何使用`cythonize`工具以及如何在jupyter中使用魔法命令`%%cython`我们在前面的工具链部分已经有介绍了这边就不多赘述;使用`setuptools`在`setup.py`中编译动态链接库并作为python模块发布则是后面<使用Cython做扩展>部分的重点之一,此处也不多赘述.本文将主要介绍参数设置和剩下几个在python运行时编译cython代码的接口.

### 编译链接指令

`Cythonize`支持的编译链接指令(也就是`setuptools.Extension`支持的编译链接指令)如下:

+ `language`,`(str)`,编译使用的编译语言,Cythonize中默认是`c`,可以指定为`c++`使用C++特性

+ `sources`,`list[str]`, 指定需要一起编译的额外源文件

+ `include_dirs`,`(list[str])`,需要用到的C/C++头文件存放目录,相当与gcc中的`-I`

+ `library_dirs`,`(list[str])`,需要用到的动态/静态链接库的存放目录,相当与gcc中的`-L`

+ `libraries`,`(list[str])`,需要用到的动态/静态链接库的名字,相当与gcc中的`-l`

+ `define_macros`,`(list[tuple[str, str|None]])`,要定义的宏列表.每个宏都是使用一个2元组定义的,其中第一个项对应于宏的名称,第二个项要么是有值的字符串,要么是定义它时没有特定值的`None`

+ `undef_macros`,`(list[str])`,要取消定义的宏列表

+ `runtime_library_dirs`,`(list[str])`,在运行时搜索C/C++库的路径列表.windows上会报错

+ `extra_objects`,`(list[str])`,要链接的额外文件的列表

+ `extra_compile_args`,`(list[str])`,编译时要使用的任何额外的平台和编译器特定信息参数.

+ `extra_link_args`,`(list[str])`,链接时要使用的任何额外的平台和编译器特定信息参数.

+ `export_symbols`,`(list[str])`,要从共享扩展导出的符号列表.不是在所有平台上都使用,通常也不是Python扩展所必需的.Python扩展通常只导出一个符号--"init"+扩展名

+ `depends`,`(list[str])`,编译扩展依赖的文件列表

+ `py_limited_api`,`(bool)`,是否使用python的限制api.


### 编译链接时会生效的环境变量

由于底层还是使用c/c++编译器,所以对应的环境变量一样生效,主要是:

+ `CC`/`CXX`,指定C/C++的编译器
+ `CFLAGS`/`CPPFLAGS`,指定C/C++编译器的选项,比如`-fPIC`,`-o`...
+ `LDSHARED`,指定用于产生最终共享对象的命令
+ `LDFLAGS`,指定链接参数和库位置等

### 如何设置编译链接指令

编译链接指令最通用的设置方式自然就是使用**环境变量**,然后是在**文件顶部注释**中设置,比如`# distutils: language=c++`,然后就是不同的场景下会有不同的**参数方法**设置这些编译链接指令.下面是总结:

场景|参数设置方法
---|---
使用`cythonize`工具编译动态链接库作为python模块|---
使用`setuptools`在`setup.py`中编译动态链接库并作为python模块发布|在`setuptools.Extension()`中设置
在python中使用`pyximport`直接加载cython源码文件作为模块|---
在python中直接使用`cython.inline(code)`执行内嵌的cython源码文本|---
在python中使用装饰器`@cython.compile`装饰一个python函数使其作为cython函数被处理|---
在jupyter中使用魔法命令`%%cython`执行cython源码|在`%%cython`同行作为参数设置,但注意它只支持部分参数且参数是gcc的样式


### 常用的编译链接指令

+ `cython 3.0.2`在mac os下亲测会出现`warning: code will never be executed`这样的警告,应该是对clang编译器的支持不够好造成的,后续版本中可能会有改进,但无论如何,碰到类似警告可以在源码头部写上如下设置来关闭这种警告
    ```cython
    # distutils: extra_compile_args=-Wno-unreachable-code
    ...
    ```


## python运行时编译cython代码

cython除了是一套工具,更是一个python模块,这个模块本身提供了在python运行时直接使用cython的功能.需要注意虽然可以直接使用,但其原理并没有改变,只是将编译过程放在了运行时而已.

### 在python中使用pyximport直接加载cython源码文件作为模块

`pyximport`模块提供了一个`install()`方法用于给import操作打猴子补丁,在执行完`pyximport.install()`后,单个的cython源码文件就可以像正常python模块一样被import加载使用了.


```python
!cat cf.pyx
```

    def f(x):
        return 2.0*x


```python
import pyximport
pyximport.install()
import cf
cf.f(10)
```

    /Users/mac/micromamba/envs/py3.10/lib/python3.10/site-packages/Cython/Compiler/Main.py:381: FutureWarning: Cython directive 'language_level' not set, using '3str' for now (Py3). This has changed from earlier releases! File: /Users/mac/WORKSPACE/GITHUB/BLOG/TutorialForPythonAsGlueLanguage/附录/Cython使用简介/Cython的编译补充/cf.pyx
      tree = Parsing.p_module(s, pxd, full_module_name)





    20.0



### 在python中直接使用cython.inline(code)执行内嵌的cython源码文本

cython的`inline`函数可以直接编译cython源码文本并执行,这种方式可以将上文中的变量自动导入计算.


```python
import cython
def f(a):
    b = 3
    ret = cython.inline("return a+b")
    return ret
```


```python
f(1)
```

    Compiling /Users/mac/Library/Caches/cython/inline/_cython_inline_07627bc2b8497c5ae42b06b173445e4a8cd5c49f.pyx because it changed.
    [1/1] Cythonizing /Users/mac/Library/Caches/cython/inline/_cython_inline_07627bc2b8497c5ae42b06b173445e4a8cd5c49f.pyx


    warning: /Users/mac/Library/Caches/cython/inline/_cython_inline_07627bc2b8497c5ae42b06b173445e4a8cd5c49f.pyx:6:4: Unreachable code





    4



### 在python中使用装饰器@cython.compile装饰一个python函数使其作为cython函数被处理

装饰器`@cython.compile`可以用于装饰一个函数将其用cython编译后加速.被装饰的函数是python函数,加速仅仅是因为使用cython编译过.


```python
import cython

@cython.compile
def plus(a, b):
    return a + b
```


```python
print(plus('3', '5'))
print(plus(3, 5))
```

    Compiling /Users/mac/Library/Caches/cython/inline/_cython_inline_d56483da52ad28263ba91493427776e57630c865.pyx because it changed.
    [1/1] Cythonizing /Users/mac/Library/Caches/cython/inline/_cython_inline_d56483da52ad28263ba91493427776e57630c865.pyx


    warning: /Users/mac/Library/Caches/cython/inline/_cython_inline_d56483da52ad28263ba91493427776e57630c865.pyx:7:4: Unreachable code


    35
    Compiling /Users/mac/Library/Caches/cython/inline/_cython_inline_8d8712a7d6c8aea8715c194922045017433d7307.pyx because it changed.
    [1/1] Cythonizing /Users/mac/Library/Caches/cython/inline/_cython_inline_8d8712a7d6c8aea8715c194922045017433d7307.pyx


    warning: /Users/mac/Library/Caches/cython/inline/_cython_inline_8d8712a7d6c8aea8715c194922045017433d7307.pyx:7:4: Unreachable code


    8



```python

```
