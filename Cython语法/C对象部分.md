# C对象部分

由于不用于python解释器对接,C/C++对象部分负责的是将Cython语法尽可能的映射为C语法的代码,也就是说这部分可以没有python的事儿的.

但并不是说这部分就不能有python对象,相反的这部分的c函数可以接收返回python对象.但个人并不推荐这样做.保持C对象部分的纯净只有好处没有坏处.

Cython转换成的代码总体而言还是C代码,在实现方面C++部分也仅是有少量支持,我们能将它看做是C++的平替.


```python
%load_ext cython
```

## C/C++变量类型声明

需要声明变量类型的就两个场景:

+ 函数/方法签名中声明参数和返回值,比如

    ```cython
    cdef int func1(int a, int b)
    ```
    
+ 代码块中声明变量类型,使用`cdef`进行声明,比如:

    ```cython
    cdef int x,y,z
    ```

C/C++变量是可以在Python部分声明的,这就涉及到两边类型自动转化和一些限制的问题,我们会在Python对象部分进行介绍.这部分我们仅考虑在C/C++对象部分的用法和行为

### 类型限定符

cython中支持使用[类型限定符(Type qualifier)](https://learn.microsoft.com/zh-cn/cpp/c-language/type-qualifiers?view=msvc-170)对变量进行约束,支持的类型限定符有:

+ `const`,声明变量不可修改
+ `volatile`声明变量的值可由超出该变量所在的程序控制范围的某个值(如并发执行的线程)合理更改.

其修饰方法就是在类型前面增加限定符,比如:

```cython
cdef volatile int a = 5 # volatile限定符的用法示例

cdef const int sum(const int a, const int b): # const限定符的正常用法,
    return a + b
```

### 指针和引用

cython同样也支持指针,和C/C++中类似,其写法就是在类型后,变量名前增加一个星号`*`


```cython
cdef int func1(const int a, int *b)

cdef int *a = 5
```

当于`const`限定符结合使用时需要注意每个部分的放置顺序

```cython
cdef void print_const_pointer(const int *value): # value为指向常量int的指针
    print(value[0])

cdef void print_pointer_to_const_value(int * const value):# value为指向int的常量指针
    print(value[0])

cdef void print_const_pointer_to_const_value(const int * const value): # value为指向常量int的常量指针
    print(value[0])
```

类似的如果我们使用C++编译,则也可以支持引用,但在实现部分,个人建议还是不用为妙.

```cython
# distutils: language = c++

cdef int i = 17
cdef int &r = i;
```

#### 取地址和解引用

cython中取地址和C中一样,都是使用`&`符号,但解引用需要使用cython库中的`cython.operator.dereference`


```cython
%%cython -c=-Wno-unreachable-code

from cython.operator cimport dereference

cdef int x = 42
cdef int* ptr = &x  # 创建一个指向整数 x 的指针

cdef int y = dereference(ptr)
print(y)
```

    42


### 类型反射

cython中我们可以使用`cython.typeof`方法来反射C/C++类型变量的类型.

```cython
from cython cimport typeof
...

cdef int my_int = 42
print(typeof(my_int)) # >>> int
cdef object my_int = 42
print(typeof(my_int)) # >>> Python object
```

### 类型转换

在cython中使用`<xxx>yyy`操作符来进行类型转换,其使用方式与C中类似.

```cython
cdef char *p, float *q
p = <char*>q

```

值得注意的是cython中python的`bool`类型会转化为`bint`,而python中的自定义类的实例则对应的`object`

### 类型检测

和C中类似,类型转换时使用`<xxx?>yyy`会先进行检测

### 类型别名

Cython中可以使用`ctypedef`关键字为C类型取别名

```cython
ctypedef unsigned int uint_t
```

### python对象类型

python对象本质上也是C对象,因此也可以使用`cdef`来声明,所有的python对象都可以使用`object`来表示类型,`object`代表的就是pyhton的最基础类型`Object`.

```cython
cdef object pyechofunc(object x):
    return x
```

如果一个参数不被声明类型则默认当做被声明为`object`来处理.也就是说上面的例子可以去掉类型声明简写为:

```cython
cdef pyechofunc(x):
    return x
```

如果我们想使用python对象的引用作为参数,我们就必须导入`cpython.ref.PyObject`并使用`PyObject *`作为参数类型

```cython
from cpython.ref cimport PyObject
...

cdef borrowed_reference(PyObject * obj):
    refcount = obj.ob_refcnt
    print('Inside borrowed_reference: {refcount}'.format(refcount=refcount))
```

#### python中的容器类型声明

`list`, `dict`也可以用于申明类型,他们就是python中对应的类型,但需要注意,这类声明并不能限定其中的内容类型.

```cython
cdef list alist = []
```

比较特殊的是`tuple`类型,它需要声明其中的数据类型,但它并不需要声明自己:

```cython
cdef (int,double) atuple
```

就表示`atuple`的类型为`tuple[int, double]`.这一语法被称为`ctuple`


### C++中stl容器的声明

如果我们使用的是C++编译器,那么我们可以使用stl中的容器作为类型,cython已经为其做好了封装,我们只要使用即可.

大多数C++标准库的容器已在位于`/Cython /Includes/libcpp`的pxd文件中声明.这些容器是：

+ `deque`双向队列
+ `list`列表
+ `map`映射
+ `pair` 对
+ `queue`队列
+ `set`集合
+ `stack`栈
+ `vector`向量
+ `string`字符串

因此要用这些容器只需简单的cimport进来即可.很多结构是可迭代的,我们可以用熟悉的`for...in...`语法对其进行遍历.

我们知道stl中的容器都是模版,很多使用时需要先指定类型参数进行实例化,cython中通过`模版名[类型]`的方式可以实现.以vector为例


```cython
%%cython -c=-Wno-unreachable-code
# distutils: language = c++

from libcpp.vector cimport vector

cdef vector[int] vect
cdef int i, x

for i in range(10):
    vect.push_back(i)

for i in range(10):
    print(vect[i])

for x in vect:
    print(x)
```

    0
    1
    2
    3
    4
    5
    6
    7
    8
    9
    0
    1
    2
    3
    4
    5
    6
    7
    8
    9


### [`*`]对C++类对象的支持

在实现部分我们不能定义C++类,但如果已经有声明包装了一个外部的C++类,我们可以对其进行实例化,删除实例和调用操作

```cython
# distutils: language = c++

from Rectangle cimport Rectangle

cdef Rectangle *rec  # 在栈上声明

def main():
    rec_ptr = new Rectangle(1, 2, 3, 4)# 在堆上创建
    try:
        rec_area = rec_ptr.getArea()
    finally:
        del rec_ptr # 在堆上删除
```

需要注意在纯净模式下我们无法`new`一个C++类的

## C自定义结构声明

Cython支持对C中的结构体,联合,枚举的声明

### 声明结构体


```cython
%%cython
# distutils: extra_compile_args=-Wno-unreachable-code
cdef struct Grail:
    int age
    float volume
    
cdef Grail grail = Grail(12,100)
print(f"{grail.age};{grail.volume}")
```

    12;100.0


### 声明联合体


```cython
%%cython
# distutils: extra_compile_args=-Wno-unreachable-code
cdef union Food:
    char *spam
    float *eggs

cdef float *arr = [1.0, 2.0]
cdef Food spam = Food(spam='b')
cdef Food eggs = Food(eggs=arr)
print(spam.spam, eggs.eggs[0])
```

    b'b' 1.0


### 声明枚举


```cython
%%cython
# distutils: extra_compile_args=-Wno-unreachable-code
cdef enum CheeseType:
    cheddar, 
    edam,
    camembert

cdef enum CheeseState:
    hard = 1
    soft = 2
    runny = 3

print(CheeseType.cheddar)
print(CheeseState.hard)
```

    0
    1


## Cython提供的运算符

Cython提供了以下运算符,他们有的是Python中本来就有的,在Cython中增加了针对C/C++不分的能力;有的则是C/C++中的东西,包括:

+ 数据计算运算符,包括`++`,`--`,`+=`,`-=`,`*=`,`/=`,`%%`,`+`,`-`,`*`,`/`.他们和python中一致,只是在C/C++中只能用于数值

+ 判断运算符,包括`>`,`<`,`>=`,`<=`,`==`,`!=`,`is`,`not`,`in`,他们和python中一致,只是在C/C++中现在也可以用

+ 取地址`&`,C/C++中的操作


```cython
%%cython
# distutils: extra_compile_args=-Wno-unreachable-code

from cython cimport typeof
cdef object my_int = 42
print(typeof(my_int))
```

    Python object


## C函数声明

Cython中申明C的可调用对象依然使用`cdef`,但需要注意这种函数只能在同模块中被访问,而是无法在python解释器中访问.在构造这种函数时有如下注意事项

1. 函数内部的变量必须申明.
2. 函数不会默认返回`None`,如果你想反回空值需要在函数申明时将返回值声明为`void`
3. 异常必须声明标志量


C函数声明的完整语法为

```cython
cdef [返回值类型] 函数名(形参类型 形参名,...) [noexcept | except 异常标志量]:
```

### 异常处理

在Python中函数内部发生的异常会用信号通知调用者,并通过定义的错误返回值向上传播到调用堆栈.对于返回Python对象(以及指向该对象的指针)的函数,错误返回值只是`NULL`指针,因此任何返回Python对象的函数都有一个定义明确的错误返回值.

虽然Python函数总是这样,但C/C++函数是没有定义明确的错误返回值的,通常情况下Cython使用一个专用的返回值来表示非外部C/C++函数引发了异常.

在cython中我们处理异常的流程是:

1. 定义自定义异常
2. 在会抛出异常的的函数声明中声明异常标志量
3. 在函数中抛出异常.


```cython
%%cython
# distutils: extra_compile_args=-Wno-unreachable-code

cdef class MyCustomException(Exception):
    def __init__(self, message):
        self.message = message
        
cdef class MyCustomException2(Exception):
    def __init__(self, message):
        self.message = message
        
cdef int spam(int x) except -1:
    if x > 0:
        return x**2
    raise MyCustomException("test exception")

    
cdef void foo() except *:
    print(spam(2))
    try:
        print(spam(-2))
    except Exception as e:
        raise MyCustomException2(str(e))

    
foo()

```

    4



    ---------------------------------------------------------------------------

    MyCustomException                         Traceback (most recent call last)

    File _cython_magic_e93106904a5583639906fcfbd7e81d30204024e1.pyx:20, in _cython_magic_e93106904a5583639906fcfbd7e81d30204024e1.foo()


    File _cython_magic_e93106904a5583639906fcfbd7e81d30204024e1.pyx:14, in _cython_magic_e93106904a5583639906fcfbd7e81d30204024e1.spam()


    MyCustomException: test exception

    
    During handling of the above exception, another exception occurred:


    MyCustomException2                        Traceback (most recent call last)

    Cell In[7], line 1
    ----> 1 get_ipython().run_cell_magic('cython', '', '# distutils: extra_compile_args=-Wno-unreachable-code\n\ncdef class MyCustomException(Exception):\n    def __init__(self, message):\n        self.message = message\n        \ncdef class MyCustomException2(Exception):\n    def __init__(self, message):\n        self.message = message\n        \ncdef int spam(int x) except -1:\n    if x > 0:\n        return x**2\n    raise MyCustomException("test exception")\n\n    \ncdef void foo() except *:\n    print(spam(2))\n    try:\n        print(spam(-2))\n    except Exception as e:\n        raise MyCustomException2(str(e))\n\n    \nfoo()\n')


    File ~/micromamba/envs/py3.10/lib/python3.10/site-packages/IPython/core/interactiveshell.py:2478, in InteractiveShell.run_cell_magic(self, magic_name, line, cell)
       2476 with self.builtin_trap:
       2477     args = (magic_arg_s, cell)
    -> 2478     result = fn(*args, **kwargs)
       2480 # The code below prevents the output from being displayed
       2481 # when using magics with decodator @output_can_be_silenced
       2482 # when the last Python token in the expression is a ';'.
       2483 if getattr(fn, magic.MAGIC_OUTPUT_CAN_BE_SILENCED, False):


    File ~/micromamba/envs/py3.10/lib/python3.10/site-packages/Cython/Build/IpythonMagic.py:359, in CythonMagics.cython(self, line, cell)
        356 # Build seems ok, but we might still want to show any warnings that occurred
        357 print_compiler_output(get_stdout(), get_stderr(), sys.stdout)
    --> 359 module = load_dynamic(module_name, module_path)
        360 self._import_all(module)
        362 if args.annotate:


    File ~/micromamba/envs/py3.10/lib/python3.10/site-packages/Cython/Build/Inline.py:51, in load_dynamic(name, path)
         49 spec = importlib.util.spec_from_file_location(name, loader=ExtensionFileLoader(name, path))
         50 module = importlib.util.module_from_spec(spec)
    ---> 51 spec.loader.exec_module(module)
         52 return module


    File <frozen importlib._bootstrap_external>:1184, in exec_module(self, module)


    File <frozen importlib._bootstrap>:241, in _call_with_frames_removed(f, *args, **kwds)


    File _cython_magic_e93106904a5583639906fcfbd7e81d30204024e1.pyx:25, in init _cython_magic_e93106904a5583639906fcfbd7e81d30204024e1()


    File _cython_magic_e93106904a5583639906fcfbd7e81d30204024e1.pyx:22, in _cython_magic_e93106904a5583639906fcfbd7e81d30204024e1.foo()


    MyCustomException2: test exception


#### 声明异常标志量

异常标志量并不是给我们看的,而是告诉转译器的,在C中并没有所谓的异常,我们需要人为的预先定义一个返回值来告诉转译器,如果返回的值是这个值时就是异常了.

异常标志量的类型可以是`int`,`enum`,`float`或者`指针`.也就是说以这4类为返回值类型的就可以标识一个明确的异常标志量来判定函数是否有异常.

异常标志量应该是明确不会作为正常值返回的值,比如上例中平方计算的值怎么都不会是-1.但如果我们实在找不出一个值可以符合上述要求,那也可以使用`? 异常标志量`的形式进行声明.比如:

```cython
cdef int spam(int x) except? -1:
    if x > 0:
        return x**3
    raise MyCustomException("test exception")
```
这样转译器会在获得返回值为`-1`时额外多进行一次对[PyErr_Occurred()](https://docs.python.org/3/c-api/exceptions.html#c.PyErr_Occurred)的检验,只有都满足时才会抛出异常.

当返回值类型为`void`时我们就只能通过检验`PyErr_Occurred()`来判断异常了,这时我们需要设置异常标志量为`*`.

之所以搞得这么复杂其实还是为了向效率妥协,毕竟使用固定异常标志量判断的开销远低于`PyErr_Occurred()`.

最后如果你的函数确定不应该出现异常,则应当声明为`noexcept`.如果`noexcept`函数最终还是以异常结束那么它将打印一条警告消息但不会继续向下传递异常.


### [`*`]函数指针

类似C中,Cython允许声明函数指针.需要注意这个功能在纯净模式中无法实现


```cython
%%cython
# distutils: extra_compile_args=-Wno-unreachable-code

cdef int(*ptr_add)(int, int)

cdef int add(int a, int b):
    return a + b

ptr_add = add

print(ptr_add(1, 3))
```

    4


### cython的内置函数

Cython将对大多数内置函数的调用编译为对相应的`Python/C API`版本,我们可以在cython代码中的直接调用他们.

内置函数|返回类型|相当于`Python/C API`中的类型
---|---|---
`abs(obj)`|object, double, ...|PyNumber_Absolute, fabs, fabsf, ...
`callable(obj)`|bint|PyObject_Callable
`delattr(obj, name)`|None|PyObject_DelAttr
`exec(code, [glob, [loc]])`|object
`dir(obj)`|list|PyObject_Dir
`divmod(a, b)`|tuple|PyNumber_Divmod
`getattr(obj, name, [default])`|object|PyObject_GetAttr
`hasattr(obj, name)`|bint|PyObject_HasAttr
`hash(obj)`|int/long|PyObject_Hash
`intern(obj)`|object|`Py*_InternFromString`
`isinstance(obj, type)`|bint|PyObject_IsInstance
`issubclass(obj, type)`|bint|PyObject_IsSubclass
`iter(obj, [sentinel])`|object|PyObject_GetIter
`len(obj)`|	Py_ssize_t|PyObject_Length
`pow(x, y, [z])`|object|PyNumber_Power
`reload(obj)`|object|PyImport_ReloadModule
`repr(obj)`|object|PyObject_Repr
`setattr(obj, name)`|void|PyObject_SetAttr

除了这些外,还有一些C/C++中的内置函数,他们也被Cython所支持

内置函数|返回类型|说明
---|---|---
`sizeof(type)`|int|获取类型的占用字节数
