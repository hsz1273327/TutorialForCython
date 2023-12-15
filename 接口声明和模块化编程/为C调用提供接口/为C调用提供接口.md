# 为C调用提供接口

python原生提供了[C语言的API](https://docs.python.org/zh-cn/3/c-api/index.html),Cython可以很好的作为媒介让Python和C程序互操作,为C调用提供接口.

C调用一般被称为`嵌入(Embedding Python)`可以分为两种:

1. C中调用Python模块.C高性能,python适合快速开发,这适合那种需要性能但一些对性能要求没那么高的逻辑部分需要经常变更需求的场景.比如说我们希望有一个高性能接口包装一个接口固定的算法,但这个算法需要快速实现快速迭代.这种场景的特点是仅需要在C中可以调用Python模块进行数据传递即可,python写的业务逻辑是固定的嵌入在C程序中的.而Cython在其中可以替代Python的作用,仅仅一个编译操作,牺牲一点点灵活和易用性性提高python模块的性能.

2. 在C程序中嵌入python解释器,让python脚本可以从外部传入改变C程序中的对象.这通常在大型项目中才会用到,相当于让python作为程序的内置脚本,通常还需要提供几个内置对象供python处理.而Cython在其中可以扮演关键性的封装内部对象的角色.

在阅读本篇之前建议先去看下我写的[C/C++攻略中与Python交互的部分](https://blog.hszofficial.site/TutorialForCLang/#/%E4%B8%8EPython%E4%BA%A4%E4%BA%92/README)

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

## 在C程序中嵌入python解释器

这部分对应[C/C++攻略中的C中C程序中嵌入python解释器部分](https://blog.hszofficial.site/TutorialForCLang/#/%E4%B8%8EPython%E4%BA%A4%E4%BA%92/C%E7%A8%8B%E5%BA%8F%E4%B8%AD%E5%B5%8C%E5%85%A5python%E8%A7%A3%E9%87%8A%E5%99%A8/C%E7%A8%8B%E5%BA%8F%E4%B8%AD%E5%B5%8C%E5%85%A5python%E8%A7%A3%E9%87%8A%E5%99%A8)

cython在其中的作用基本可以认为就是其中[用C构造Python模块](https://blog.hszofficial.site/TutorialForCLang/#/%E4%B8%8EPython%E4%BA%A4%E4%BA%92/C%E7%A8%8B%E5%BA%8F%E4%B8%AD%E5%B5%8C%E5%85%A5python%E8%A7%A3%E9%87%8A%E5%99%A8/C%E7%A8%8B%E5%BA%8F%E4%B8%AD%E5%B5%8C%E5%85%A5python%E8%A7%A3%E9%87%8A%E5%99%A8?id=%e7%94%a8c%e6%9e%84%e9%80%a0python%e6%a8%a1%e5%9d%97)的部分.毕竟Cython写python模块是专业的.
