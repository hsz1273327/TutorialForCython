# C对象部分

C对象部分在纯净模式中使用装饰器和typehints类型进行标注.当直接在python解释器中执行时他们就是python对象,而经过编译后他们就会映射为C语法的代码.

但并不是说这部分内部就不能有python对象,相反的这部分的c函数可以接收返回python对象.但个人并不推荐这样做.保持C对象部分的纯净只有好处没有坏处.

Cython转换成的代码总体而言还是C代码,在实现方面C++部分也仅是有少量支持,我们能将它看做是C++的平替.


```python
%load_ext cython
```

## C/C++变量类型声明

纯净模式下的C/C++变量可以使用cython模块下的特定typehints类型来指定.下面是对照表:

cython模块中的类型|c/c++中类型|说明
---|---|---
`cython.char`|`char`| 8-bit|单字符类型,表示`ASCII码`对应编码(0~127) 
`cython.uchar`|`unsigned char`|8-bit|不带符号的char类型,用于将字符作为数处理,范围`0~255`
`cython.schar`|`signed char`|8-bit|带符号的char类型,用于将字符作为数处理,范围`-127~127`
`cython.short`/`cython.sshort`|`short`/`short int`/`signed short`/`signed short int`|16-bit|带符号的短整型数,范围`−32,767~+32,767`
`cython.ushort`|`unsigned short`/`unsigned short int`|16-bit|不带符号短整型数,范围`0~65,535`
`cython.int`/`cython.sint`|`int`/`signed int`/`signed`|16-bit|带符号基础整型数,范围`−32,767~+32,767`
`cython.uint`|`unsigned int`/`unsigned`|16-bit|不带符号基础整型数,范围`0~65,535`
`cython.long`/`cython.slong`|`long`/`signed long`/`long int`/`signed long int`|32-bit|带符号长整型数,范围`−2,147,483,647~+2,147,483,647`
`cython.ulong`|`cython.ulong`/`unsigned long int`|32-bit|不带符号长整型数,范围`0~4,294,967,295`
`cython.longlong`/`cython.slonglong`|`long long`/`signed long long`/`long long int`/`signed long long int`|64-bit|带符号超长整型数,范围`−9,223,372,036,854,775,807~+9,223,372,036,854,775,807`
`cython.ulonglong`|`unsigned long long`/`unsigned long long int`|64-bit|不带符号超长整型数,范围`0, 18,446,744,073,709,551,615`
`cython.float`|`float`|16-bit(取决于平台)|单精度浮点数
`cython.double`|`double`|32-bit(取决于平台)|双精度浮点数
`cython.longdouble`|`long double`|64-bit(取决于平台)|扩展精度浮点数
`cython.floatcomplex`|`<complex.h>->float complex`|32-bit(取决于平台)|每一位都是单精度浮点数的复数
`cython.complex`/`cython.doublecomplex`|`<complex.h>->double complex`|64-bit(取决于平台)|每一位都是双精度浮点数的复数
`cython.longdoublecomplex`|`<complex.h>->long double complex`|128-bit(取决于平台)|每一位都是扩展精度浮点数的复数
`cython.size_t`|`size_t`|随实现不同不同|表示一个对象的大小的类型
`cython.bint`|`int`|16-bit|布尔类型,`0`值为`False`其余为`True`
`cython.void`|`void`|---|仅用作参数或返回值,表示空值
`cython.Py_tss_t`|`<Python.h>->Py_tss_t`|int(16-bit)+unsigned long(32-bit)|python线程内本地存储的标识
`cython.Py_UNICODE`|`<Python.h>->Py_UNICODE`|---|python的uncode类型
`cython.Py_UCS4`|`<Python.h>->Py_UCS4`|32-bits|python的uncode单个字符
`cython.Py_ssize_t`|`<Python.h>->Py_ssize_t`|随实现不同不同|python中符号化了的`size_t`
`cython.Py_hash_t`|`<Python.h>->Py_hash_t`|随实现不同不同|同`Py_ssize_t`


需要声明变量类型的就两个场景:

+ 函数/方法签名中声明参数和返回值,比如

    ```python
    import cython
    ...
    @cython.cfunc
    def func1(a: cython.int, b cython.int)->cython.int:
        pass
    ```
    
    旧式写法为
    
    ```pyhton
    import cython
    ...
    @cython.cfunc
    @cython.returns(cython.int)
    @cython.locals(a=cython.int, b=cython.int)
    def func1(a, b):
        pass
    ```
    需要注意装饰器顺序不能乱.
    一般来说更推荐第一种使用typehints的写法,更加简洁.
    
