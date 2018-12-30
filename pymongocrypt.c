#include <Python.h> /* must be included before any other headers. */
#include "pymongocrypt.h"
#include "mongocrypt.h"

static PyObject *
_encrypt (PyObject *self, PyObject *args) {
    char* doc;
    int doc_len;

    if (!PyArg_ParseTuple(args, "s#", &doc, &doc_len)) {
        return NULL;
    }
    
    printf("got buffer of size: %d\n", doc_len);
    return Py_BuildValue("i", 1);
} 


static PyObject *
_setup (PyObject *self, PyObject *args) {
    char* aws_region, *aws_access_key_id, *aws_secret_access_key;
    if (!PyArg_ParseTuple(args, "sss", &aws_region, &aws_access_key_id, &aws_secret_access_key)) {
        return NULL;
    }
    printf("aws_region=%s\n", aws_region);

    mongoc_crypt_init();

    return Py_BuildValue("i", 1);
}


static PyObject *
_cleanup(PyObject *self, PyObject *args) {
    mongoc_crypt_cleanup();
    return Py_BuildValue("i", 1);
}


static PyMethodDef CryptMethods[] = {
    {"encrypt", _encrypt, METH_VARARGS, "encrypt a document"},
    {"setup", _setup, METH_VARARGS, "set options and initialize"},
    {"cleanup", _cleanup, METH_VARARGS, "set options and initialize"},
    {NULL, NULL, 0, NULL} /* sentinel */
};


PyMODINIT_FUNC
initpymongocrypt(void)
{
    (void) Py_InitModule("pymongocrypt", CryptMethods);
}
