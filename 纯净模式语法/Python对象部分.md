# Python对象部分

python对象部分负责和python解释器对接,也就是针对python的接口.

接口的种类无非不过:

+ 全局变量
+ 函数
+ 类

这三种.

下面我们就来分别讨论这三种接口在纯净模式中的实现.


```python
%load_ext Cython
```

## 全局变量

全局变量接口必须是Python类型,也就是说不要用`cdef`申明,否则python解释器无法捕获.

```cython
a = cython.declare(cython.int,10) # python解释器无法识别

b = 11 # python解释器可以正常识别
```

当然了你用`b = cython.declare(cython.int,11)`也是可以的,但有点啰嗦, 个人更推荐用python的type hints声明python类型,这样接口更加明确:

```cython
b: int = 11 # python解释器可以正常识别
```

## 函数接口

函数接口的传入参数和返回值自然是python对象,在这一前提下,Cython允许通过改造内部来实现直接对python对象部分进行加速.

python对象部分的核心也就在于如何在函数/类内部尽可能的使用C/C++对象.


```cython
%%cython
# distutils: extra_compile_args=-Wno-unreachable-code
import cython

if cython.compiled:
    print("Yep, I'm compiled.")
    def func1(a:int,b:int)->int:
        a_c: cython.int = a
        b_c: cython.int = b
        result_c: cython.int = a + b
        result: int = result_c
        return result
    print(func1(1,2))
else:
    print("Just a lowly interpreted script.")

```

    Yep, I'm compiled.
    3


### 静态化参数

我们还可以通过指定函数接口的参数类型让Cython对传入的参数进行自动转化.这可以大幅提高这个函数的执行效率.还是上面的例子,我们可以简化成如下


```cython
%%cython
# distutils: extra_compile_args=-Wno-unreachable-code
import cython
if cython.compiled:
    print("Yep, I'm compiled.")
    def func1(a:cython.int ,b:cython.int)->int:
        return a+b
else:
    print("Just a lowly interpreted script.")
```

    Yep, I'm compiled.



```python
func1(1,2)
```




    3



### 类型自动转换

除了函数出入的参数,返回值,内部的赋值都可以进行自动类型转换.这一特性也是Cython的基础特性.

在大多数情况下,Python对象和C/C++值之间需要转换时,cython会对基本的数字和字符串类型等执行自动转换,对应表如下

从Python中来的类型|C中的类型|转入到Python中的类型
---|---|---
`bool`|`bint`|bool
`int`|`[unsigned] char`,`[unsigned] short`,`int`,`long`,`unsigned int`,`unsigned long`,`[unsigned] long long`|`int`
`float`|`float`,`double`,`long double`|`float`
`bytes`|`char*`,`libcpp.string`|`bytes`
`iterable`|`C array`,`std::vector`,`std::list`|`list`
`iterable`|`std::set`,`std::unordered_set`|`set`
`iterable (len 2)`|`std::pair`|`tuple (len 2)`
`mapping`|`std::map`,`std::unordered_map`|`dict`
`complex`|`std::complex`|`complex`
---|`struct`, `union`|`dict`
`numpy.dnarray`|内存视图|---
` array.array`|内存视图|---

需要注意:

1. 如果定义C/C++类型为`C array`,`std::vector`,`std::list`,`std::set`,`std::unordered_set`则输入和输出是同构容器
2. 如果定义C/C++类型为`C array`,在传入后Cython是无法知道序列长度的
3. 如果定义C/C++类型为`struct`, `union`,python无法向C中传递结构
4. python中的int是无限长的,但C/C++中对应的都是有长度限制的,就需要注意防止溢出.我们需要在使用时先预估好整型数的长度再确定申明的类型
5. 内存视图的返回类型需要自己指定,对于python部分的输出来说,通常使用`numpy.asarray(mv)`来转成`numpy.dnarray`作为返回值.

### Python和C都可调用的函数

如果我们希望定义一个函数在C和Python中都可以调用,那可以使用装饰器`@cython.ccall`.`@cython.ccall`定义的函数性能在python函数和C函数之间.

