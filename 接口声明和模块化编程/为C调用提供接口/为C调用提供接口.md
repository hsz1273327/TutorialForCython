# 为C调用提供接口

python原生提供了[C语言的API](https://docs.python.org/zh-cn/3/c-api/index.html),Cython可以很好的作为媒介让Python和C程序互操作,为C调用提供接口.

C调用一般被称为`嵌入(Embedding Python)`可以分为两种:

1. C中调用Python模块.C高性能,python适合快速开发,这适合那种需要性能但一些对性能要求没那么高的逻辑部分需要经常变更需求的场景.比如说我们希望有一个高性能接口包装一个接口固定的算法,但这个算法需要快速实现快速迭代.这种场景的特点是仅需要在C中可以调用Python模块进行数据传递即可,python写的业务逻辑是固定的嵌入在C程序中的.而Cython在其中可以替代Python的作用,仅仅一个编译操作,牺牲一点点灵活和易用性性提高python模块的性能.

2. 在C程序中嵌入python解释器,让python脚本可以从外部传入改变C程序中的对象.这通常在大型项目中才会用到,相当于让python作为程序的内置脚本,通常还需要提供几个内置对象供python处理.而Cython在其中可以扮演关键性的封装内部对象的角色.

在阅读本篇之前建议先去看下我写的[C/C++攻略中与Python交互的部分](https://blog.hszofficial.site/TutorialForCLang/#/%E4%B8%8EPython%E4%BA%A4%E4%BA%92/README)


## 在C程序中嵌入python解释器

这部分对应[C/C++攻略中的C中C程序中嵌入python解释器部分](https://blog.hszofficial.site/TutorialForCLang/#/%E4%B8%8EPython%E4%BA%A4%E4%BA%92/C%E7%A8%8B%E5%BA%8F%E4%B8%AD%E5%B5%8C%E5%85%A5python%E8%A7%A3%E9%87%8A%E5%99%A8/C%E7%A8%8B%E5%BA%8F%E4%B8%AD%E5%B5%8C%E5%85%A5python%E8%A7%A3%E9%87%8A%E5%99%A8)

cython在其中的作用基本可以认为就是其中[用C构造Python模块](https://blog.hszofficial.site/TutorialForCLang/#/%E4%B8%8EPython%E4%BA%A4%E4%BA%92/C%E7%A8%8B%E5%BA%8F%E4%B8%AD%E5%B5%8C%E5%85%A5python%E8%A7%A3%E9%87%8A%E5%99%A8/C%E7%A8%8B%E5%BA%8F%E4%B8%AD%E5%B5%8C%E5%85%A5python%E8%A7%A3%E9%87%8A%E5%99%A8?id=%e7%94%a8c%e6%9e%84%e9%80%a0python%e6%a8%a1%e5%9d%97)的部分.毕竟Cython写python模块是专业的.


我们把模块定义的部分抽出来:

+ `emb.pyx`

```cyhton
# distutils: language = c++
cdef int numargs = 0

cdef int get_numargsc():
    # global numargs
    return numargs


def get_numargs():
    # global numargs
    return numargs

cdef void set_numargsc(int i):
    global numargs
    numargs = i

def set_numargs(int i):
    cdef int _i = i
    set_numargsc(_i)
```

+ `emb.pxd`

```cython
cdef int numargs
cdef public int get_numargsc()
cdef public void set_numargsc(int i)
```

### public关键字

申明了函数`int get_numargsc()`和`void set_numargsc(int i)`为`public`方法,这样在转译`emb.pyx`时会同步创建一个名为`emb.h`的头文件,其中就会有`int get_numargsc()`和`void set_numargsc(int i)`的声明,在C代码中就可以直接调用了.`public`修饰用于声明希望暴露出来给C语言程序调用的东西,这个东西可以是全局变量,结构体,枚举,联合,函数.也就是只要是cython可以定义的c类型都可以用它修饰.我们把`int numargs`设为`public`也可以让我们的主程序直接访问这个变量,这边仅仅是为了和C语言攻略中对应的例子有所区别就用函数进行演示.

只要转译的cython代码声明中有`public`声明,使用`cython`或`cythonize`或使用`setup.py`转译都会生成模块的同名头文件.这个头文件中会有模块的初始化函数,在本例中就是`PyMODINIT_FUNC PyInit_emb(void)`.有了这个我们就可以沿用`C/C++攻略中的C中C程序中嵌入python解释器部分`中介绍的方法将模块挂载为内置模块,然后`import`后直接使用了

## C中调用Cython模块

这部分对应[C/C++攻略中的C中调用Python模块](https://blog.hszofficial.site/TutorialForCLang/#/%E4%B8%8EPython%E4%BA%A4%E4%BA%92/C%E4%B8%AD%E8%B0%83%E7%94%A8Python%E6%A8%A1%E5%9D%97/C%E4%B8%AD%E8%B0%83%E7%94%A8Python%E6%A8%A1%E5%9D%97).

在这种场景下Cython模块干的事其实还是python干的事,依然使用的是针对python的接口.因此步骤是:

1. 用cython写一个python模块
2. 将python模块安装到C程序嵌入的python解释器设置的环境中(最好设置虚拟环境)

我们以在一个C程序中调用上个例子[example-cython-package](https://github.com/hsz1273327/example-cython-package)
第一步怎么做在前文中已经有介绍,这边就只给出第二步中C程序的入口和调用部分.例子在[ccallcy](https://github.com/hsz1273327/TutorialForCython/tree/master/%E6%8E%A5%E5%8F%A3%E5%A3%B0%E6%98%8E%E5%92%8C%E6%A8%A1%E5%9D%97%E5%8C%96%E7%BC%96%E7%A8%8B/%E4%B8%BAC%E8%B0%83%E7%94%A8%E6%8F%90%E4%BE%9B%E6%8E%A5%E5%8F%A3/ccallcy)

我们在程序之外要做的事包括:

1. 创建一个虚拟环境用来装之前例子中的包
2. 激活这个虚拟环境,安装好这个包,为了便于演示,我已经将这个包的编译结果挂在了[github上](https://github.com/hsz1273327/example-cython-package/releases/tag/v0.0.1).可以根据自己的python版本和运行平台自行挑选下载.下载好后我们在虚拟环境下使用`pip install wheel文件路径`来安装.

在C程序内,我们需要做的事包括:

1. 初始化python解释器
2. 加载模型
3. 在模型中执行如下业务逻辑

    ```python
    from binary_vector import Vector
    v1 = Vector.new(1,2)
    v1.mod()
    ```

4. 结束程序前回收Python解释器

这之中只有具体业务逻辑部分和`C/C++攻略中的C中调用Python模块`部分不同,请去那里先看下,这边就只讲不同的地方了.
它对应的C++代码可以写成

实现上面的业务逻辑代码如下

```C++
void call_mod(PyObject* pModule) {
    // 在模块中找到类名为`PyVector`的类,将这个类对象提出来
    const char* Claz_Name = "Vector";
    auto pClaz = PyObject_GetAttrString(pModule, Claz_Name);
    auto guard_pClaz = sg::make_scope_guard([&pClaz]() noexcept {
        Py_XDECREF(pClaz);  // 释放对象pName的gc计数器
        printf("Py_XDECREF(pClaz) ok\n");
    });
    if (pClaz) {
        const char* Claz_new_Name = "new";
        auto pClaz_new = PyObject_GetAttrString(pClaz, Claz_new_Name);
        auto guard_pClaz_new = sg::make_scope_guard([&pClaz_new]() noexcept {
            Py_XDECREF(pClaz_new);  // 释放对象pName的gc计数器
            printf("Py_XDECREF(pClaz_new) ok\n");
        });
        /* pClaz is a new reference */
        if (pClaz_new && PyCallable_Check(pClaz_new)) {
            // pClaz存在且为可调用的类对象,则执行调用
            // 构造参数args
            auto x = PyLong_FromLong(1);
            auto y = PyLong_FromLong(2);
            auto args = PyTuple_Pack(2, x, y);
            auto guard_pClaz_new_args = sg::make_scope_guard([&args, &x, &y]() noexcept {
                // 回收args参数
                Py_DECREF(args);
                Py_DECREF(x);
                Py_DECREF(y);
                printf("Py_DECREF pClaz_new (args) ok\n");
            });

            // 调用函数对象
            auto pObj = PyObject_Call(pClaz_new, args, NULL);
            auto guard_pObj = sg::make_scope_guard([&pObj]() noexcept {
                Py_XDECREF(pObj);  // 释放对象pName的gc计数器
                printf("Py_XDECREF(pObj) ok\n");
            });
            // 提取结果数据
            if (pObj != NULL) {
                const char* Method_Name = "mod";
                auto pMethod = PyObject_GetAttrString(pObj, Method_Name);
                auto guard_pMethod = sg::make_scope_guard([&pMethod]() noexcept {
                    Py_XDECREF(pMethod);  // 释放对象pName的gc计数器
                    printf("Py_XDECREF(pMethod) ok\n");
                });

                if (pMethod && PyCallable_Check(pMethod)) {
                    auto margs = PyTuple_New(0);
                    auto guard_pMethod_args = sg::make_scope_guard([&margs]() noexcept {
                        // 回收args参数
                        Py_DECREF(margs);
                        printf("Py_DECREF pMethod (args) ok\n");
                    });
                    auto result = PyObject_Call(pMethod, margs, NULL);
                    auto guard_pMethod_result = sg::make_scope_guard([&result]() noexcept {
                        // 回收result参数
                        Py_XDECREF(result);
                        printf("Py_DECREF pMethod (result) ok");
                    });
                    if (result != NULL) {
                        auto result_double = PyFloat_AsDouble(result);
                        printf("Result of call: %f\n", result_double);
                    } else {
                        PyErr_Print();
                        throw AppException(std::format("Call {} . {} failed", Claz_Name, Method_Name).c_str());
                    }
                } else {
                    if (PyErr_Occurred()) {
                        PyErr_Print();  // 捕获错误,并打印
                    }
                    throw AppException(std::format("Cannot find method {} . {}", Claz_Name, Method_Name).c_str());
                }
            } else {
                PyErr_Print();
                throw AppException(std::format("Call {} failed", Claz_Name).c_str());
            }
        } else {
            if (PyErr_Occurred()) {
                PyErr_Print();  // 捕获错误,并打印
            }
            throw AppException(std::format("Cannot find class new method {}", Claz_new_Name).c_str());
        }

    } else {
        if (PyErr_Occurred()) {
            PyErr_Print();  // 捕获错误,并打印
        }
        throw AppException(std::format("Cannot find class {}", Claz_Name).c_str());
    }
}
```

需要注意虽然我们的Cython写的代码可以摆脱GIL限制,但这也仅限于纯C部分,在`C中调用Cython模块`这种场景下由于必然经过python解释器,所以在并发等情况下我们依然需要使用GIL限制资源

### C中调用Cython模块的C方法

上面的写法看起来就累,Cython能不能简化我们的调用呢?可以,这也是为什么这篇文章和对应的C攻略中的顺序相反的原因.

Cython除了可以定义python对象也可以定义C对象,其中C函数就可以使用关键字`api`声明后暴露给python解释器.同时会在转译的时候创建一个名为`模块名_api.h`的头文件用于导入.被声明为`api`的C函数在使用特制的函数`import_模块名();`模块后就可以像正常的C函数一样被调用了.

需要注意的点有如下几个:

1. `api`只能修饰`cfunc`(`cdef`)定义的函数
2. 如果声明为`api`的函数参数或返回值为特定c结构,需要将这个c结构用`public`修饰出来让C程序可以访问调用
3. 仅需要导入`模块名_api.h`头文件,不要重复导入`模块名.h`
4. 比如有个模块是某个模块的子模块`foo.spam`,那它转译出来的头文件应该是`foo.spam_api.h`,导入的函数名则是`import_foo__spam()`
5. 允许你在多个动态链接库中使用`import_模块名()`,每处都需要先调用这个函数再使用定义的c接口.
6. 依然需要在调用`import_模块名()`之前先初始化python解释器,在调用完后执行python解释器的释放操作.

这个例子在[delorean](https://github.com/hsz1273327/TutorialForCython/tree/master/%E6%8E%A5%E5%8F%A3%E5%A3%B0%E6%98%8E%E5%92%8C%E6%A8%A1%E5%9D%97%E5%8C%96%E7%BC%96%E7%A8%8B/%E4%B8%BAC%E8%B0%83%E7%94%A8%E6%8F%90%E4%BE%9B%E6%8E%A5%E5%8F%A3/delorean),大致如下:

1. 写模块实现,由于这个模块并不需要python调用,我们就纯写个c的

   + `delorean.pyx`

       ```cython
       cdef public struct Vehicle:
           int speed
           float power

       cdef api int activate(Vehicle *v) except *:
           if v.speed >= 88 and v.power >= 1.21:
               print("Time travel achieved")
               return 1
           else:
               return 0
       ```

2. 用`setup.py`方式编译它为二进制`wheel`模块.这个过程中,转译生成的`delorean_api.h`以及其他头文件,c实现文件也会被整体打包到模块中.
3. 将这个二进制`wheel`模块安装到C程序将可以加载模块的环境中,这个操作也会将`delorean_api.h`以及其他头文件,c实现文件放到环境中.
4. 构造我们的程序入口

   + `main.cpp`

    ```C++
    #define PY_SSIZE_T_CLEAN
    #include <Python.h>
    #include "delorean_api.h"
    ...

    Vehicle car; //Vehicle是public出来的结构
    int main(int argc, char* argv[]) {
        try {
            // 初始化
            init_py(argv[0], (char*)"env/", NULL, false);
            import_delorean(); //导入模块
            // 开始执行python调用
            car.speed = atoi(argv[1]); 
            car.power = atof(argv[2]);
            int x = activate(&car); //调用cfunc,获取结果
            if (PyErr_Occurred()) {
                PyErr_Print();  // 捕获错误,并打印
            }
            printf("get result %d\n", x);
        } catch (const AppException& ex) {
            fprintf(stderr, ex.what());
            return 1;
        }
        // 回收python解释器
        return finalize_py();
    }
    ```

    需要注意`cfunc`名实际是一个宏,很多开发工具很难跟踪到它的参数,返回值类型信息,我们得自己注意着调用.

5. 编译程序,在模块安装后会被放置在`<环境>/lib/python3.10/site-packages/<模块名>`中.这个路径需要加入到头文件的搜索目录,即`-I <环境>/lib/python3.10/site-packages/<模块名>`.

之后我们就可以验证这个例子了:

```bash
>>> ./ccallcy 1 2.5
python init config clear
get result 0
>>> ./ccallcy 111 22.5
python init config clear
Time travel achieved
get result 1
```
