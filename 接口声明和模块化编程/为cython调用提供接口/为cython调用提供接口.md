# 为cython调用提供接口

cython虽然必须依赖python或C/C++才能真正实现功能,但作为一个编程语言,它也需要组织可以有结构.在语法部分我们也简单介绍了下`cimport`导入语法.它是cython模块化的关键.

为cython调用提供接口可以分为:


+ 声明内部接口
+ 声明外部包装接口
+ 模块调用
    + 外部模块调用
    + 内部子模块调用


```python
%load_ext cython
```

## 声明内部接口

内部接口的声明包括:

+ 声明类型别名
+ 声明C全局变量
+ 声明C函数
+ 声明扩展类型

尤其是结合纯净模式,我们可以做到渐进式的优化模块性能.

无论哪种声明,它必须与实现部分一一对应--比如有个实现`A.pyx`,那它的声明文件即为`A.pxd`,`A.pyx`中有一个函数为`cdef void func(int a)`那如果我们希望将它暴露给其他cython模块就需要在`A.pyx`中也有一个`cdef void func(int a)`,当然如果我们并不想将它暴露给其他cython模块就不要在`A.pyx`中有对应的声明.这个逻辑基本和C语言的头文件是一致的.

### 声明类型别名

Cython中我们可以使用关键词`ctypedef`来声明类型别名,其语法为`ctypedef 类型 类型别名`,比如将`unsigned int`定义为`uint_t`可以写作

+ `A.pxd`

    ```cython
    ctypedef unsigned int uint_t
    ```

`ctypedef`可以也仅可以用于任何**C类型**,例如整数,浮点数,struct,指针等.通常用于提高可读性和对模块使用场景的匹配程度.

通常别名声明既可以放在`.pxd`文件也可以放在实现部分,这取决于如下几点:

1. 你是否希望外部cython模块使用这个别名,如果希望则放在`.pxd`文件,不希望则放实现部分
2. 你的对外接口是否有用到这个别名,如果有用到则放在`.pxd`文件,没有则放实现部分

一旦你在`.pxd`文件中声明了别名,那实现部分就不需要重复声明了.

### 声明C变量

你可以直接用`cdef 类型 变量名`声明一个变量,比如

+ `A.pxd`

    ```cython
    cdef int GLOBAL_VAL
    ```

通常变量声明既可以放在`.pxd`文件也可以放在实现部分,这取决于你是否希望外部cython模块使用这个变量,如果希望则放在`.pxd`文件,不希望则放实现部分.

一旦你在.pxd文件中声明变量,那实现部分就不需要重复声明了可以直接赋值.比如上面声明文件对应的实现文件中可以直接这样赋值

+ `A.pyx`

    ```cython
    GLOBAL_VAL = 100
    ```

### 声明C函数

我们可以使用`cdef [返回值类型] 函数名(形参类型 形参名,...) [noexcept | except 异常标志量]`来声明一个C函数;`cpdef 返回值类型 函数名(形参类型 形参,...)`来声明一个Python和C都可调用的函数

需要注意的是如果参数有默认值,这个默认值在实现部分设置,但`.pxd`文件的声明中要使用`=*`来表示这个参数有默认值

+ `A.pxd`

    ```cython
    cdef uint_t mycfunc(uint_t x, uint_t y=*)

    cpdef int mycpfunc(int x, int y=*)
    ```
    
+ `A.pyx`

    ```cython
    cdef uint_t mycfunc(uint_t x, uint_t y=2):
        cdef uint_t a
        a = x-y
        return a + x * y

    cpdef int mycpfunc(int x, int y=2):
        cdef int a
        a = x+y
        return a + x * y

    ```
    
**特别需要注意的是**:python函数不能在`.pxd`中声明,**包括静态化参数的python函数**.

通常函数声明既可以放在`.pxd`文件也可以放在实现部分,这取决于你是否希望外部cython模块使用这个函数,如果希望则放在.pxd文件,不希望则放实现部分.

#### 声明内联函数