当你使用`@cython.ccall`定义一个函数或方法时`Cython`编译器会生成两个版本的函数代码:

+ 一个是C的版本,当在C部分调用时Cython会调用生成的C版本,这样性能更好;
+ 一个是Python的版本,当在Python部分调用时Cython会调用生成的Python版本,这样就获得了python部分的可见性

也正是由于多了一层判断自然就带来了复杂性,同时性能也就不及纯C函数了.

使用`@cython.ccall`定义函数的语法是python定义函数和定义C函数语法的结合:

```python
@cython.ccall
def 函数名(形参:形参类型 ,...)->返回值类型:
    pass
```

与定义C函数语法不同的是我们并不需要考虑异常标志量的问题.


```cython
%%cython
# distutils: extra_compile_args=-Wno-unreachable-code
import cython

@cython.ccall
def func2(a:cython.int,b:cython.int)->cython.int:
    return a + b

print(func2(1,2))
```

    3



```python
func2(3,4)
```




    7



## 扩展类型

Cython并不能直接定义C++中的类,但可以使用扩展类型为Python类提供扩展.方法就是在定义类的时候使用装饰器的`@cython.cclass`.

一个典型的扩展类型如下


```cython
%%cython
# distutils: extra_compile_args=-Wno-unreachable-code
import cython

@cython.cclass
class Rectangle:
    x0 = cython.declare(cython.int, visibility='public')
    y0 = cython.declare(cython.int, visibility='readonly')
    x1: cython.int
    y1: cython.int
    
    def __init__(self, x0: cython.int, y0: cython.int, x1: cython.int, y1: cython.int)->None:
        self.x0 = x0
        self.y0 = y0
        self.x1 = x1
        self.y1 = y1
    @cython.cfunc
    def _area(self)->cython.int:
        area: cython.int
        area = (self.x1 - self.x0) * (self.y1 - self.y0)
        if area < 0:
            area = -area
        return area

    def area(self):
        return self._area()
```


```python
rect = Rectangle(0, 0, 1, 1)
rect.area()
```




    1



我们在扩展类型中可以通过`cython.declare`或`typehints`声明C属性,但需要注意`typehints`方式无法声明属性的限定词;可以使用`@cython.cfunc`和C方法;也可以像正常在Python中一样定义普通的属性和方法;也可以用`@cython.ccall`声明定义Python中和C中都支持的方法.

### 属性

Cython的扩展类型中可以定义C属性,C属性必须指定静态类型.属性默认是私有的无法被Python解释器识别.我们可以使用限定词`public`或`readonly`来为其提供可见性.

上面例子中我们演示了Cython中属性的所有声明情况

+ `readonly`可以让字段被Python解释器识别,且会进行自动类型转换,但其中的数据是只读的
+ `public`可以让字段被Python解释器识别,且会进行自动类型转换,我们可以随意读取和改变其中的值


就像上面的例子

```cython
@cython.cclass
class Rectangle:
    x0 = cython.declare(cython.int, visibility='public')
    y0 = cython.declare(cython.int, visibility='readonly')
    x1: cython.int # 默认私有
    y1: cython.int # 默认私有
    ...
```


```python
print(rect.x0)
print(rect.y0)
print(rect.x1)
```

    0
    0



    ---------------------------------------------------------------------------

    AttributeError                            Traceback (most recent call last)

    Cell In[17], line 3
          1 print(rect.x0)
          2 print(rect.y0)
    ----> 3 print(rect.x1)


    AttributeError: '_cython_magic_8b7438824c6399a1cc680142c115f36726f6' object has no attribute 'x1'



```python
rect.x0 = 1
print(rect.x0)
```

    1


### 属性动态化

扩展类型中定义属性都是静态的,如果我们希望可以像python类一样可以动态的增加属性,我们可以在其中声明`__dict__`


```cython
%%cython
# distutils: extra_compile_args=-Wno-unreachable-code
import cython

@cython.cclass
class A:
    n: cython.int

    def __init__(self, int n: cython.int):
        self.n = n

@cython.cclass
class B:
    n: cython.int
    __dict__ : dict

    def __init__(self, n: cython.int):
        self.n = n

        
```