+ 代码块中声明变量类型,使用`cdef`进行声明,比如:

    ```python
    import cython
    ...
    x: cython.int
    y: cython.int
    z: cython.int = 1
    ```

    旧式写法为
    
    ```pyhton
    import cython
    ...
    x = cython.declare(cython.int)
    y = cython.declare(cython.int)
    z = cython.declare(cython.int, 1)
    ```
    
    需要注意,typehints写法仅在函数内或类内生效,模块顶层无效,而其中的内容就是旧式写法中`cython.declare()`的第一个参数
    
typehints写法有如下几种不支持:

+ `typing.Optional[any_type]`

+ `typing.List[any_type]`等其他容器,可以直接使用`list`等容器名,但不可以指定其中的类型

+ `typing.ClassVar[...]`

C/C++变量是可以在Python部分声明的,这就涉及到两边类型自动转化和一些限制的问题,我们会在Python对象部分进行介绍.这部分我们仅考虑在C/C++对象部分的用法和行为

### [`*`]类型限定符

纯净模式不支持,`const`修饰符在很多上下文中都不可用,因为Cython需要分别生成定义和它们的赋值.如必须使用建议使用在声明文件中指定.

### [`+`]指针和引用

纯净模式下同样也支持指针,其实现使用函数`cython.pointer()`


```cython
import cython
...
@cython.cfunc
def func1(a: cython.int ,b: cython.pointer(cython.int))->cython.int:
    pass

a= cython.declare(cython.pointer(cython.int),5)
```

由于纯净模式不支持`const`,指针作为参数时也就不支持与`const`的配合使用了

需要注意cython也有`NULL`,一般仅用作指针初始化.

#### 取地址和解引用

纯净模式中取地址使用函数`address(val)`,解引用需要使用cython库中的`cython.operator.dereference`


```cython
%%cython -c=-Wno-unreachable-code
import cython
if cython.compiled:
    print("Yep, I'm compiled.")
    from cython.operator import dereference

    x = cython.declare(cython.int,42)
    ptr=  cython.declare(cython.pointer(cython.int),cython.address(x))  # 创建一个指向整数 x 的指针
    y = cython.declare(cython.int,dereference(ptr))
    print(y)
else:
    print("Just a lowly interpreted script.")
```

    Yep, I'm compiled.
    42


### 类型反射

cython中我们可以使用`cython.typeof`方法来反射C/C++类型变量的类型.

```cython
import cython
from cython import typeof
...
my_int = cython.declare(cython.int,42)
print(cython.typeof(my_int)) # >>> int
my_int_obj = cython.declare(object,42)
print(cython.typeof(my_int_obj)) # >>> int
```

### 类型转换

在cython中使用函数`cython.cast(类型,变量)`来进行类型转换.

```cython
p = cython.declare(cython.pointer(cython.char),'a')
q = cython.declare(cython.pointer(cython.int))
q = cython.cast(cython.pointer(cython.int),p)
```

值得注意的是cython中python的`bool`类型会转化为`bint`,而python中的自定义类的实例则对应的`object`

### 类型检测

类型转换时使用`cython.cast(类型,变量,typecheck=True)`会先进行检测,再执行类型转换

### 类型别名

Cython中可以使用`ctypedef`关键字为C类型取别名

```cython
uint_t = cython.typedef(cython.uint)
```

### python对象类型

python对象本质上也是C对象,因此也可以使用`cdef`来声明,所有的python对象都可以使用`object`来表示类型,`object`代表的就是pyhton的最基础类型`Object`.

```cython
@cython.cfunc
def pyechofunc(x: object)->object:
    return x
```

如果一个参数不被声明类型则默认当做被声明为`object`来处理.也就是说上面的例子可以去掉类型声明简写为:

```cython
@cython.cfunc
def pyechofunc(x):
    return x
```

如果我们想使用python对象的引用作为参数,我们就必须导入`cpython.ref.PyObject`并使用`cython.pointer(PyObject)`作为参数类型

```cython
from cython.cimports.cpython.ref import PyObject
...

@cython.cfunc
def borrowed_reference(obj: cython.pointer(PyObject)):
    refcount = obj.ob_refcnt
    print('Inside borrowed_reference: {refcount}'.format(refcount=refcount))
```

#### python中的容器类型声明

`list`, `dict`也可以用于申明类型,他们就是python中对应的类型,但需要注意,这类声明并不能限定其中的内容类型.

