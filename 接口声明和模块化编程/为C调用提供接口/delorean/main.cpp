#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <exception>
#include <filesystem>
#include <format>
#include <vector>
#include "delorean_api.h"
#include "scope_guard.hpp"

class AppException : public std::runtime_error {
   public:
    AppException(const char* err) : std::runtime_error(err) {}
};

void init_py(char* programname, char* envpath, char* pymodulepath, bool debugmod) {
    // 参数校验变量赋值
    if (programname == NULL) {
        throw AppException("Fatal error: programname must set");
    }
    wchar_t* program;
    auto guard_program = sg::make_scope_guard([&program]() noexcept { PyMem_RawFree(program); });
    program = Py_DecodeLocale(programname, NULL);
    if (program == NULL) {
        throw AppException("Fatal error: cannot decode programname");
    }
    // 初始化python设置
    PyStatus status;
    PyConfig config;
    PyConfig_InitPythonConfig(&config);
    auto guard_config = sg::make_scope_guard([&config]() noexcept {
        PyConfig_Clear(&config);
        printf("python init config clear\n");
    });

    // 设置python程序名
    status = PyConfig_SetString(&config, &config.program_name, program);
    if (PyStatus_Exception(status)) {
        throw AppException("Fatal error: InitPythonConfig set program_name get error");
    }
    // 加载默认配置
    status = PyConfig_Read(&config);
    if (PyStatus_Exception(status)) {
        throw AppException("Fatal error: PyConfig_Read get error");
    }
    // 设置python的sys.path用于查找模块
    wchar_t* pymodule_dir_name;
    auto guard_pymodule_dir_name = sg::make_scope_guard([&pymodule_dir_name]() noexcept { PyMem_RawFree(pymodule_dir_name); });
    std::filesystem::path pymodule_dir;
    if (pymodulepath == NULL) {
        pymodule_dir = std::filesystem::current_path();
    } else {
        pymodule_dir = pymodulepath;
        if (pymodule_dir.is_relative()) {
            pymodule_dir = std::filesystem::absolute(pymodule_dir);
        }
    }
    const char* _pymodule_dir_name = nullptr;
    {
        auto _pymodule_dir_name_str = pymodule_dir.string();
        _pymodule_dir_name = _pymodule_dir_name_str.c_str();
    }
    pymodule_dir_name = Py_DecodeLocale(_pymodule_dir_name, NULL);
    if (pymodule_dir_name == NULL) {
        throw AppException("Fatal error: cannot decode pymodule_dir_name");
    } else {
        if (debugmod) {
            printf("pymodule_dir %s \n", _pymodule_dir_name);
        }
    }
    config.module_search_paths_set = 1;
    status = PyWideStringList_Append(&config.module_search_paths, pymodule_dir_name);
    if (PyStatus_Exception(status)) {
        throw AppException("Fatal error: InitPythonConfig set module_search_paths get error");
    }

    // 设置虚拟环境
    wchar_t* env_dir_name;
    auto guard_env_dir_name = sg::make_scope_guard([&env_dir_name, &envpath]() noexcept {
        if (envpath != NULL) {
            PyMem_RawFree(env_dir_name);
        }
    });
    if (envpath != NULL) {
        std::filesystem::path env_dir = envpath;
        if (env_dir.is_relative()) {
            env_dir = std::filesystem::absolute(env_dir);
        }
        const char* _env_dir_name = nullptr;
        {
            auto _env_dir_name_str = env_dir.string();
            _env_dir_name = _env_dir_name_str.c_str();
        }
        env_dir_name = Py_DecodeLocale(_env_dir_name, NULL);
        if (env_dir_name == NULL) {
            throw AppException("Fatal error: cannot decode _env_dir_name");
        } else {
            if (debugmod) {
                printf("use virtual environments %s \n", _env_dir_name);
            }
        }
        status = PyConfig_SetString(&config, &config.prefix, env_dir_name);
        if (PyStatus_Exception(status)) {
            throw AppException("Fatal error: InitPythonConfig set prefix get error");
        }
        status = PyConfig_SetString(&config, &config.exec_prefix, env_dir_name);
        if (PyStatus_Exception(status)) {
            throw AppException("Fatal error: InitPythonConfig set exec_prefix get error");
        }
    }

    status = Py_InitializeFromConfig(&config);
    if (PyStatus_Exception(status)) {
        if (PyStatus_IsExit(status)) {
            // return status.exitcode;
            throw AppException("Fatal error: PyStatus_IsExit");
        }
        // 抛出错误
        Py_ExitStatusException(status);
    }
    if (debugmod) {
        PyRun_SimpleString("import sys;print(sys.path);print(sys.prefix)");
    }
}

int finalize_py() {
    if (Py_FinalizeEx() < 0) {
        return 120;
    }
    return 0;
}

Vehicle car;
int main(int argc, char* argv[]) {
    try {
        // 初始化
        init_py(argv[0], (char*)"env/", NULL, false);
        import_delorean();
        // 开始执行python调用
        car.speed = atoi(argv[1]);
        car.power = atof(argv[2]);
        int x = activate(&car);
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