```python
a = A(10)
a.o = 1
```


    ---------------------------------------------------------------------------

    AttributeError                            Traceback (most recent call last)

    Cell In[20], line 2
          1 a = A(10)
    ----> 2 a.o = 1


    AttributeError: '_cython_magic_a9738fbe6b6724958cc4d39eccab3b8c769ae705.A' object has no attribute 'o'



```python
b = B(10)
b.o = 1
print(b.o)
```

    1


### 方法

扩展类型的方法分C方法和Python方法.和函数的规则一样--C方法性能高但Python解释器无法识别;Python方法性能低些但对Python解释器可见;还有使用`@cython.ccall`定义的方法性能介于C方法和Python方法之间,同时提供对Python解释器的可见性.

方法的定义语法也和函数基本一致,只是有如下几个注意点:

+ 支持静态方法`@staticmethod`但**不支持类方法`@classmethod`**,注意`@staticmethod`需要写在`@cython.cfunc`或`@cython.ccall`的上面
+ 静态方法`@staticmethod`通常不会是C方法

上例中`_area`是C级别的函数,不可被python解释器访问,而`area`则是Python函数,上面的例子中我们实际上用`area`封装了C方法`_area`.这么写没啥问题但比较啰嗦,更多的时候这种简单封装的写法会用`cpdef`方法来替代


```cython
%%cython
# distutils: extra_compile_args=-Wno-unreachable-code
import cython

@cython.cclass
class Rectangle2:
    x0 = cython.declare(cython.int, visibility='public')
    y0 = cython.declare(cython.int, visibility='readonly')
    x1: cython.int
    y1: cython.int
    
    
    def __init__(self, x0: cython.int, y0: cython.int, x1: cython.int, y1: cython.int)->None:
        self.x0 = x0
        self.y0 = y0
        self.x1 = x1
        self.y1 = y1
    
    @cython.ccall
    def area(self)->cython.int:
        area: cython.int
        area = (self.x1 - self.x0) * (self.y1 - self.y0)
        if area < 0:
            area = -area
        return area

```


```python
r = Rectangle2(1, 2, 3, 1)
```

### 特性

python中的特性本质上还是函数,扩展类型中依然支持,只是必须使用Python函数定义

```cython
@cython.cclass
class Spam:
    ...
    @property
    def cheese(self):
        # 只读
        ...

    @cheese.setter
    def cheese(self, value):
        # 可写
        ...

    @cheese.deleter
    def cheese(self):
        # 可删
        ...
    ...
```


### 初始化和实例化

我们知道Python中一个类实例化的过程是

1. 类方法`__new__`方法被调用,执行分配内存并返回一个实例的操作
2. 实例方法`__init__`被调用,执行实例的初始化.

Cython中额外定义了特殊方法`__cinit__`用于处理C级别的初始化.它的执行位置在`__new__`和`__init__`之间,且参数和传入当前实例`__init__`的参数完全一致.也就是说扩展类型及其子类的实例化过程是这样

1. 类方法`__new__`方法被调用,执行分配内存并返回一个实例的操作
2. 实例方法`__cinit__`被调用,执行实例的C部分初始化.
3. 实例方法`__init__`被调用,执行实例的初始化.


需要注意:

+ `__cinit__`被执行时实例已经创建且被赋了初值--C属性已初始化为`0`或`null`,Python属性也被初始化为`None`.
+ 无论什么情况`__cinit__`都只会被执行一次,且只要被定义了就会被执行.

在无继承的情况下并不复杂,但如果存在继承,我们就需要注意执行顺序了.

下面这个例子可以清晰的看到执行顺序