```cython
alist = cython.declare(list,[])
```

比较特殊的是`tuple`类型,它还需要声明其中的数据类型:

```cython
atuple = cython.declare(tuple[cython.int, cython.double])
```

就表示`atuple`的类型为`tuple[int, double]`.这一语法被称为`ctuple`

### array声明

定长array使用`元素类型[长度]`的形式声明,不定长array则使用一个固定类型指针`pointer(元素类型)`来声明

```cython
x = cython.declare(cython.int[10][5])
y = cython.declare(cython.pointer(cython.int))
```


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
import cython
from cython.cimports.libcpp.vector import vector

vect = cython.declare(vector[int])
i = cython.declare(int)
x = cython.declare(int)

for i in range(10):
    vect.push_back(i)

for i in range(10):
    print(vect[i])

for x in vect:
    print(x)
```

    Content of stderr:
    ld: warning: dylib (/usr/local/Cellar/gcc/13.1.0/lib/gcc/current/libstdc++.dylib) was built for newer macOS version (11.0) than being linked (10.9)0
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


### [`+`]对C++类对象的支持

在实现部分我们不能定义C++类,但如果已经有声明包装了一个外部的C++类,我们可以对其进行实例化,删除实例和调用操作

```cython
# distutils: language = c++

from cython.cimports.Rectangle cimport Rectangle

rec  = cython.declare(cython.pointer(Rectangle)) # 在栈上声明

```
需要注意在纯净模式下我们无法`new`和`delete`在堆上分配处理一个C++类的的实例

## C自定义结构声明

Cython支持对C中的结构体,联合,枚举的声明

### 声明结构体


```cython
%%cython
# distutils: extra_compile_args=-Wno-unreachable-code
    
import cython
Grail = cython.struct(
    age=cython.int,
    volume=cython.float)
grail = cython.declare(Grail,Grail(12,100))
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

import cython
Food = cython.union(
    spam=cython.p_char,
    eggs=cython.p_float)

arr = cython.declare(cython.pointer(float),[1.0,2.0])
spam = cython.declare(Food,Food(spam='b'))
eggs = cython.declare(Food,Food(eggs=arr))
print(spam.spam, eggs.eggs[0])
```

    b'b' 1.0


### [`*`]声明枚举

纯净模式暂时不支持枚举

## Cython提供的运算符

Cython提供了以下运算符,他们有的是Python中本来就有的,在Cython中增加了针对C/C++不分的能力;有的则是C/C++中的东西,包括:

+ 数据计算运算符,包括`++`,`--`,`+=`,`-=`,`*=`,`/=`,`%%`,`+`,`-`,`*`,`/`.他们和python中一致,只是在C/C++中只能用于数值

+ 判断运算符,包括`>`,`<`,`>=`,`<=`,`==`,`!=`,`is`,`not`,`in`,他们和python中一致,只是在C/C++中现在也可以用

+ 取地址`&`,在纯净模式中必须使用`cython.address`来实现

+ 取对象字节数`sizeof`,在纯净模式中必须使用`cython.sizeof`来实现

+ 取对象类型`typeof`,在纯净模式中必须使用`cython.typeof`来实现

+ 类型转换`<T>t`/<T?>t,在纯净模式中必须使用`cython.cast(T,t[,typecheck=True])`来实现



```cython
%%cython
# distutils: extra_compile_args=-Wno-unreachable-code
import cython
from cython import typeof
my_int = cython.declare(object,42)
print(typeof(my_int))
```

    Python object


## C函数声明

纯净模式中使用装饰器`@cython.cfunc`来声明一个函数是c函数.需要注意这种函数只能在同模块中被访问,且无法在python解释器中访问.在构造这种函数时有如下注意事项

1. 函数内部的变量必须申明.
2. 函数不会默认返回`None`,如果你想反回空值需要在函数申明时将返回值声明为`void`
3. 异常必须声明标志量


C函数声明的完整语法通常为

```cython
@cython.cfunc
@cython.exceptval(异常标志量)
def 函数名(形参名: 形参类型,...)->[返回值类型]:
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
import cython

@cython.cclass
class MyCustomException(Exception):
    def __init__(self, message):
        self.message = message
@cython.cclass    
class MyCustomException2(Exception):
    def __init__(self, message):
        self.message = message

@cython.cfunc
@cython.exceptval(-1)
def spam(x:cython.int) -> cython.int:
    if x > 0:
        return x**2
    raise MyCustomException("test exception")

