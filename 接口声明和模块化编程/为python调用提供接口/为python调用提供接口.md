# 为python调用提供接口

python调用Cython的接口实际是python调用动态链接库.python可以调用符合特定规范的动态链接库,而cython代码可以通过`cythonize`编译转换为符合这种规范的python可调用的动态链接库.

在这个基础上,我们还需要考虑分发的问题,如何组织代码,让我们的程序可以尽量的在所有平台上有机会使用.本篇的例子在[hsz1273327/example-cython-package](https://github.com/hsz1273327/example-cython-package).这是一个单独的github项目,我们用它演示整个打包过程

## Cython模块的`cythonize`编译

`Cythonize`编译出的是python可以调用的动态链接库模块,必须指定实现部分的源文件.无论是cython的简单模块还是复杂模块,编译后都是每个实现部分`.pyx`或`.py`源文件都会编译出一个python可以调用的动态链接库文件.因此可以说cython模块的编译本质上是将其中的所有实现部分源文件分别编译为python可以调用的动态链接库文件.

`cythonize`支持指定的资源可以是单一的cython实现部分源码也可以是复杂模块.

如果我们编译的是个单文件的模块,那么没什么好纠结的,比如上面的例子中的`cythoncallmymath.pyx`,编译直接使用命令`cythonize -i --3 cythoncallmymath.pyx`即可,使用时也是直接`import cythoncallmymath`即可.

而编译复杂模块比如我们上一篇的例子`mymath`,则可以直接使用`cythonize -i --3 mymath`进行编译,使用`import mymath`的方式导入.但这并不意味着这种方式就是唯一方式,实际上对于编译复杂模块有3种选择:

+ 不带`__init__.py`或`__init__.pyx`.使用`cythonize -i --3 mymath`进行编译.编译好后会长这样(以macos下为例):

    ```txt
    mymath---\
        |---__init__.pxd
        |---normalize_and_l2norm.pyx
        |---normalize_and_l2norm.pxd
        |---normalize_and_l2norm.cpp
        |---normalize_and_l2norm.cpython-310-darwin.so # 实际有用的
        |---inner---\
        |       |---__init__.pxd
        |       |---l2norm.pxd
        |       |---l2norm.pyx
        |       |---l2norm.cpp
        |       |---l2norm.cpython-310-darwin.so  # 实际有用的
        |---median_along_axis0.pxd
        |---median_along_axis0.pyx
        |---median_along_axis0.cpp
        |---median_along_axis0.cpython-310-darwin.so  # 实际有用的
        |---normalize.pxd
        |---normalize.pyx
        |---normalize.cpp
        |---normalize.cpython-310-darwin.so  # 实际有用的
        |---notexist.pxd #示例有声明无实现
    ```

    python正常模块用`__init__.py`识别,没有它就会作为[命名空间包](https://blog.hszofficial.site/TutorialForPython/#/%E8%AF%AD%E6%B3%95%E7%AF%87/%E6%A8%A1%E5%9D%97/%E5%91%BD%E5%90%8D%E7%A9%BA%E9%97%B4%E5%8C%85)处理.因此`mymath`模块这种方式下就只能作为命名空间包使用了.这虽然不影响使用,但多少会影响加载速度.

+ 带`__init__.py`或`__init__.pyx`.使用`cythonize -i --3 mymath`进行编译.编译好后会长这样(以macos下为例):

    ```txt
    mymath---\
        |---__init__.pxd
        |---__init__.pyx
        |---__init__.cpp
        |---__init__.cpython-310-darwin.so # 实际有用的
        |---normalize_and_l2norm.pyx
        |---normalize_and_l2norm.pxd
        |---normalize_and_l2norm.cpp
        |---normalize_and_l2norm.cpython-310-darwin.so # 实际有用的
        |---inner---\
        |       |---__init__.pxd
        |       |---__init__.pyx
        |       |---__init__.cpp
        |       |---__init__.cpython-310-darwin.so # 实际有用的
        |       |---l2norm.pxd
        |       |---l2norm.pyx
        |       |---l2norm.cpp
        |       |---l2norm.cpython-310-darwin.so  # 实际有用的
        |---median_along_axis0.pxd
        |---median_along_axis0.pyx
        |---median_along_axis0.cpp
        |---median_along_axis0.cpython-310-darwin.so  # 实际有用的
        |---normalize.pxd
        |---normalize.pyx
        |---normalize.cpp
        |---normalize.cpython-310-darwin.so  # 实际有用的
        |---notexist.pxd #示例有声明无实现
    ```

    因为有`__init__.cpython-310-darwin.so`存在,`mymath`模块这种方式下可以作为正常模块使用.但由于编译,原本只要几百个字节的`__init__`文件会被编译为一个100多k的动态连接库.这就有略有点浪费了.当然你也可以之后删掉换成同功能的`__init__.py`,不过如果模块构造复杂这就太麻烦了.

+ 将复杂模块作为简单模块进行编译--我们在构造模块时让模块中只有`__init__.py`是`.py`文件,其他的源文件都是`.pyx`文件,编译时指定source不再指定模块目录而是使用通配符`*`查找其中的所有`.pyx`文件进行编译,用上面的例子就是`cythonize -i --3 mymath/**/*.pyx`.编译好后会长这样(以macos下为例):
    ```txt
    mymath---\
        |---__init__.pxd
        |---__init__.py
        |---normalize_and_l2norm.pyx
        |---normalize_and_l2norm.pxd
        |---normalize_and_l2norm.cpp
        |---normalize_and_l2norm.cpython-310-darwin.so # 实际有用的
        |---inner---\
        |       |---__init__.pxd
        |       |---__init__.py
        |       |---l2norm.pxd
        |       |---l2norm.pyx
        |       |---l2norm.cpp
        |       |---l2norm.cpython-310-darwin.so  # 实际有用的
        |---median_along_axis0.pxd
        |---median_along_axis0.pyx
        |---median_along_axis0.cpp
        |---median_along_axis0.cpython-310-darwin.so  # 实际有用的
        |---normalize.pxd
        |---normalize.pyx
        |---normalize.cpp
        |---normalize.cpython-310-darwin.so  # 实际有用的
        |---notexist.pxd #示例有声明无实现
    ```

    这种方式编译好后我们的模块即可当做正常模块使用,又没有因为编译额外的`__init__`.而且由于只编译`.pyx`文件这并不会影响py文件的存在,而python也是优先加载动态链接库,因此也不会影响python模块的整体结构,非常方便从纯python项目开始渐进式改造. 个人认为这种方式是最优雅的.
    
**在我们的例子中**,项目结构如下

```txt
example-cython-package
     |
     |---binary_vector---\
     |                   |---__init__.pxd # 让别的cython代码也可以调用本库
     |                   |---__init__.py # 指定该库为一个普通的python包
     |                   |---version.py # 让安装脚本动态获取项目版本
     |                   |---vector.py  # Vector的纯python实现
     |                   |---vector.pyx # Vector的纯cython实现,C代码`binary_vector.cpp`的包装
     |                   |---vector.pxd # vector.pyx的cython接口声明
     |                   |---spdlog.pxd # vector.pyx中用的log实现,为C++库spdlog的包装
     |                   |---binary_vector.pxd # C代码头文件`binary_vector.h`的cython包装
     |                   |---binary_vector.h # `binary_vector.cpp`的头文件
     |                   |---binary_vector.cpp # Vector的C实现
     |
     |---pyproject.toml # 项目安装设置
     |---requirements.txt # 运行时的依赖
     |---setup.py # cython编译相关的设置

```

这个结构可以在没有编译时也正常使用,编译后则可以为子模块`binary_vector`提供加速.最终,在我们的正确设置下,安装的时候可以选择安装二进制wheel包直接获得编译过的模块,也可以直接安装源码包,如果安装环境下有cython可以本地编译,没有则可以直接使用纯python版本,非常灵活.
    
### 没有实现的接口

cython的模块系统仅仅只是解决代码组织问题,无论哪种形式,光有声明文件也是没用的.使用`cimport`导入cython模块必须要有对应的实现部分,这个实现部分可以是模块中源文件`Cythonize`编译后的动态链接库,也可以是外部C/C++代码或库.如果没有,在运行时导入模块时一样会报`ModuleNotFoundError`错.

## 外部包装接口的处理

在复杂模块中如果有纯外部包装接口的声明,且仅有`.pyd`声明文件,我们并不需要为它单独写实现部分,只要有子模块引用到了它,`cythonize`编译时会自动识别将其加入编译成的C/C++源码中.我们仅需要在用到的子模块编译时指定外部依赖的编译设置,可以是

+ 源码,通过`sources`指定源码路径即可
+ 链接库,通过`library_dirs`和`libraries`指定即可

外部编译个人更推荐使用注释方式,即在`.pyx`,`pxd`文件顶部使用`# distutils: `标注编译选项.主要是:

+ `# distutils: language = c++`,指定使用g++而不是gcc
+ `# distutils: sources = xxx.cpp`,指定对应的的`.c`或`.cpp`实现文件,如果你的cython模块是外部包装,且这个外部包装的c实现文件就在项目内,就需要指定.如果有多个实现文件,可以使用`# distutils: sources = A.c, B.c, C.c, ...`的形式指定
+ `# distutils: include_dirs = xxx`,指定对应的头文件查找目录.如果你的cython模块是外部包装,且这个外部包装的c实现文件就在项目内,就需要指定.如果有多个头文件,和上面一样可以使用`逗号空格`分隔的形式指定.如果你的cython模块是外部包装,但这个外部包装的实现是项目外的第三方项目,建议不要在注释中预先指定,而是在编译期指定,具体如何指定会在下一节中详细介绍
+ `# distutils: include_dirs = xxx`,指定对应的头文件查找目录.
+ `# distutils: library_dirs=vec/lib`和`# distutils: libraries=vector`,指定对应的外部包装的c实现已经编译好的动态或静态链接库信息,如果有多个和上面一样可以使用`逗号空格`分隔的形式指定,通常只有模块是外部包装且这个外部包装的c实现是外部的第三方项目的情况才会使用,这种状态建议根据导入包的用法确定怎么指定,具体如何指定会在下一节中详细介绍.
+ `# distutils: extra_compile_args = -Wc++11-extensions, -std=c++11`,指定编译项目时的设置项,一般需要根据平台,编译器设置,所以建议根据条件动态指定,具体如何指定会在下一节中详细介绍.


### 对于**包装内部包**

不同情况对应的顶部注释如下

+ C包:

    ```cython
    # distutils: include_dirs = <包中的相对路径>
    # distutils: sources = <包中的相对路径>/xxx.c
    ```

+ C++包:

    + 使用的包是纯模版,仅有头文件
    
        ```cython
        # distutils: language = c++
        # distutils: include_dirs = <包中的相对路径>
        ```
        
    + 使用的包不是纯模版,有实现源文件
    
        ```cython
        # distutils: language = c++
        # distutils: include_dirs = <包中的相对路径>
        # distutils: sources = <包中的相对路径>/xxx.cpp
        ```
    
### 对于**包装外部包**

不同情况对应的顶部注释如下

+ C包
    
    ```cython
    # distutils: libraries=vector
    ```
    `include_dirs`和`library_dirs`在编译期指定
    

+ C++包:

    + 使用的包是纯模版,仅有头文件
    
        ```cython
        # distutils: language = c++
        ```
        
        `include_dirs`在编译期指定
        
    + 使用的包不是纯模版,有编译好的链接库
    
        ```cython
        # distutils: language = c++
        # distutils: libraries=xxx
        ```
        
        `include_dirs`和`library_dirs`在编译期指定
        

### 例子分析

**在我们的例子中**Cyhton实现部分使用了两个外部包装

+ `binary_vector.pxd`,包装的是同目录下的`binary_vector.h`和`binary_vector.cpp`,因此在头部注释中我们会写上下面的内容直接指定头文件查找目录和实现文件

    ```cython
    # distutils: language = c++
    # distutils: include_dirs = binary_vector
    # distutils: sources = binary_vector/binary_vector.cpp
    ```
    
+ `spdlog.pxd`,包装的是第三方项目[gabime/spdlog](https://github.com/gabime/spdlog).这个库可以纯用头文件也可以编译后再使用,我们上点难度,在windows下不编译,其他平台则编译,这样头部注释就需要像下兼容:

    ```cython
    # distutils: language = c++
    ```

### 编译期指定依赖查找位置

如果项目没有任何第三方的C/C++依赖,那完全没必要在编译器指定依赖查找位置,直接在项目内以相对位置指定即可.但更多的时候我们是包装外部的第三方项目,这些项目的安装状态也可能非常复杂,那就需要根据各种情况动态的指定依赖位置.

通常我们可以遵循如下流程:

1. 查看指定环境变量比如`{外部包名}_INCLUDE_DIR`(依赖头文件所在的文件夹,`,`分隔),`{外部包名}_LIB_DIR`(依赖链接库所在的文件夹,`,`分隔)和`{外部包名}_EXT_LIB`(额外的依赖库名),有些时候一些库加进编译项中是可选的.为了更可控,我们认为`{外部包名}_EXT_LIB`中指定的库都是已经编译好了且可以在`{外部包名}_INCLUDE_DIR`和`{外部包名}_LIB_DIR`或在linux系统默认路径中找到的状态
2. 判断安装的系统,根据系统不同考虑使用系统库
    + linux:我们可以从系统库中查找,通常INCLUDE是`/usr/include`或`/usr/local/include`通常LIB是`/usr/lib`或`/usr/local/lib`或`/usr/lib64`或`/usr/local/lib64`,这些并不需要额外添加
    + macos:可以考虑从homebrew中查找,通常INCLUDE是`/usr/local/Cellar/{外部包名}/外部包版本/include`通常LIB是`/usr/local/Cellar/{外部包名}/外部包版本/lib`,如果执行过`brew link`操作(安装时一般都会自动执行)那么通常根据芯片不同会被链接到:
        + x86_64(Intel Macs)芯片--`/usr/local/include`(INCLUDE)和`/usr/local/lib`(LIB)
        + arm64(M1 Macs)芯片--`/opt/homebrew/include`(INCLUDE)和`/opt/homebrew/lib`(LIB)
    
    但由于homebrew本质上是用户级的,且可以通过设置改变路径,这个不建议加进去
    + window:不存在系统库
3. 如果有依赖在系统中找不到,且不是指定的额外依赖库,则下载源码安装,如何安装就要看这个外部项目的具体说明文档了,我们可以考虑放到一个默认目录下,比如`~/.cython_vendor`等,这个就可以自由发挥了.

要实现这个过程,推荐在`setup.py`中覆写`build_ext`来实现.

方法就是继承并重新实现部分[setuptools.command.build_ext](https://github.com/pypa/setuptools/blob/main/setuptools/command/build_ext.py)中的内容,通常是实现方法`build_extension`.

通常形式为

```python
...
from setuptools.command.build_ext import build_ext
...

class uvloop_build_ext(build_ext):

    def build_extension(self):
       ...
```

**在我们的例子中**,就是

```python
import os
import sys
import shutil
import logging
import subprocess
from pathlib import Path
import chardet
from setuptools import setup
from setuptools.command.build_ext import build_ext
from typing import Optional, Dict, Tuple


DEFAULT_CYTHON_VENDER = Path.home().joinpath(".cython_vendor")


def runcmd(command: str, *, cwd: Optional[Path] = None, env: Optional[Dict[str, str]] = None, visible: bool = False, fail_exit: bool = False,) -> str:
    """执行命令行命令并返回其stdout的值

    Args:
        command (str): 命令行命令
        cwd (Optional[Path]): 执行命令时的位置.Default: None
        env (Optional[Any]): 执行命令时的环境变量. Default:None
        visible (bool): 命令可见度. Default: False
        fail_exit (bool): 当执行失败时退出程序. Default: False

    Returns:
        str: stdout捕获的字符串
    """
    try:
        if visible:
            logging.info(f"[run cmd] {command}")
        if command.startswith("[") and command.endswith("]"):
            try:
                command_list = eval(command)
            except SyntaxError:
                sys.exit(1)
            except Exception:
                sys.exit(1)
            else:
                res = subprocess.run(
                    command_list, capture_output=True, shell=True, check=True, cwd=cwd, env=env)
        else:
            res = subprocess.run(command, capture_output=True,
                                 shell=True, check=True, cwd=cwd, env=env)
    except subprocess.CalledProcessError as ce:
        if ce.stderr:
            encoding = chardet.detect(ce.stderr).get("encoding")
            content = ce.stderr.decode(encoding).strip()
        else:
            encoding = chardet.detect(ce.stdout).get("encoding")
            content = ce.stdout.decode(encoding).strip()

        if fail_exit:
            sys.exit(1)
        else:
            raise ce
    except Exception as e:
        if fail_exit:
            sys.exit(1)
        else:
            raise e
    else:
        content = ""
        if res.stdout:
            encoding = chardet.detect(res.stdout).get("encoding")
            content = res.stdout.decode(encoding).strip()
        return content


def build_spdlog() -> Tuple[str, Optional[str], Optional[str]]:
    """编译spdlog"""
    logging.info('[build spdlog] start!')
    # 检查系统中是否有相关的工具(git,cmake,make)
    runcmd("git --version", visible=False, fail_exit=True)
    runcmd("make -v", visible=False, fail_exit=True)
    runcmd("cmake --version", visible=False, fail_exit=True)
    logging.info(f'[build spdlog]can install by source')

    url = "https://github.com/gabime/spdlog.git"
    to = DEFAULT_CYTHON_VENDER.joinpath("spdlog")
    logging.info(f'[build spdlog]spdlog will install in {to}')
    if not to.parent.exists():
        to.parent.mkdir(parents=True)
    else:
        if not to.parent.is_dir():
            logging.info(
                f'[build spdlog]{to.parent} exists but not dir,please remove it first')
            raise Exception(f"{to.parent} is not dir")
    inwondows = False
    if not sys.platform.startswith('linux') and not sys.platform == 'darwin':
        inwondows = True
    if not to.is_dir():
        # 不存在则下载后从源码安装
        to_build = to.parent.joinpath("spdlogbuild")
        logging.info(f'[build spdlog]will build in {to_build}')
        if to_build.exists():
            if to_build.is_dir():
                shutil.rmtree(to_build)
            else:
                to_build.unlink(True)
        clone_cmd = f"git clone {url} {str(to_build)}"
        runcmd(clone_cmd, fail_exit=True)
        logging.info(f'[build spdlog]clone source done')

        try:
            logging.info(f'[build spdlog]compiling')
            if inwondows is False:
                # 非window平台
                runcmd("mkdir build", cwd=to_build)
                runcmd("cmake ..", cwd=to_build.joinpath("build"))
                runcmd("make -j", cwd=to_build.joinpath("build"))
                logging.info(f'[build spdlog]compile done')
                # copy库文件
                to.joinpath("lib").mkdir(exist_ok=True)
                shutil.copyfile(to_build.joinpath("build/libspdlog.a"),
                                to.joinpath("lib/libspdlog.a"))
                logging.info(f'[build spdlog]copy lib to {to} done')
            # copy头文件
            shutil.copytree(to_build.joinpath(
                "include"), to.joinpath("include"))
            logging.info(f'[build spdlog]copy header to {to} done')
                
        except Exception as e:
            raise e
        finally:
            # 删除编译源文件
            try:
                shutil.rmtree(to_build)
            except Exception as e:
                logging.warn(f'[build spdlog]remove build dir {to_build} get error, please remove it manually')
            else:
                logging.info(f'[build spdlog]remove build dir {to_build} done')
    else:
        logging.info('[build spdlog]already build yet')
    if inwondows:
        return str(to.joinpath("include")), None, None
    return str(to.joinpath("include")), str(to.joinpath("lib")), "spdlog"


# 待编译包对应的编译函数
LIB_CPL = {
    "spdlog": build_spdlog
}


class binary_build_ext(build_ext):
    """覆写build_ext"""

    def build_extension(self, ext):
        logging.info(f'[build_ext]build_extension start')
        # 从环境变量中指定配置
        candidate_3rdpart = ["spdlog"]
        BINARY_VECTOR_INCLUDE_DIR = os.environ.get('BINARY_VECTOR_INCLUDE_DIR')
        BINARY_VECTOR_LIB_DIR = os.environ.get('BINARY_VECTOR_LIB_DIR')
        # 额外的lib,如果有则需要指定它的对应lib_dir和include_dir
        BINARY_VECTOR_EXT_LIB = os.environ.get('BINARY_VECTOR_EXT_LIB')
        add_include_dirs = []
        add_lib_dirs = []
        add_libs = []
        noneed_downloads = []
        if BINARY_VECTOR_EXT_LIB:
            # 通过环境变量控制额外增加的lib
            add_libs = [i.strip()
                        for i in BINARY_VECTOR_EXT_LIB.split(",")]
            logging.info(f'[build_ext]add_ext_libs {add_libs}')
        # 判断是否存在外部依赖库的环境变量
        if BINARY_VECTOR_INCLUDE_DIR:
            # 判断是否存在外部依赖库的环境变量
            add_include_dirs = [i.strip()
                                for i in BINARY_VECTOR_INCLUDE_DIR.split(",")]
            logging.info(f'[build_ext]add_include_dirs {add_include_dirs}')
        if BINARY_VECTOR_LIB_DIR:
            add_lib_dirs = [i.strip()
                            for i in BINARY_VECTOR_LIB_DIR.split(",")]
            logging.info(f'[build_ext]add_lib_dirs {add_lib_dirs}')
        # # [可选]查找系统库,看有没有用到的candidate_3rdpart,建议注释掉还是源码编译
        if sys.platform.startswith('linux'):
            # linux
            sys_lib_base_dirs = ["/usr/include", "/usr/local/include"]
            for sys_lib_base_dir_str in sys_lib_base_dirs:
                sys_lib_base_dir = Path(sys_lib_base_dir_str)
                for libname in candidate_3rdpart:
                    libpath = sys_lib_base_dir.joinpath(libname)
                    if libpath.is_dir():
                        logging.info(f'[build_ext]{libname} use sys lib')
                        noneed_downloads.append(libname)
                    libpath = sys_lib_base_dir.joinpath(libname+".h")
                    if libpath.is_file():
                        logging.info(f'[build_ext]{libname} use sys lib')
                        noneed_downloads.append(libname)
                    libpath = sys_lib_base_dir.joinpath(libname+".hpp")
                    if libpath.is_file():
                        logging.info(f'[build_ext]{libname} use sys lib')
                        noneed_downloads.append(libname)

        elif sys.platform == 'darwin':
            # macos,查看homebrew链接的库
            if platform.machine() == "x86_64":
                sys_lib_base_dir = Path("/usr/local/include")
            else:
                sys_lib_base_dir = Path("/opt/homebrew/include")

            for libname in candidate_3rdpart:
                libpath = sys_lib_base_dir.joinpath(libname)
                if libpath.is_dir():
                    libpath = sys_lib_base_dir.joinpath(libname)
                    if libpath.is_dir():
                        logging.info(f'[build_ext]{libname} use sys lib')
                        noneed_downloads.append(libname)
                    libpath = sys_lib_base_dir.joinpath(libname+".h")
                    if libpath.is_file():
                        logging.info(f'[build_ext]{libname} use sys lib')
                        noneed_downloads.append(libname)
                    libpath = sys_lib_base_dir.joinpath(libname+".hpp")
                    if libpath.is_file():
                        logging.info(f'[build_ext]{libname} use sys lib')
                        noneed_downloads.append(libname)

        need_downloads = list(set(candidate_3rdpart)-set(noneed_downloads))
        logging.info(f'[build_ext]{need_downloads} will install by source')
        # 分别编译
        for libname in need_downloads:
            cf = LIB_CPL.get(libname)
            if cf is None:
                logging.error(
                    f'[build_ext]{libname } do not has compile function')
                raise Exception(f"{libname} without compile function")
            include_dir, lib_dir, lib = cf()
            add_include_dirs.append(include_dir)
            if lib_dir:
                add_lib_dirs.append(lib_dir)
            if lib:
                add_libs.append(lib)

        for inc in add_include_dirs:
            self.compiler.add_include_dir(inc)
        for lib_dir in add_lib_dirs:
            self.compiler.add_library_dir(lib_dir)
        for lib in add_libs:
            self.compiler.add_library(lib)
        logging.info(f'[build_ext]setting done')
        # 针对平台编译器的额外参数
        if sys.platform == 'darwin':
            # 针对clang:
            ext.extra_compile_args += ["-Wno-unreachable-code","-Wc++11-extensions", "-std=c++11"]
        elif sys.platform.startswith('linux'):
            # 针对gcc
            ext.extra_compile_args += ["-Wno-unreachable-code"]
        else:
            # 针对cl.exe
            pass
        super().build_extension(ext)

...
setup(
    ext_modules=cythonize("binary_vector/**/*.pyx"),
    cmdclass={
        'build_ext': binary_build_ext
    }
)

```

其中函数`runcmd`是用来执行子进程的,我们检查编译工具,执行编译操作都需要在子进程中进行.如果你本身使用我写的包[pmfp](https://github.com/Project-Manager-With-Git/pmfp)来管理项目,也可以直接使用其中的`pmfp.utils.run_command_utils.run`来执行子进程.

全局字典`LIB_CPL`用于管理需要编译的包和编译脚本的对应关系,而在这个例子中`build_spdlog`是`spdlog`包的编译脚本.由于C/C++并没有统一的编译分发模式,不同的包只能各自单独实现自己的编译脚本.

类`binary_build_ext`是`setuptools.command.build_ext.build_ext`的子类,我们覆写其中的方法`build_extension(self, ext)->None`,主要就是干两个事

+ 根据环境编译依赖的第三方外部包
+ 将第三方外部包的头文件,动态库路径等加入模块的编译设置(`self.compiler.add_include_dir`,`self.compiler.add_library_dir`,`self.compiler.add_library`实现)
+ 根据平台设置扩展对象编译时的额外参数(ext.extra_compile_args)

最后我们需要将`setuptools.command.build_ext.build_ext`扩展的子类也就是`binary_build_ext`注册进`cmdclass`中,也就是

```python
setup(
    ...
    cmdclass={
        'build_ext': binary_build_ext
    }
)
```


## 打包

cython介入到python项目中一般不太会是在早期,很多时候是业务需求稳定后开始追求效率的时候.因此通常需求是--在不影响现有实现的情况下提高执行效率.这就需要找到性能瓶颈后渐进的修改原有项目.

我们的例子便体现了这一过程,在cython实现`vector.pyx`外还有个纯python实现`vector.py`,它和cython完全无关.如果不编译这个例子项目一样可以被import,一样可以使用;如果编译了,由于python解释器会优先导入动态链接库模块,编译后的高性能版本就会替代纯python版本实现,从而达到上面的需求.

为了不影响纯python版本的安装,我们可以做如下设置:

+ `pyproject.toml`
    1. `tool.setuptools.package-data`中将`.pyx`, `.pxd`, `.h`, `.c`, `.hpp`, `.cpp`这些**源文件都放入包中**
        ```toml
        ...
        [tool.setuptools.package-data]
        "*" = ["*.pyx", "*.pxd", "*.h", "*.c", "*.hpp", "*.cpp"]
        ...
        ```
    2. `build-system.requires`中**不要设置Cython**

        ```toml
        [build-system]
        requires = ["setuptools>=61.0.0", "wheel", "chardet"]
        build-backend = "setuptools.build_meta"
        ...
        ```
        
+ `setup.py`,增加对cython安装状态的判断

    ```python
    ...
    try:
        from Cython.Build import cythonize
    except Exception as e:
        logging.info(f'[build]pure python mode')
        setup()
    else:
        logging.info(f'[build]cython mode')
        setup(
            ext_modules=cythonize("binary_vector/**/*.pyx"),
            cmdclass={
                'build_ext': binary_build_ext
            }
        )
    ...
    ```
    
+ [可选]在实现部分打出警告自己是python实现还是cython实现,尤其是python实现有必要

    ```python
    import warnings

    warn_message = "model vector has cython implement, but now just pure python implement"
    warnings.warn(warn_message)
    ...
    ```
    
+ build源码版本,二进制版本仅作为补充.

    ```bash
    python -m build --sdist
    ```

+ 源码版本安装时不使用临时虚拟环境,而是使用当前环境

    ```bash
    pip install --no-build-isolation dist/binary_vector-0.0.1.tar.gz
    ```
    
    
这样如果安装环境中有Cython则会进行编译,没有就当纯python实现执行.

### 完整的例子

这样我们的例子中关于打包的设置可以总结为:

+ `pyproject.toml`

    ```toml
    [build-system]
    requires = ["setuptools>=61.0.0", "wheel", "chardet>=5.2.0"]
    build-backend = "setuptools.build_meta"

    [project]
    name = "binary_vector"
    authors = [{ name = "hsz", email = "hsz1273327@mail.com" }]
    classifiers = [
      "License :: OSI Approved :: MIT License",
      "Programming Language :: Python :: 3",
      "Programming Language :: Python :: 3.10",
      "Programming Language :: Python :: 3.11",
      "Programming Language :: Python :: 3.12"
    ]
    description = "A sample Cython project for test."
    keywords = ["test"]
    license = { file = "LICENSE" }
    dynamic = ["version", "readme", "dependencies"]
    requires-python = ">=3.10"

    [project.urls]
    changelog = "https://github.com/me/spam/blob/master/CHANGELOG.md"
    documentation = "https://readthedocs.org"
    homepage = "https://example.com"
    repository = "https://github.com/me/spam.git"


    [tool.setuptools]
    platforms = ["all"]
    include-package-data = true

    [tool.setuptools.dynamic]
    readme = { file = ["README.md"], content-type = "text/markdown" }
    version = { attr = "binary_vector.version.__version__" }
    dependencies = { file = ["requirements.txt"] }

    [tool.setuptools.package-data]
    "*" = ["*.pyx", "*.pxd", "*.h", "*.c", "*.hpp", "*.cpp"]

    [tool.setuptools.packages.find]
    exclude = ['contrib', 'docs', 'test']

    ```
    
+ `setup.py`

    ```python
    import os
    import sys
    import shutil
    import logging
    import subprocess
    from pathlib import Path
    import chardet
    from setuptools import setup
    from setuptools.command.build_ext import build_ext
    from typing import Optional, Dict, Tuple


    DEFAULT_CYTHON_VENDER = Path.home().joinpath(".cython_vendor")


    def runcmd(command: str, *, cwd: Optional[Path] = None, env: Optional[Dict[str, str]] = None, visible: bool = False, fail_exit: bool = False,) -> str:
        """执行命令行命令并返回其stdout的值

        Args:
            command (str): 命令行命令
            cwd (Optional[Path]): 执行命令时的位置.Default: None
            env (Optional[Any]): 执行命令时的环境变量. Default:None
            visible (bool): 命令可见度. Default: False
            fail_exit (bool): 当执行失败时退出程序. Default: False

        Returns:
            str: stdout捕获的字符串
        """
        try:
            if visible:
                logging.info(f"[run cmd] {command}")
            if command.startswith("[") and command.endswith("]"):
                try:
                    command_list = eval(command)
                except SyntaxError:
                    sys.exit(1)
                except Exception:
                    sys.exit(1)
                else:
                    res = subprocess.run(
                        command_list, capture_output=True, shell=True, check=True, cwd=cwd, env=env)
            else:
                res = subprocess.run(command, capture_output=True,
                                     shell=True, check=True, cwd=cwd, env=env)
        except subprocess.CalledProcessError as ce:
            if ce.stderr:
                encoding = chardet.detect(ce.stderr).get("encoding")
                content = ce.stderr.decode(encoding).strip()
            else:
                encoding = chardet.detect(ce.stdout).get("encoding")
                content = ce.stdout.decode(encoding).strip()

            if fail_exit:
                sys.exit(1)
            else:
                raise ce
        except Exception as e:
            if fail_exit:
                sys.exit(1)
            else:
                raise e
        else:
            content = ""
            if res.stdout:
                encoding = chardet.detect(res.stdout).get("encoding")
                content = res.stdout.decode(encoding).strip()
            return content


    def build_spdlog() -> Tuple[str, Optional[str], Optional[str]]:
        """编译spdlog"""
        logging.info('[build spdlog] start!')
        # 检查系统中是否有相关的工具(git,cmake,make)
        runcmd("git --version", visible=False, fail_exit=True)
        runcmd("make -v", visible=False, fail_exit=True)
        runcmd("cmake --version", visible=False, fail_exit=True)
        logging.info(f'[build spdlog]can install by source')

        url = "https://github.com/gabime/spdlog.git"
        to = DEFAULT_CYTHON_VENDER.joinpath("spdlog")
        logging.info(f'[build spdlog]spdlog will install in {to}')
        if not to.parent.exists():
            to.parent.mkdir(parents=True)
        else:
            if not to.parent.is_dir():
                logging.info(
                    f'[build spdlog]{to.parent} exists but not dir,please remove it first')
                raise Exception(f"{to.parent} is not dir")
        inwondows = False
        if not sys.platform.startswith('linux') and not sys.platform == 'darwin':
            inwondows = True
        if not to.is_dir():
            # 不存在则下载后从源码安装
            to_build = to.parent.joinpath("spdlogbuild")
            logging.info(f'[build spdlog]will build in {to_build}')
            if to_build.exists():
                if to_build.is_dir():
                    shutil.rmtree(to_build)
                else:
                    to_build.unlink(True)
            clone_cmd = f"git clone {url} {str(to_build)}"
            runcmd(clone_cmd, fail_exit=True)
            logging.info(f'[build spdlog]clone source done')

            try:
                logging.info(f'[build spdlog]compiling')
                if inwondows is False:
                    # 非window平台
                    runcmd("mkdir build", cwd=to_build)
                    runcmd("cmake ..", cwd=to_build.joinpath("build"))
                    runcmd("make -j", cwd=to_build.joinpath("build"))
                    logging.info(f'[build spdlog]compile done')
                    # copy库文件
                    to.joinpath("lib").mkdir(parents=True, exist_ok=True)
                    shutil.copyfile(to_build.joinpath("build/libspdlog.a"),
                                    to.joinpath("lib/libspdlog.a"))
                    logging.info(f'[build spdlog]copy lib to {to} done')
                # copy头文件
                shutil.copytree(
                    to_build.joinpath("include"),
                    to.joinpath("include")
                )
                logging.info(f'[build spdlog]copy header to {to} done')

            except Exception as e:
                logging.error(
                    f'[build spdlog]compiling get error {type(e)}:{str(e)}')
                raise e
            finally:
                # 删除编译源文件
                try:
                    shutil.rmtree(to_build)
                except Exception as e:
                    logging.warn(
                        f'[build spdlog]remove build dir {to_build} get error, please remove it manually')
                else:
                    logging.info(f'[build spdlog]remove build dir {to_build} done')
        else:
            logging.info('[build spdlog]already build yet')
        if inwondows:
            return str(to.joinpath("include")), None, None
        return str(to.joinpath("include")), str(to.joinpath("lib")), "spdlog"


    # 待编译包对应的编译函数
    LIB_CPL = {
        "spdlog": build_spdlog
    }


    class binary_build_ext(build_ext):
        """覆写build_ext"""

        def build_extension(self, ext):
            logging.info(f'[build_ext]build_extension start')
            # 从环境变量中指定配置
            candidate_3rdpart = ["spdlog"]
            BINARY_VECTOR_INCLUDE_DIR = os.environ.get('BINARY_VECTOR_INCLUDE_DIR')
            BINARY_VECTOR_LIB_DIR = os.environ.get('BINARY_VECTOR_LIB_DIR')
            # 额外的lib,如果有则需要指定它的对应lib_dir和include_dir
            BINARY_VECTOR_EXT_LIB = os.environ.get('BINARY_VECTOR_EXT_LIB')
            add_include_dirs = []
            add_lib_dirs = []
            add_libs = []
            noneed_downloads = []
            if BINARY_VECTOR_EXT_LIB:
                # 通过环境变量控制额外增加的lib
                add_libs = [i.strip()
                            for i in BINARY_VECTOR_EXT_LIB.split(",")]
                logging.info(f'[build_ext]add_ext_libs {add_libs}')
            # 判断是否存在外部依赖库的环境变量
            if BINARY_VECTOR_INCLUDE_DIR:
                # 判断是否存在外部依赖库的环境变量
                add_include_dirs = [i.strip()
                                    for i in BINARY_VECTOR_INCLUDE_DIR.split(",")]
                logging.info(f'[build_ext]add_include_dirs {add_include_dirs}')
            if BINARY_VECTOR_LIB_DIR:
                add_lib_dirs = [i.strip()
                                for i in BINARY_VECTOR_LIB_DIR.split(",")]
                logging.info(f'[build_ext]add_lib_dirs {add_lib_dirs}')
            # # [可选]查找系统库,看有没有用到的candidate_3rdpart,建议注释掉还是源码编译
            if sys.platform.startswith('linux'):
                # linux
                sys_lib_base_dirs = ["/usr/include", "/usr/local/include"]
                for sys_lib_base_dir_str in sys_lib_base_dirs:
                    sys_lib_base_dir = Path(sys_lib_base_dir_str)
                    for libname in candidate_3rdpart:
                        libpath = sys_lib_base_dir.joinpath(libname)
                        if libpath.is_dir():
                            logging.info(f'[build_ext]{libname} use sys lib')
                            noneed_downloads.append(libname)
                        libpath = sys_lib_base_dir.joinpath(libname+".h")
                        if libpath.is_file():
                            logging.info(f'[build_ext]{libname} use sys lib')
                            noneed_downloads.append(libname)
                        libpath = sys_lib_base_dir.joinpath(libname+".hpp")
                        if libpath.is_file():
                            logging.info(f'[build_ext]{libname} use sys lib')
                            noneed_downloads.append(libname)

            # elif sys.platform == 'darwin':
            #     # macos,查看homebrew
            #     sys_lib_base_dir = Path("/usr/local/Cellar")
            #     for libname in candidate_3rdpart:
            #         libpath = sys_lib_base_dir.joinpath(libname)
            #         if libpath.is_dir():
            #             libversionpath = [i for i in libpath.iterdir() if i.is_dir() and not i.name.startswith(".")]
            #             if len(libversionpath) >= 1:
            #                 version = libversionpath[0].name
            #                 noneed_downloads.append(libname)
            #                 logging.info(f'[build_ext]{libname} use sys lib')
            #                 if Path(f"/usr/local/Cellar/{libname}/{version}/include"):
            #                     add_include_dirs.append(
            #                         f"/usr/local/Cellar/{libname}/{version}/include")
            #                 if Path(f"/usr/local/Cellar/{libname}/{version}/lib").is_dir():
            #                     add_lib_dirs.append(
            #                         f"/usr/local/Cellar/{libname}/{version}/lib")

            need_downloads = list(set(candidate_3rdpart)-set(noneed_downloads))
            logging.info(f'[build_ext]{need_downloads} will install by source')
            # 分别编译
            for libname in need_downloads:
                cf = LIB_CPL.get(libname)
                if cf is None:
                    logging.error(
                        f'[build_ext]{libname } do not has compile function')
                    raise Exception(f"{libname} without compile function")
                include_dir, lib_dir, lib = cf()
                add_include_dirs.append(include_dir)
                if lib_dir:
                    add_lib_dirs.append(lib_dir)
                if lib:
                    add_libs.append(lib)

            for inc in add_include_dirs:
                self.compiler.add_include_dir(inc)
            for lib_dir in add_lib_dirs:
                self.compiler.add_library_dir(lib_dir)
            for lib in add_libs:
                self.compiler.add_library(lib)
            logging.info(f'[build_ext]setting done')
            # 针对平台编译器的额外参数
            if sys.platform == 'darwin':
                # 针对clang:
                ext.extra_compile_args += ["-Wno-unreachable-code",
                                           "-Wc++11-extensions", "-std=c++11"]
            elif sys.platform.startswith('linux'):
                # 针对gcc
                ext.extra_compile_args += ["-Wno-unreachable-code"]
            else:
                # 针对cl.exe
                pass
            super().build_extension(ext)


    try:
        from Cython.Build import cythonize
    except Exception as e:
        logging.info(f'[build]pure python mode')
        setup()
    else:
        logging.info(f'[build]cython mode')
        setup(
            ext_modules=cythonize("binary_vector/**/*.pyx"),
            cmdclass={
                'build_ext': binary_build_ext
            }
        )

    ```

## 分发

对于cython项目而言,分发可以认为就是将源码和一些常用平台,指令集,python实现版本的wheel包挂到大家普遍比较容易获取到的地方.我们以github作为项目代码的托管场所,也就用[github action](https://blog.hszofficial.site/introduce/2020/11/30/%E4%BD%BF%E7%94%A8GithubActions%E8%87%AA%E5%8A%A8%E5%8C%96%E5%B7%A5%E4%BD%9C%E6%B5%81/)进行演示了.

我们使用[pypa/cibuildwheel](https://github.com/pypa/cibuildwheel)这个action.它本身是一个python工具,专门用来做wheel的打包工作


### 分发平台设置

平台指的是操作系统,主流的就是windows,macos,linux,在github action中可以设置的包括`ubuntu-22.04`(`ubuntu-latest`), `ubuntu-20.04`, `windows-2022`(`windows-latest`), `windows-2019`, `macos-12`(`macos-latest`), `macos-11`以及截止2023年12月1日还在beta版本的`macos-13`.为了兼容性通常常用的平台指的是`ubuntu-20.04`,`windows-2019`和`macos-11`.在workflow中一般在`joibs.{job名}.runs-on`中指定,对于编译wheel来说一般用`matrix`指定:

```yaml
jobs:
  ...
  build_wheels:
    name: Build wheels on ${{ matrix.os }}
    runs-on: ${{ matrix.os }}
    strategy:
      matrix:
        os: [ubuntu-20.04, windows-2019, macOS-11]

```

### 分发指令集设置

指令集指的是cpu指令集,通常主流的是x86_64(又叫amd64),arm64(又叫aarch64),在`cibuildwheel`中支持的是:

+ windows下:支持`AMD64`,`x86`,`ARM64`,一般`AMD64`算常用.使用环境变量`CIBW_ARCHS_WINDOWS: AMD64`设置
+ macos下:支持`x86_64`,`arm64`,一般都算常用.使用环境变量`CIBW_ARCHS_MACOS: x86_64 arm64`设置.除此之外还可选`universal2`,这是一个兼容模式,打一份包`x86_64`,`arm64`都可以使用,通常两种选择选一种即可.
+ linux下:支持`x86_64`, `i686`, `aarch64`, `ppc64le`, `s390x`,一般`x86_64`和`aarch64`算常用.使用环境变量`CIBW_ARCHS_LINUX: x86_64 aarch64`设置

```yaml
jobs:
  ...
  build_wheels:
    steps:
      ...
      - name: Build Wheels Without Pypy 
        if: ${{ inputs.withpypy }}
        uses: pypa/cibuildwheel@v2.16.2
        env:
          ...
          # linux下编译哪些arch
          CIBW_ARCHS_LINUX: x86_64 aarch64
          # mac下编译哪些arch
          CIBW_ARCHS_MACOS: x86_64 arm64
          # windows下编译哪些arch
          CIBW_ARCHS_WINDOWS: AMD64
```

### python实现版本设定

python实现版本指的是是cpython实现还是pypy实现,针对的是哪个版本.这个通常会根据你的项目中的setup信息来自动判别.比如我们的例子在配置文件`pyproject.toml`中的`project.requires-python`中设定的是`>=3.10`,因此就会执行当前`cibuildwheel`支持的且大于等于cpython 3.10的版本,在写这个例子的当下(cibuildwheel 2.16.2)也就是会执行`3.10`,`3.11`,`3.12`.同时`cibuildwheel`默认也会为pypy打包,如果不需要可以使用环境变量`CIBW_SKIP: pp*`取消.

我们不妨将是否编译pypy版本作为可选项,默认不编译,但如果是手动触发且设置了要编译则进行编译

```yaml
on: 
  workflow_dispatch:
  inputs:
    withpypy:
      description: 'True to print to STDOUT'
      required: false
      default: false
      type: boolean
  ...
  
jobs:
  ...
  build_wheels:
    name: Build wheels on ${{ matrix.os }}
      ...
      steps:
        ...
        - name: Build Wheels With Pypy 
          if: ${{ inputs.withpypy }}
          uses: pypa/cibuildwheel@v2.16.2
          env:
            ...

        - name: Build Wheels Without Pypy
          if: ${{ !inputs.withpypy }} 
          uses: pypa/cibuildwheel@v2.16.2
          env:
            # 不为pypy编译
            CIBW_SKIP: pp*
            ...

```

### 常用发布位置设置

对于python生态来说,大家普遍比较容易获取到的地方自然是[pypi](https://pypi.org/).我们可以使用一个发布步骤发布我们的包

```yaml
jobs:
  build_sdist:
    runs-on: ubuntu-latest
    name: Build Sdist
    steps:
      ...
      - name: Publish Wheels
        env:
          TWINE_USERNAME: ${{ secrets.PYPI_USERNAME }}
          TWINE_PASSWORD: ${{ secrets.PYPI_PASSWORD }}
        run:
          twine upload wheelhouse/*.whl
```

但一些时候我们可能不想或不能公开包到pypi,那最简单的办法就是编译完后找个地方存下来,我们可以下载它再另做打算,我们就可以使用一个上传步骤发布我们的包
    
```yaml
jobs:
  build_sdist:
    runs-on: ubuntu-latest
    name: Build Sdist
    steps:
      ...
      - name: 'Upload Wheels'
        uses: actions/upload-artifact@v3
        with:
          name: packages
          path: ./wheelhouse/*.whl
```

### 指定编译命令和依赖

我们使用命令`python -m build --wheel --no-isolation`编译wheel,在`cibuildwheel`中对应的设置是环境变量`CIBW_BUILD_FRONTEND: "build; args: --no-isolation"`,它表示build操作使用工具`build`,且命令有额外参数`--no-isolation`.由于我们的setup中Cython是可选的,而有个不太常用的依赖`chardet`,这两个都需要预先安装,因此需要额外设置环境变量`CIBW_BEFORE_BUILD: pip install Cython==3.0.6 chardet==5.2.0`让`cibuildwheel`在build操作之前先安装好这些依赖

```yaml
jobs:
  ...
  build_wheels:
    steps:
      ...
      - name: Build Wheels Without Pypy 
        if: ${{ inputs.withpypy }}
        uses: pypa/cibuildwheel@v2.16.2
        env:
          ...
          # 注意,不在临时虚拟环境中编译
          CIBW_BUILD_FRONTEND: "build; args: --no-isolation"
          # 安装编译用的依赖
          CIBW_BEFORE_BUILD: pip install Cython==3.0.6 chardet==5.2.0
```
              
### 先源码,再二进制

对于我们写的这种架构的cython项目而言在二进制包之外,其实更重要的是源码包.没有二进制包不过是一一些平台需要要么自行编译,要么是只能用python版实现.而如果没有源码包,那整个发布都是不成功的.

```yaml
jobs:
  build_sdist:
    runs-on: ubuntu-latest
    name: Build Sdist
    steps:
      - uses: actions/checkout@v4

      - name: Set up Python
        uses: actions/setup-python@v4
        with:
            python-version: '3.x'

      - name: Install Dependencies
        run: |
            python -m pip install --upgrade pip
            pip install setuptools wheel twine build

      - name: Build Sdist
        run: |
            python -m build --sdist

      - name: 'Upload Sdist'
        uses: 'actions/upload-artifact@v3'
        with:
            name: packages
            path: dist/*

    #   - name: Publish Sdist
    #     env:
    #         TWINE_USERNAME: ${{ secrets.PYPI_USERNAME }}
    #         TWINE_PASSWORD: ${{ secrets.PYPI_PASSWORD }}
    #     run:
    #         twine upload dist/*
```

### 完整的例子

最终我们的workflow是这样:


+ `.github/workflows/release.yml`

    ```yaml
    name: Build_And_Release

    on: 
      workflow_dispatch:
        inputs:
          withpypy:
            description: 'True to print to STDOUT'
            required: false
            default: false
            type: boolean
      release:
            types: [created]

    jobs:
      build_sdist:
        runs-on: ubuntu-latest
        name: Build Sdist
        steps:
          - uses: actions/checkout@v4

          - name: Set up Python
            uses: actions/setup-python@v4
            with:
                python-version: '3.x'

          - name: Install Dependencies
            run: |
                python -m pip install --upgrade pip
                pip install setuptools wheel twine build

          - name: Build Sdist
            run: |
                python -m build --sdist

          - name: 'Upload Sdist'
            uses: 'actions/upload-artifact@v3'
            with:
                name: packages
                path: dist/*

        #   - name: Publish Sdist
        #     env:
        #         TWINE_USERNAME: ${{ secrets.PYPI_USERNAME }}
        #         TWINE_PASSWORD: ${{ secrets.PYPI_PASSWORD }}
        #     run:
        #         twine upload dist/*

      build_wheels:
        name: Build wheels on ${{ matrix.os }}
        needs: build_sdist
        runs-on: ${{ matrix.os }}
        strategy:
          matrix:
            os: [ubuntu-20.04, windows-2019, macOS-11]

        steps:
          - uses: actions/checkout@v4

          # Used to host cibuildwheel
          - name: Set up QEMU 
            if: runner.os == 'Linux'
            uses: docker/setup-qemu-action@v3
            with:
              platforms: all

          - name: Build Wheels With Pypy 
            if: ${{ inputs.withpypy }}
            uses: pypa/cibuildwheel@v2.16.2
            env:
              # linux下编译哪些arch
              CIBW_ARCHS_LINUX: x86_64 aarch64
              # mac下编译哪些arch
              CIBW_ARCHS_MACOS: x86_64 arm64
              # windows下编译哪些arch
              CIBW_ARCHS_WINDOWS: AMD64
              # 注意,不在临时虚拟环境中编译
              CIBW_BUILD_FRONTEND: "build; args: --no-isolation"
              # 安装编译用的依赖
              CIBW_BEFORE_BUILD: pip install Cython==3.0.6 chardet==5.2.0

          - name: Build Wheels Without Pypy
            if: ${{ !inputs.withpypy }} 
            uses: pypa/cibuildwheel@v2.16.2
            env:

              # 不为pypy编译
              CIBW_SKIP: pp*
              # linux下编译哪些arch
              CIBW_ARCHS_LINUX: x86_64 aarch64
              # mac下编译哪些arch
              CIBW_ARCHS_MACOS: x86_64 arm64
              # windows下编译哪些arch
              CIBW_ARCHS_WINDOWS: AMD64
              # 注意,不在临时虚拟环境中编译
              CIBW_BUILD_FRONTEND: "build; args: --no-isolation"
              # 安装编译用的依赖
              CIBW_BEFORE_BUILD: pip install Cython==3.0.6 chardet==5.2.0

          - name: 'Upload Wheels'
            uses: actions/upload-artifact@v3
            with:
              name: packages
              path: ./wheelhouse/*.whl

        #   - name: Publish Wheels
        #     env:
        #         TWINE_USERNAME: ${{ secrets.PYPI_USERNAME }}
        #         TWINE_PASSWORD: ${{ secrets.PYPI_PASSWORD }}
        #     run:
        #         twine upload wheelhouse/*.whl
    ```

## 从pip安装源码

如果我们将项目发布到了pypi或满足pypi协议的python包仓库,我们就可以使用pip安装.pip默认情况下会优先查找看是否有满足安装条件的二进制包可以安装,如果没有则会下载源码包进行安装.这里主要要讨论的就是源码包的安装.

简单说就是如下这句

```bash
pip install --no-build-isolation --no-binary=binary_vector binary_vector
```

指定`--no-binary=binary_vector`可以让pip在查找`binary_vector`包时直接查找源码包,`--no-build-isolation`则会保证根据环境中是否有cython的存在而确定是否会编译

不过由于我并没有将这个包放到pypi上,因此上面的安装命令不会有用.

## 结语

本文用一个例子介绍了本人实践的用cython写python项目的最佳项目架构.并从代码到项目编译设置再到发布设置走完了完整的开发声明周期.