```cython
%%cython
# distutils: extra_compile_args=-Wno-unreachable-code

import cython

@cython.cclass
class A:
    x0 = cython.declare(cython.int, visibility='public')
    y0 = cython.declare(cython.int, visibility='readonly')
    x1: cython.int
    y1: cython.int
    
    
    def __init__(self,  x0: cython.int, y0: cython.int, x1: cython.int, y1: cython.int)->None:
        self.x0 = x0
        self.y0 = y0
        self.x1 = x1
        self.y1 = y1
        print(f"A init {x0} {x1} {y0} {y1}")
        
    def __cinit__(self,*args,**kwargs)->None:
        print(f"A cinit args {args} and kwargs {kwargs}")

@cython.cclass
class AA(A):
    
    def __init__(self, x0: cython.int, y0: cython.int, x1: cython.int, y1: cython.int)->None:
        print(f"AA init {x0} {x1} {y0} {y1}")
        super().__init__( x0+1 , y0+ 1, x1+1, y1+1)
        
    def __cinit__(self,*args,**kwargs)->None:
        print(f"AA cinit args {args} and kwargs {kwargs}")
        
class B(A):
    def __init__(self, x0:int , y0:int, x1:int, y1: int)->None:
        print(f"B init {x0} {x1} {y0} {y1}")
        super().__init__( x0+1 , y0+ 1, x1+1, y1+1)
        
```


```python
a = A(1,2,3,4)
```

    A cinit args (1, 2, 3, 4) and kwargs {}
    A init 1 3 2 4



```python
aa = AA(1,2,3,4)
```

    A cinit args (1, 2, 3, 4) and kwargs {}
    AA cinit args (1, 2, 3, 4) and kwargs {}
    AA init 1 3 2 4
    A init 2 4 3 5



```python
b = B(1,2,3,4)
```

    A cinit args (1, 2, 3, 4) and kwargs {}
    B init 1 3 2 4
    A init 2 4 3 5


在有继承的情况下我们可以总结为如下执行顺序:

1. 类方法`__new__`方法被调用,执行分配内存并返回一个实例的操作
2. 实例方法`__cinit__`被调用,执行实例的C部分初始化.实例会按父类->子类的顺序执行各级`__cinit__`.
3. 实例方法`__init__`被调用,执行实例的初始化.实例根据是否有`super().__init__`调用按子类->父类的顺序执行各级`__init__`,规则和python中一致.

#### 快速实例化

在特殊的使用场景下提高Cython扩炸类型实例化的性能有两种方法:

+ 忽略执行`__init__`直接实例化.
    + 使用条件:
        1. 扩展类型并没有python部分需要初始化

    因为Cython提供了`__cinit__`方法来初始化C部分,如果我们的扩展类型并没有python部分需要初始化那就可以忽略执行`__init__`直接实例化,这种实例化可以直接调用特殊静态方法`__new__(扩展类型,初始化参数...)`来实现.


```cython
%%cython
# distutils: extra_compile_args=-Wno-unreachable-code
import cython
from typing import Any

@cython.cclass
class Penguin:

    food = cython.declare(object, visibility='public')
    def __cinit__(self, food: Any)->None:
        self.food = food
        print("ciniting!")
 
    def __init__(self, food: Any)->None:
        print("initing!")
```


```python
penguin = Penguin("wheat") # 正常实例化
```

    ciniting!
    initing!



```python
fast_penguin = Penguin.__new__(Penguin, 'wheat') # 快速实例化
```

    ciniting!


+ 尽可能的避免内存分配

    + 使用条件: 
        1. 存在的实例数量可控
        2. 仅基类无继承
    
    可以使用装饰器`@cython.freelist(N)`为被装饰的扩展类型创建一个由N个实例组成的静态内存列表,由于内存已经被分配好了因此可以尽可能避免了代价高昂的分配步骤.在合适的场景中，这可以将对象实例化速度提高20-30%.



```cython
%%cython
# distutils: extra_compile_args=-Wno-unreachable-code
import cython
from typing import Any

@cython.freelist(8)
@cython.cclass
class Penguin:
    food: object
    def __cinit__(self, food: Any)->None:
        self.food = food
        print("ciniting!")
 
    def __init__(self, food: Any)->None:
        print("initing!")
```


```python
penguin = Penguin('fish 1')
penguin = None
penguin = Penguin('fish 2')  # 无需分配内存
```

    ciniting!
    initing!
    ciniting!
    initing!


### 析构方法