在`.pyd`文件中同样可以写内联函数.但要注意内联函数除了要声明还要在`.pyd`中有实现

```cython
cdef inline int int_min(int a, int b):
    return b if b < a else a
```

## 声明扩展类型

扩展类型中只需要且只能声明其中的C方法,Python和C都可调用的方法以及C属性,python部分不能声明.

+ `A.pxd`

```cython
cdef class A:
    cdef public int a
    cdef int b
    cdef double foo(self,double x)
    cpdef double bar(self, double x)
```

其中C属性部分在`.pxd`中声明了就不需要再在实现部分声明了,只要正常赋值即可.

+ `A.pyx`

```cython
cdef class A:
    def __init__(self, b:int = 0):
        self.a = 3
        self.b = b

    cdef double foo(self, double x):
        return x + _helper(1.0)
    
    cpdef double bar(self, double x):
        return x**2 + _helper(1.0)

    def foobar(self,x:float)->float:
        return x**2 + _helper(1.0)
```


这块的演示代码在[define_pyx](https://github.com/hsz1273327/TutorialForCython/tree/master/%E6%8E%A5%E5%8F%A3%E5%A3%B0%E6%98%8E%E5%92%8C%E6%A8%A1%E5%9D%97%E5%8C%96%E7%BC%96%E7%A8%8B/%E4%B8%BAcython%E8%B0%83%E7%94%A8%E6%8F%90%E4%BE%9B%E6%8E%A5%E5%8F%A3/define_pyx)中

### 与纯净模式结合使用

普通cython语法具有更好的表达能力,代码可读性更好功能也最全.但纯净模式与声明文件`.pyd`结合使用更加适合渐进式的性能优化工作流.我们可以先使用纯python实现,然后根据需要在对原始python代码做出极小改动的情况下使用声明文件`.pyd`将其改造为cython模块.

cython在编译时会将声明文件`.pyd`中声明的变量,函数,方法,等与源文件中进行匹配,在纯净模式下,由于有声明文件`.pyd`中的声明,即便实现文件中定义的是python函数python类,也可以被当做声明成的C函数扩展类来进行编译.


#### 纯净模式改写原例子 

我们只需要改写实现部分:

+ `A.py`

```cython
# cython: embedsignature=True
# cython: embedsignature.format=python
import cython

if cython.compiled:
    print("Yep, I'm compiled.")

else:
    print("Just a lowly interpreted script.")
    uint_t = int
    def int_min(a: int, b: int) -> int:
        return b if b < a else a

GLOBAL_VAL = 100

# uint_t = cython.typedef(cython.uint)


def mycfunc(x: uint_t, y: uint_t = 2) -> uint_t:
    a: uint_t = x - y
    return a + x * y


def mycpfunc(x: cython.int, y: cython.int = 2) -> int:
    a: cython.int = x + y
    return a + x * y


@cython.locals(x=cython.int, y=cython.int)
def mypfuncstatic(x: int, y: int) -> int:
    return x * y


def mypfunc(x: int, y: int) -> int:
    return 2 * x * y


def _helper(a: cython.double) -> float:
    return a + 1


class A:
    def __init__(self, b: int = 0):
        self.a = 3
        self.b = b

    def foo(self, x: cython.double) -> cython.double:
        return x + _helper(1.0)

    def bar(self, x: float) -> float:
        return x**2 + _helper(1.0)

    def foobar(self, x: float) -> float:
        return x**2 + _helper(1.0)
```

通常用纯净模式都是要考虑不编译情况下纯python代码的可执行性的,因此需要注意的有如下几点:

1. 声明文件中的类型别名在cython中可以被直接使用,但在python下不行,需要像例子中一样在检查到没有编译的情况下提前声明别名
2. 推荐无论是C函数,python函数还是C和python都可以调用的函数都保留typehints方便维护
3. 建议C函数,C和python都可以调用的函数在typehints中使用`cython`模块中的类型进行声明;不静态化参数的python函数在typehints中使用python;静态化参数的python函数必须在typehints中使用`cython`模块中的类型进行声明
4. 在给python写扩展的情况下建议非python接口的函数以及方法遵循python社区西关使用`_`开头命名


这块的演示代码在[define_py](https://github.com/hsz1273327/TutorialForCython/tree/master/%E6%8E%A5%E5%8F%A3%E5%A3%B0%E6%98%8E%E5%92%8C%E6%A8%A1%E5%9D%97%E5%8C%96%E7%BC%96%E7%A8%8B/%E4%B8%BAcython%E8%B0%83%E7%94%A8%E6%8F%90%E4%BE%9B%E6%8E%A5%E5%8F%A3/define_py)中

## 声明外部包装接口

cython提供了包装C/C++接口的能力.这部分仅需要在声明文件`.pxd`中定义即可.

这种模式主要作用是让python可以借助cython调用C/C++的程序.其逻辑是:

```
C/C++的源码或链接库
    |
    包装
    |
    v
cython模块
    |
    包装
    |
    v
pyhton模块

```
我们的`声明外部包装接口`做的是就是将`C/C++的源码或链接库`包装为`cython模块`这一步.

cython包装外部接口本质上是包装C/C++的头文件,使用如下语法:

```cython
cdef extern from "A.h" namespace "shapes":
    int spam_counter
    void order_spam(int tons)
```

其中

+ `cdef extern`声明这是一个外部包装
+ `from "A.h"`声明包装的是哪个头文件.
+ `namespace "shapes"`是可选的,用于声明C++头文件中声明的命名空间.
+ `:`后面的则是要声明的内容.这些内容包括:
    + 变量
    + 函数
    + 结构体/枚举/联合体
    + 别名
    + C++类
    + C++模版

### C接口声明

Cython主要还是针对的C接口,这块的支持是最稳定可靠的.

#### 声明变量

```cython
cdef extern from "A.h":
    int spam_counter
```

#### 声明函数

```cython
cdef extern from "A.h":
    void order_spam(int tons)
```

如果刚好python标准库有和头文件中同名函数可以兼容,那么也可以直接将其声明为`cpdef`,这样cython和python中就都可以直接调用了

```cython
cdef extern from "math.h":
    cpdef double sin(double x)
```

#### 声明别名

```cython
cdef extern from "A.h":
    ctypedef void* QueueValue
```

#### 声明结构体/枚举/联合体

```cython
cdef extern from "A.h":
    struct spam:
        pass
    enum color:
        pass
    union food:
        pass
```

在C头文件中声明`structs`,`union`和`enum`主要有两种方式--使用标记名或使用`typedef`。基于这些的各种组合，也有一些变化:

+ 仅标记名:

    + `C`中定义形式
    
        ```C
        struct Foo {
          ...
        };
        ```
    + `cython`中定义形式
    
        ```cython
        cdef struct Foo:
            ...
        ```

+ 仅`typedef`

    + `C`中定义形式
    
        ```C
        typedef struct {
            ...
        } Foo;
        ```
    + `cython`中定义形式
    
        ```cython
        ctypedef struct Foo:
            ...
        ```

+ 标记名 +`typedef`且不同名

    + `C`中定义形式
        ```C
        typedef struct foo {
            ...
        } Foo;
        ```
        
    + `cython`中定义形式根据
    
        ```cython
        cdef struct foo:
            ...
        ctypedef foo Foo #可选
        ```
        
        
+ 标记名 +`typedef`且同名   
    + `C`中定义形式

        ```C
        typedef struct Foo {
            ...
        } Foo;
        ```
    + `cython`中定义形式根据     
        ```cython
        cdef struct Foo:
            ...
        ```

#### 使用工具自动生成`pyd`

[python-autopxd2](https://github.com/elijahr/python-autopxd2)提供了对C语言头文件的支持,用它可以直接指定头文件生成对应的`pyd`文件.

安装使用`pip install autopxd2`

#### 例子

这部分我们复用[C语言攻略中的binary_vector](https://blog.hszofficial.site/TutorialForCLang/#/%E5%B7%A5%E5%85%B7%E9%93%BE/%E7%BC%96%E8%AF%91%E5%B7%A5%E5%85%B7%E9%93%BE/%E7%BC%96%E8%AF%91%E5%99%A8?id=%e4%be%8b%e5%ad%90-%e4%ba%8c%e7%bb%b4%e5%90%91%e9%87%8f%e6%93%8d%e4%bd%9c)这个例子.来演示如何包装.

这个例子只有一个简单的头文件

+ `binary_vector.h`

    ```C
    /* $Id$ */
    #ifndef BINARY_VECTOR_H
    #define BINARY_VECTOR_H//一般是文件名的大写 头文件结尾写上一行

    struct BINARY_VECTOR {
        float x;
        float y;
    };

    typedef struct BINARY_VECTOR *BINARY_VECTOR_P;

    BINARY_VECTOR_P VEC_new(void);
    BINARY_VECTOR_P VEC_init(float x,float y);
    void VEC_del(BINARY_VECTOR_P);

    float VEC_mod(BINARY_VECTOR_P);
    BINARY_VECTOR_P VEC_add(BINARY_VECTOR_P,BINARY_VECTOR_P);
    float VEC_mul(BINARY_VECTOR_P ,BINARY_VECTOR_P);


    #endif
    ```

它可以实现简单的二维向量结构的取模,加法和乘法操作.

实际上无论是包装源码还是静态库动态库,声明文件的写法是一样的,不同之处仅在编译过程.我们以静态库为例.

我们仅需要构造一个`.pxd`声明文件即可将外部库进行包装,但需要注意仅包装python解释器是无法使用的,如果我们的目的是将其包装为python库,那我们还需要为它额外包装一个python接口.

一般有两种方案

+ 在声明文件`.pxd`同目录下建一个同名的`.pyx`或`.py`源文件,在其中用将它封装为python可以调用的接口
+ 在这个项目外额外新建一个模块,然后调用这个包装模块将其包装为python可调用的接口.

第一种方式简单直接,但对复用有些多余,毕竟cython除了写python库也有别的用处;第二种则灵活很多,像cython的标准库也都是第二种写法.为了演示方便我们用第一种方式作为主要的演示方式.

步骤有如下:

1. 使用`python-autopxd2`自动生成`.pdx`并提供对头文件的包装

    ```bash
    autopxd ./vec/inc/binary_vector.h binary_vector.pxd
    ```
2. 编译待包装的代码,让其成为静态链接库

    ```bash
    cd vec
    gcc -c -I./inc ./src/struct_operator.c -o src/struct_operator.o
    gcc -c -I./inc ./src/binary_operator.c -o src/binary_operator.o
    gcc -c -I./inc ./src/unary_operator.c -o src/unary_operator.o
    ar crv ./lib/libvector.a src/*.o
    ```

这样,作为cython模块就已经完成了,为了验证是否可用,我们再将其包装为python模块,有如下步骤:

1. 新建同名`.pyx`文件`binary_vector.pyx`,在其中根据声明文件中对头文件的包装定义写一个包装扩展类型

    ```cython
    # distutils: extra_compile_args=-Wno-unreachable-code
    # distutils: library_dirs=vec/lib
    # distutils: libraries=vector
    # distutils: include_dirs=vec/inc

    cdef class Vector:
        @staticmethod
        cdef create(BINARY_VECTOR_P ptr):
            p = Vector()
            p.data = ptr
            return p
 
        @staticmethod
        def new(float x, float y):
            p = Vector()
            p.data = VEC_init(x, y)
            return p

        def init(self,float x, float y):
            self.data= VEC_init(x, y)
        
        cdef void init_from_point(self,BINARY_VECTOR_P ptr):
            self.data = ptr

        def __dealloc__(self):
            if self.data is not NULL:
                print(f"A dealloc")
                VEC_del(self.data)

        def mod(self)->float:
            if self.data is not NULL:
                return VEC_mod(self.data)
            raise Exception("vector not init")

        def __add__(self,other: Vector)->Vector:
            return Vector.create(VEC_add(self.data, other.data))

        def __mul__(self,other: Vector)->float:
            return VEC_mul(self.data, other.data)

    ```

2. 修改`binary_vector.pxd`,为其中增加扩展类对应的C部分声明

    ```cython
    
    ...
    cdef class Vector:
        cdef BINARY_VECTOR_P data

        @staticmethod
        cdef create(BINARY_VECTOR_P ptr)

        cdef void init_from_point(self,BINARY_VECTOR_P ptr)
    ```

3. 使用`cythonize`编译`binary_vector.pyx`

    ```bash
    cythonize -i -3 binary_vector.pyx
    ```
    
这样我们启动python后就可以直接`import binary_vector`加载模块了.

这部分的示例代码在[wrapC](https://github.com/hsz1273327/TutorialForCython/tree/master/%E6%8E%A5%E5%8F%A3%E5%A3%B0%E6%98%8E%E5%92%8C%E6%A8%A1%E5%9D%97%E5%8C%96%E7%BC%96%E7%A8%8B/%E4%B8%BAcython%E8%B0%83%E7%94%A8%E6%8F%90%E4%BE%9B%E6%8E%A5%E5%8F%A3/wrapC)

### C++接口

C++是一个相当复杂特性极多的语言,cython对它的支持远没有C充分,因此通常C++接口会被先包装一层成为C接口,然后再由cython进行包装.但Cython也还是能支持C++一定范围内的原始特性的.这主要是C++的类和模版的一部分功能.

需要注意,`autopxd`无法处理C++接口,因此对应的步骤我们必须手动写.

#### 声明C++类

对于C++类的支持,cython支持

+ 类声明
+ 方法重载
+ 运算符重载

我们以官方的例子`Rectangle`来演示

+ `inc/Rectangle.hpp`

    ```C++
    #ifndef RECTANGLE_H
    #define RECTANGLE_H

    namespace shapes {
        class Rectangle {
            public:
                int x0, y0, x1, y1;
                Rectangle();
                Rectangle(int x0, int y0, int x1, int y1);
                ~Rectangle();
                int getArea();
                void getSize(int* width, int* height);
                void move(int dx, int dy);
        };
    }
    #endif
    ```
    
+ `src/Rectangle.cpp`

    ```C++
    #include <iostream>
    #include "Rectangle.hpp"

    namespace shapes {

        // Default constructor
        Rectangle::Rectangle () {}

        // Overloaded constructor
        Rectangle::Rectangle (int x0, int y0, int x1, int y1) {
            this->x0 = x0;
            this->y0 = y0;
            this->x1 = x1;
            this->y1 = y1;
        }

        // Destructor
        Rectangle::~Rectangle () {}

        // Return the area of the rectangle
        int Rectangle::getArea () {
            return (this->x1 - this->x0) * (this->y1 - this->y0);
        }

        // Get the size of the rectangle.
        // Put the size in the pointer args
        void Rectangle::getSize (int *width, int *height) {
            (*width) = x1 - x0;
            (*height) = y1 - y0;
        }

        // Move the rectangle by dx dy
        void Rectangle::move (int dx, int dy) {
            this->x0 += dx;
            this->y0 += dy;
            this->x1 += dx;
            this->y1 += dy;
        }
    }
    ```
    
与之对应的,我们手动为其写包装的`.pxd`文件

+ `Rectangle.pxd`

    ```Cython
    cdef extern from "Rectangle.hpp" namespace "shapes":
        cdef cppclass Rectangle:
            Rectangle() except +
            Rectangle(int, int, int, int) except +
            int x0, y0, x1, y1
            int getArea()
            void getSize(int* width, int* height)
            void move(int, int)
    ```

    我们可以看到这个包装有如下几个部分:

    + 声明包装的是一个C++的类,使用关键字`cppclass`:

        ```cython
        cdef cppclass Rectangle:`
            ...
        ```
    + 包装构造函数,需要注意所有的构造函数需要指定`except +`

        ```cython
        ...
            Rectangle() except +
            Rectangle(int, int, int, int) except +
        ...
        ```

    + 声明属性,方法,运算符重载

        如果方法有重载需求(包括构造函数),可以像下面这样定义:

        ```cython
        cdef extern from "Foo.hpp":
            cdef cppclass Foo:
                Foo(int)
                Foo(bool)
                Foo(int, bool)
                Foo(int, int)    
        ```

        如果运算符有重载需求,可以像下面这样定义:

        ```cython
        cdef extern from "Foo.hpp":
            cdef cppclass Foo:
               ...
                Foo operator+(Foo)
                Foo operator-(Foo)
                int operator*(Foo)
                int operator/(int)
                int operator*(int, Foo) # allows 1*Foo()
            # nonmember operators can also be specified outside the class
            double operator/(double, Foo)

        ```

和上面C部分的例子一样,为了验证这个例子,我们再改造它的实现部分,将其包装成python可以使用的动态连接库.这个例子我们不再先编译为动态/静态连接库,而是直接从源码编译


+ `Rectangle.pyx`
    ```cython
    # distutils: language=c++
    # distutils: sources=src/Rectangle.cpp
    # distutils: include_dirs=inc

    cdef class PyRectangle:

        def __cinit__(self):
            self.c_rect = new Rectangle()

        def __init__(self, int x0, int y0, int x1, int y1):
            self.c_rect.x0 = x0
            self.c_rect.y0 = y0
            self.c_rect.x1 = x1
            self.c_rect.y1 = y1

        def __dealloc__(self):
            del self.c_rect

        def get_area(self):
            return self.c_rect.getArea()

        def get_size(self):
            cdef int width, height
            self.c_rect.getSize(&width, &height)
            return width, height

        def move(self, dx, dy):
            self.c_rect.move(dx, dy)

        # Attribute access
        @property
        def x0(self):
            return self.c_rect.x0
        @x0.setter
        def x0(self, x0):
            self.c_rect.x0 = x0

        # Attribute access
        @property
        def x1(self):
            return self.c_rect.x1
        @x1.setter
        def x1(self, x1):
            self.c_rect.x1 = x1

        # Attribute access
        @property
        def y0(self):
            return self.c_rect.y0
        @y0.setter
        def y0(self, y0):
            self.c_rect.y0 = y0

        # Attribute access
        @property
        def y1(self):
            return self.c_rect.y1
        @y1.setter
        def y1(self, y1):
            self.c_rect.y1 = y1
    ```

    可以看到我们访问C++类实例指针中的属性时也是使用`.`而非`->`,cpython中也没有`->`操作符.
    当我们设置`# distutils: language=c++`后,我们就可以在Cython中使用关键字`new`和`del`了.通常用Python包装的C++类都会把被包装的类实例放在堆上使用Cython的`__cinit__`和`__dealloc__`进行管理.

然后修改`Rectangle.pxd`,为其增加`PyRectangle`的C++部分的声明

+ `Rectangle.pxd`

    ```cython
    ...       
    cdef class PyRectangle:
        cdef Rectangle * c_rect
    ```
    
我们直接编译python库:

```bash
cythonize -i -3 Rectangle.pyx
```
这样进入python后就可以直接用`import`导入模块`Rectangle`了.

这部分的示例代码在[wrapCpp](https://github.com/hsz1273327/TutorialForCython/tree/master/%E6%8E%A5%E5%8F%A3%E5%A3%B0%E6%98%8E%E5%92%8C%E6%A8%A1%E5%9D%97%E5%8C%96%E7%BC%96%E7%A8%8B/%E4%B8%BAcython%E8%B0%83%E7%94%A8%E6%8F%90%E4%BE%9B%E6%8E%A5%E5%8F%A3/wrapCpp)

#### 声明C++模版

C++的模版让C++可以泛型编程.通常模版是纯头文件,我们将上面的例子改造成模版,然后构造一个python可以调用的但属性类型为float的类来演示

+ `inc/Rectangle.hpp`

    我们要包装的模版
    
    ```C++
    #ifndef RECTANGLE_H
    #define RECTANGLE_H

    namespace shapes {
        template <class T>
        class Rectangle {
            public:
            T x0, y0, x1, y1;
            Rectangle() {};
            Rectangle(T x0, T y0, T x1, T y1) {
                this->x0 = x0;
                this->y0 = y0;
                this->x1 = x1;
                this->y1 = y1;
                };
            ~Rectangle() {};
            T getArea() {
                return (this->x1 - this->x0) * (this->y1 - this->y0);
            };
            void getSize(T* width, T* height) {
                (*width) = x1 - x0;
                (*height) = y1 - y0;
            };
            void move(T dx, T dy) {
                this->x0 += dx;
                this->y0 += dy;
                this->x1 += dx;
                this->y1 += dy;
            };
        };
    }
    #endif

    ```
    
+ `Rectangle.pxd`

    包装声明
    
    ```cython
    cdef extern from "Rectangle.hpp" namespace "shapes":
        cdef cppclass Rectangle[T]:
            T x0
            T y0
            T x1
            T y1
            Rectangle() except +
            Rectangle(T, T, T, T) except +
            T getArea()
            void getSize(T* width, T* height)
            void move(T, T)

    ```
    将泛型定义放在类/函数的命名后面,用`[]`包裹.然后在这个类中我们就可以用这个声明的符号(类型参数)来指代特定类型
    
为了验证这个例子,我们再改造它的实现部分,将其包装成python可以使用的动态连接库.这个例子我们不再先编译为动态/静态连接库,而是直接从源码编译

+ `Rectangle.pyx`

```cython
# distutils: language=c++
# distutils: include_dirs=inc

cdef class PyRectangleFloat:

    def __cinit__(self):
        self.c_rect = new Rectangle[float]()

    def __init__(self, float x0, float y0, float x1, float y1):
        self.c_rect.x0 = x0
        self.c_rect.y0 = y0
        self.c_rect.x1 = x1
        self.c_rect.y1 = y1

    def __dealloc__(self):
        del self.c_rect

    def get_area(self):
        return self.c_rect.getArea()

    def get_size(self):
        cdef float width, height
        self.c_rect.getSize(&width, &height)
        return width, height

    def move(self, dx, dy):
        self.c_rect.move(dx, dy)

    # Attribute access
    @property
    def x0(self):
        return self.c_rect.x0
    @x0.setter
    def x0(self, x0):
        self.c_rect.x0 = x0

    # Attribute access
    @property
    def x1(self):
        return self.c_rect.x1
    @x1.setter
    def x1(self, x1):
        self.c_rect.x1 = x1

    # Attribute access
    @property
    def y0(self):
        return self.c_rect.y0
    @y0.setter
    def y0(self, y0):
        self.c_rect.y0 = y0

    # Attribute access
    @property
    def y1(self):
        return self.c_rect.y1
    @y1.setter
    def y1(self, y1):
        self.c_rect.y1 = y1
```

这个例子中我们仅使用模版实例化了一个float类型的类.模版类使用`模版类名[特定类名]`语法进行实例化.

再使用`cythonize -i -3 Rectangle.pyx`编译后就可以在python中导入使用了.

需要注意,模版是在编译时根据调用实例化的,在实例化后类型就不再可变了,也就是说模版仅是编译时可以多态不是运行时可以多态.

这部分的示例代码在[wrapTemplate](https://github.com/hsz1273327/TutorialForCython/tree/master/%E6%8E%A5%E5%8F%A3%E5%A3%B0%E6%98%8E%E5%92%8C%E6%A8%A1%E5%9D%97%E5%8C%96%E7%BC%96%E7%A8%8B/%E4%B8%BAcython%E8%B0%83%E7%94%A8%E6%8F%90%E4%BE%9B%E6%8E%A5%E5%8F%A3/wrapTemplate)

## cython的模块组织

作为cython模块,它的主要作用是让cython代码间可以模块化编程,从而实现抽象和复用,是配合`cimport`语法的的代码组织工具.比如我们可以写一个cython模块A,当在一个新的cython项目中需要用到A中定义过的C函数或扩展类型中定义的C方法时我们就可以用`cimport`语法导入这个模块并使用了.因此cython模块可以看做是作为python模块的模块化和作为C模块的模块化的前提,后两者是前者的应用.只有弄明白如何作为cython模块进行模块化才能在后两种环境下进行的下去.而cython作为一个python和C之间的桥接语言,cython模块是无法脱离python或C使用的,也就不存在编译一说.

cython模块主要是看声明文件`.pxd`,和python的模块系统非常类似,一个cython项目如果要作为cython模块必须满足两种形式中的一种:

+ 简单模块,可以是一个单独的`.pxd`
+ 复杂模块,可以是一个带`__init__.pxd`的文件夹.

cython的复杂模块允许内部继续包含子模块,以我们的例子`mymath`为例:

```txt
mymath---\
    |---__init__.pxd
    |---normalize_and_l2norm.pyx # 示例内部调用
    |---normalize_and_l2norm.pxd # 示例内部调用的声明
    |---inner---\
    |       |---__init__.pxd
    |       |---l2norm.pxd   # 有接口`l2norm`
    |       |---l2norm.pyx
    |
    |---median_along_axis0.pxd  # 有接口`_median_along_axis0`
    |---median_along_axis0.pyx
    |---normalize.pxd  # 有接口`_normalize`
    |---normalize.pyx
    |---notexist.pxd #示例有声明无实现
    ...
cythoncallmymath.pyx # 示例外部调用
cythoncallmymath.py # 示例外部调用纯净模式
```

需要注意,**复杂模块仅`cythonize`相关工具可以处理,纯转译的`cython`工具并不能识别.

复杂模块的例子可以看[mymath](https://github.com/hsz1273327/TutorialForCython/tree/master/%E6%8E%A5%E5%8F%A3%E5%A3%B0%E6%98%8E%E5%92%8C%E6%A8%A1%E5%9D%97%E5%8C%96%E7%BC%96%E7%A8%8B/%E4%B8%BAcython%E8%B0%83%E7%94%A8%E6%8F%90%E4%BE%9B%E6%8E%A5%E5%8F%A3/mymath)

## 模块调用

cython中的模块调用可以分为内部调用和外部调用.所谓内部调用是同一复杂模块中子模块间的调用,而外部调用则是不同模块间的调用.我们依然沿用上面的例子.

### 内部调用

在其中的子模块可以和python中一样,使用相对引用导入需要的接口,如果是纯净模式则还是只能老老实实绝对引用导入,比如在一个新实现文件`mymath.pyx`中要导入接口`l2norm`和`_normalize`:

+ `normalize_and_l2norm.pyx`(cython语法)

```cython
...
from .inner.l2norm cimport l2norm
from .normalize cimport _normalize
...
```

+ `normalize_and_l2norm.py`(纯净模式语法)

```python
from cython.cimports.mymath.inner.l2norm import l2norm
from cython.cimports.mymath.normalize import _normalize
```

当然了Cython中同样要避免钻石引用.

### 外部调用

对于外部调用,无论哪种语法都只能老老实实绝对调用

+ `cythoncallmymath.pyx`(cython语法)

    ```cython
    from mymath.normalize cimport _normalize
    import numpy as np


    def callmymath():
        cdef double[:] output = _normalize(np.array([1.1,2.2,3.3,4.4]))
        print(np.asarray(output))
    ```

+ `cythoncallmymath.py`(纯净模式语法)

```cython
import cython
from cython.cimports.mymath.normalize cimport _normalize
import numpy as np


def callmymath():
    output: cython.double[:] = _normalize(np.array([1.1,2.2,3.3,4.4]))
    print(np.asarray(output))
```


```python

```
