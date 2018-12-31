#include <Python.h> /* must be included before any other headers. */
#include "pymongocrypt.h"
#include "mongocrypt.h"

/* TODO: apply concurrency controls. */
static mongoc_crypt_t* crypt_handle;

static PyObject *
_encrypt (PyObject *self, PyObject *args) {
    mongoc_crypt_bson_t schema = {0}, doc = {0}, out = {0};

    if (!PyArg_ParseTuple(args, "s#s#", &schema.data, &schema.len, &doc.data, &doc.len)) {
        Py_RETURN_FALSE;
    }
    
    mongoc_crypt_error_t error;

    int ret = mongoc_crypt_encrypt (crypt_handle, &schema, &doc, &out, &error);
    
    if (!ret) {
        printf("error: %s\n", error.message);
        Py_RETURN_FALSE;
    }

    return Py_BuildValue("s#", out.data, out.len);
} 


static PyObject *
_decrypt (PyObject *self, PyObject *args) {
    mongoc_crypt_bson_t doc = {0}, out = {0};

    if (!PyArg_ParseTuple(args, "s#", &doc.data, &doc.len)) {
        Py_RETURN_FALSE;
    }
    
    mongoc_crypt_error_t error;

    int ret = mongoc_crypt_decrypt (crypt_handle, &doc, &out, &error);
    
    if (!ret) {
        printf("error: %s\n", error.message);
        Py_RETURN_FALSE;
    }

    return Py_BuildValue("s#", out.data, out.len);
}


static PyObject *
_setup (PyObject *self, PyObject *args) {
    char* aws_region, *aws_access_key_id, *aws_secret_access_key;
    if (!PyArg_ParseTuple(args, "sss", &aws_region, &aws_access_key_id, &aws_secret_access_key)) {
        Py_RETURN_FALSE;
    }

    if (crypt_handle) {
        printf("already set up\n");
        Py_RETURN_FALSE;
    }

    mongoc_crypt_init();

    mongoc_crypt_opts_t* opts = mongoc_crypt_opts_new();
    mongoc_crypt_opts_set_opt (opts, MONGOCRYPT_AWS_REGION, aws_region);
    mongoc_crypt_opts_set_opt (opts, MONGOCRYPT_AWS_ACCESS_KEY_ID, aws_access_key_id);
    mongoc_crypt_opts_set_opt (opts, MONGOCRYPT_AWS_SECRET_ACCESS_KEY, aws_secret_access_key);

    mongoc_crypt_error_t error;
    crypt_handle = mongoc_crypt_new(opts, &error);
    if (!crypt_handle) {
        printf("error: %s\n", error.message);
        Py_RETURN_FALSE;
    }

    Py_RETURN_TRUE;
}


static PyObject *
_cleanup(PyObject *self, PyObject *args) {
    mongoc_crypt_cleanup();
    Py_RETURN_TRUE;
}


static PyMethodDef CryptMethods[] = {
    {"setup", _setup, METH_VARARGS, "set options and initialize"},
    {"cleanup", _cleanup, METH_VARARGS, "set options and initialize"},
    {"encrypt", _encrypt, METH_VARARGS, "encrypt a document"},
    {"decrypt", _decrypt, METH_VARARGS, "decrypt a document"},
    {NULL, NULL, 0, NULL} /* sentinel */
};


PyMODINIT_FUNC
initpymongocrypt(void)
{
    (void) Py_InitModule("pymongocrypt", CryptMethods);
}