python中有[__del__](https://docs.python.org/zh-cn/3/reference/datamodel.html?highlight=__del__#object.__del__)方法用于在销毁python实例时触发.通常触发销毁实例有两种情况:

+ 调用`del 变量名`手动销毁
+ gc自动回收销毁.

无论哪种情况,在python中析构流程如下:

1. 执行实例的`__del__`方法
2. 销毁对象释放内存

与`__cinit__`对应,Cython则提供了`__dealloc __`方法用于控制C对象的删除.它的执行时间点在`__del__`方法完成之后,必定只会执行一次且只要有定义就会被执行.


实例析构过程是这样

1. 执行实例的`__del__`方法
2. 执行`__dealloc __`方法
3. 销毁对象释放内存

在无继承的情况下并不复杂,但如果存在继承,我们就需要注意执行顺序了.

下面这个例子可以清晰的看到执行顺序


```cython
%%cython
# distutils: extra_compile_args=-Wno-unreachable-code
import cython


@cython.cclass
class A:
    def __del__(self):
        print(f"A delete")
        
    def __dealloc__(self):
        print(f"A dealloc")

@cython.cclass
class AA(A):
    def __del__(self):
        print(f"AA delete")
        
    def __dealloc__(self):
        print(f"AA dealloc")
    
        
class B(A):
    def __del__(self):
        print(f"B delete")
        
        
```


```python
a = A()
aa = AA()
b = B()
```


```python
del a
```

    A delete
    A dealloc



```python
del aa
```

    AA delete
    AA dealloc
    A dealloc



```python
del b
```

    B delete
    A dealloc


在有继承的情况下我们可以总结为如下执行顺序:

1. 执行实例的`__del__`方法,只会执行最后重载的那个`__del__`方法
2. 执行`__dealloc __`方法,会按子类->父类的顺序执行各级`__dealloc __`方法,只要有定义就会执行
3. 销毁对象释放内存.

### 其他特殊方法

扩展类型几乎支持所有Python的魔术方法,这些魔术方法在Cython中被归类为*特殊方法*.上面介绍的`__init__()`,`__cinit__()`等等都是特殊方法.

支持的特殊方法可在[这里查找到](http://docs.cython.org/en/latest/src/userguide/special_methods.html#special-methods-table)


**注意**:

特殊方法必须用`def`定义而不是`cdef`或`cpdef`,这不会影响他们的性能--Python使用不同的调用约定来调用这些特殊方法.

## 继承

Cython扩展类型的继承规则如下:

1. 扩展类型不可以继承python类型
2. 扩展类型可以被另一个扩展类型继承,但只能单继承
3. 扩展类型可以被python类继承,且支持多继承


我们可以通过装饰器`@cython.final`防止被装饰的扩展类型在Python中被子类化

```cython
import cython
 
@cython.final
@cython.cclass
class Parrot:
    def done(self): pass
```

### 方法重载

在扩展类型中同一申明方式的可以相互重载,而不同申明方式的则有一套优先级:

+ `@cython.ccall`装饰的方法可以重载`@cython.cfunc`装饰的方法,而反过来就不行
+ 无装饰器的方法可以重载`@cython.ccall`装饰的方法,而反过来就不行

> 同一申明方式相互重载


```cython
%%cython
# distutils: extra_compile_args=-Wno-unreachable-code
import cython

@cython.cclass
class A:
    @cython.cfunc
    def foo(self):
        print("A")
        
@cython.cclass
class AA(A):
    @cython.cfunc
    def foo(self):
        print("AA")
    @cython.ccall
    def bar(self):
        self.foo()
        
```


```python
AA().bar()
```

    AA


> 优先级重载


```cython
%%cython
# distutils: extra_compile_args=-Wno-unreachable-code
import cython

@cython.cclass
class A:
    @cython.cfunc
    def foo(self):
        print("A")
        
@cython.cclass
class B(A):
    @cython.ccall
    def foo(self, x=None):
        print("B", x)

class C(B):
    def foo(self, x=True, int k=3):
        print("C", x, k)
```


```python
B(12).foo()
```

    B None



```python
C().foo()
```

    C True 3

