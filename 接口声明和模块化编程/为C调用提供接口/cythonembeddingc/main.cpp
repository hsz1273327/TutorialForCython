#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <exception>
#include <filesystem>
#include <map>
#include <string>
#include "crow_all.h"
#include "scope_guard.hpp"
#include "emb.h"

// 应用部分
class AppException : public std::runtime_error {
   public:
    AppException(const char* err) : std::runtime_error(err) {}
};

/**
 * @fn BINARY_VECTOR_P VEC_init(float,float)
 * @brief 相对通用的初始化python解释器函数
 * @param[programname]  应用名.
 * @param[envpath]  虚拟环境路径.为NULL则不使用虚拟环境;为相对路径则从应用所在文件夹开始查找
 * @param[pymodulepath]  python模块的额外查找路径.为NULL则将当前应用所在文件夹加入;为相对路径则从应用所在文件夹开始查找
 * @param[pyhomepath]  设置python_home.为相对路径则从应用所在文件夹开始查找.如果`isolated`为true则不能为NULL
 * @param[tabs] 需要预先加载的模块名模块初始化函数信息
 * @param[isolated]  是否使用隔离配置初始化python解释器
 * @param[debugmod]  是否打印debug用的文本
 * @return void
 * @exception <AppException> { 应用级别异常 }
 */
void init_py(char* programname,
             char* envpath,
             char* pymodulepath,
             char* pyhomepath,
             const std::map<std::string, PyObject* (*)(void)>* tabs,
             bool isolated,
             bool debugmod) {
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

    // 初始化
    if (isolated) {
        // 预设置
        PyStatus statusp;
        PyPreConfig preconfig;
        PyPreConfig_InitIsolatedConfig(&preconfig);
        preconfig.utf8_mode = 1;
        statusp = Py_PreInitialize(&preconfig);
        if (PyStatus_Exception(statusp)) {
            Py_ExitStatusException(statusp);
        }
    }
    // 基本设置
    //  隔离模式
    PyStatus status;
    PyConfig config;
    if (isolated) {
        PyConfig_InitIsolatedConfig(&config);
    } else {
        PyConfig_InitPythonConfig(&config);
    }
    auto guard_config = sg::make_scope_guard([&config]() noexcept {
        PyConfig_Clear(&config);
        printf("python init config clear\n");
    });
    // 设置python程序名
    status = PyConfig_SetString(&config, &config.program_name, program);
    if (PyStatus_Exception(status)) {
        throw AppException("Fatal error: InitPythonConfig set program_name get error");
    }

    // 设置python_home
    wchar_t* pyhome;
    auto guard_pyhome = sg::make_scope_guard([&pyhome, &pyhomepath]() noexcept {
        if (pyhomepath != NULL) {
            PyMem_RawFree(pyhome);
        }
    });
    if (pyhomepath != NULL) {
        std::filesystem::path pyhome_dir = pyhomepath;
        if (pyhome_dir.is_relative()) {
            pyhome_dir = std::filesystem::absolute(pyhome_dir);
        }
        const char* _pyhome_dir_name = nullptr;
        {
            auto _pyhome_dir_name_str = pyhome_dir.string();
            _pyhome_dir_name = _pyhome_dir_name_str.c_str();
        }

        pyhome = Py_DecodeLocale(_pyhome_dir_name, NULL);
        if (pyhome == NULL) {
            throw AppException("Fatal error: cannot decode pyhome");
        } else {
            if (debugmod) {
                printf("use python_home %s \n", pyhome);
            }
        }
        status = PyConfig_SetString(&config, &config.home, pyhome);
        if (PyStatus_Exception(status)) {
            throw AppException("Fatal error: InitPythonConfig set home get error");
        }
    } else {
        if (isolated) {
            throw AppException("Fatal error: isolated config must set pyhomepath");
        }
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

    // 提前初始化模块`emb`
    if (tabs != NULL) {
        for (const auto& [key, value] : *tabs) {
            PyImport_AppendInittab(key.c_str(), value);
        }
    }

    // 初始化python解释器
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
    printf("finalize_py ok\n");
    return 0;
}
PyObject* init_pymodule(char* Module_Name) {
    auto pName = PyUnicode_DecodeFSDefault(Module_Name);  // 将模块名类型转为python对象字符串
    auto guard = sg::make_scope_guard([&pName]() noexcept {
        Py_DECREF(pName);  // 释放对象pName的gc计数器
    });
    auto pModule = PyImport_Import(pName);  // 导入模块
    return pModule;
}

int main(int argc, char* argv[]) {
    // 初始化python解释器
    try {
        set_numargsc(10);
        std::map<std::string, PyObject* (*)(void)> tabs{{"emb", PyInit_emb}};
        init_py(argv[0], NULL, NULL, NULL, &tabs, false, true);
    } catch (const AppException& ex) {
        fprintf(stderr, ex.what());
        return 1;
    }
    auto pModule = init_pymodule((char*)"emb");
    auto guard_pModule = sg::make_scope_guard([&pModule]() noexcept {
        Py_XDECREF(pModule);  // 释放pModule
    });
    // http接口逻辑
    crow::SimpleApp app;
    CROW_ROUTE(app, "/api")
    ([](const crow::request& req) {
        // 结束python调用,转换结果
        crow::json::wvalue x({{"status", "ok"}});
        x["result"] = get_numargsc();
        return crow::response(x);
    });

    CROW_ROUTE(app, "/submit").methods("POST"_method)([](const crow::request& req) {
        crow::multipart::message msg(req);
        std::string code_str = msg.get_part_by_name("script").body;
        if (code_str.empty()) {
            return crow::response(crow::status::BAD_REQUEST);  // same as crow::response(400)
        }
        auto code = code_str.c_str();
        // 开始执行python调用
        // // PyGILState_STATE gstate;
        auto _save = PyEval_SaveThread();
        auto gstate = PyGILState_Ensure();
        auto guard_gstate = sg::make_scope_guard([&gstate, &_save]() noexcept {
            PyGILState_Release(gstate);
            PyEval_RestoreThread(_save);
            CROW_LOG_INFO << "PyGILState_Release ok";
        });
        CROW_LOG_INFO << "PyGILState_Ensure ok";
        /* Perform Python actions here. */
        CROW_LOG_INFO << std::format("submit code {}", code);
        int res = PyRun_SimpleString(code);
        if (res == 0) {
            CROW_LOG_INFO << "PyRun_SimpleString ok";
            crow::json::wvalue x({{"status", "ok"}});
            x["result"] = get_numargs();
            return crow::response(x);
        } else {
            CROW_LOG_ERROR << "Python code get error";
            return crow::response(crow::status::INTERNAL_SERVER_ERROR);
        }
    });
    app.loglevel(crow::LogLevel::Info).port(18080).multithreaded().run();
    return finalize_py();
}
