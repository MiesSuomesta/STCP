#define PY_SSIZE_T_CLEAN
#include <Python.h>

int main(int argc, char *argv[]) {
    Py_Initialize();

    // Lisää hakupolku, jotta Python löytää moduulin
    PyRun_SimpleString("import sys\nsys.path.insert(0, '.')");

    // Lataa `test_client`-moduuli
    PyObject *pModule = PyImport_ImportModule("test_server");
    if (!pModule) {
        PyErr_Print();
        return 1;
    }

    // Hae `main()`-funktio ja suorita se
    PyObject *pFunc = PyObject_GetAttrString(pModule, "main");
    if (pFunc && PyCallable_Check(pFunc)) {
        PyObject_CallObject(pFunc, NULL);
    } else {
        PyErr_Print();
    }

    Py_XDECREF(pFunc);
    Py_XDECREF(pModule);
    Py_Finalize();

    return 0;
}