@cython.cfunc
@cython.exceptval(check=True) 
def foo() ->cython.void:
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

    File _cython_magic_98af45db6e3381eca1045d1eae6f4101067a0fa7.pyx:25, in _cython_magic_98af45db6e3381eca1045d1eae6f4101067a0fa7.foo()


    File _cython_magic_98af45db6e3381eca1045d1eae6f4101067a0fa7.pyx:18, in _cython_magic_98af45db6e3381eca1045d1eae6f4101067a0fa7.spam()


    MyCustomException: test exception

    
    During handling of the above exception, another exception occurred:


    MyCustomException2                        Traceback (most recent call last)

    Cell In[18], line 1
    ----> 1 get_ipython().run_cell_magic('cython', '', '# distutils: extra_compile_args=-Wno-unreachable-code\nimport cython\n\n@cython.cclass\nclass MyCustomException(Exception):\n    def __init__(self, message):\n        self.message = message\n@cython.cclass    \nclass MyCustomException2(Exception):\n    def __init__(self, message):\n        self.message = message\n\n@cython.cfunc\n@cython.exceptval(-1)\ndef spam(x:cython.int) -> cython.int:\n    if x > 0:\n        return x**2\n    raise MyCustomException("test exception")\n\n@cython.cfunc\n@cython.exceptval(check=True) \ndef foo() ->cython.void:\n    print(spam(2))\n    try:\n        print(spam(-2))\n    except Exception as e:\n        raise MyCustomException2(str(e))\n\n    \nfoo()\n')


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


    File _cython_magic_98af45db6e3381eca1045d1eae6f4101067a0fa7.pyx:30, in init _cython_magic_98af45db6e3381eca1045d1eae6f4101067a0fa7()


    File _cython_magic_98af45db6e3381eca1045d1eae6f4101067a0fa7.pyx:27, in _cython_magic_98af45db6e3381eca1045d1eae6f4101067a0fa7.foo()


    MyCustomException2: test exception


#### 声明异常标志量

异常标志量并不是给我们看的,而是告诉转译器的,在C中并没有所谓的异常,我们需要人为的预先定义一个返回值来告诉转译器,如果返回的值是这个值时就是异常了.

异常标志量的类型可以是`int`,`enum`,`float`或者`指针`.也就是说以这4类为返回值类型的就可以标识一个明确的异常标志量来判定函数是否有异常.在纯净模式下使用装饰器`@cython.exceptval()`来指定标质量.

异常标志量应该是明确不会作为正常值返回的值,比如上例中平方计算的值怎么都不会是`-1`.但如果我们实在找不出一个值可以符合上述要求,那也可以使用`@cython.exceptval(异常标质量, check=True)`的形式进行声明.比如:

```cython
@cython.cfunc
@cython.exceptval(-1,check=True) 
def spam(x: cython.int) -> cython.int:
    if x > 0:
        return x**3
    raise MyCustomException("test exception")
```

这样转译器会在获得返回值为`-1`时额外多进行一次对[PyErr_Occurred()](https://docs.python.org/3/c-api/exceptions.html#c.PyErr_Occurred)的检验,只有都满足时才会抛出异常.

当返回值类型为`void`时我们就只能通过检验`PyErr_Occurred()`来判断异常了,这时我们需要设置异常标志量为`@cython.exceptval(check=True)`.

之所以搞得这么复杂其实还是为了向效率妥协,毕竟使用固定异常标志量判断的开销远低于`PyErr_Occurred()`.

最后如果你的函数确定不应该出现异常,则应当声明为`@cython.exceptval(check=False)`.如果`@cython.exceptval(check=False)`函数最终还是以异常结束那么它将打印一条警告消息但不会继续向下传递异常.


+ `@cython.exceptval(check=False)`,表示c函数不应出现异常,异常将作为警告打印并且不会向下传递. 

+ `@cython.exceptval(check=True)`,当返回值为`void`,`struct`或`union`时仅通过检验`PyErr_Occurred()`来判断异常.

+ `@cython.exceptval(-1)`,当返回值为`int`,`enum`,`float`或者`指针`时找出一个必定为不会被覆盖到的异常值作为异常值

+ `@cython.exceptval(-1,check=True)`,当返回值为`int`,`enum`,`float`或者`指针`时无法找出一个必定为不会被覆盖到的异常值,仅能通过检验`PyErr_Occurred()`来判断异常.


### [`*`]函数指针

类似C中,Cython允许声明函数指针.需要注意这个功能在纯净模式中无法实现

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
