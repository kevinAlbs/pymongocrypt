#include <Python.h> /* must be included before any other headers. */
#include "pymongocrypt.h"
#include "mongocrypt.h"

/* Utilities */
static
void _status_to_exception (mongocrypt_status_t* status) {
    const char* msg = mongocrypt_status_message (status);
    PyErr_SetString(PyExc_RuntimeError, msg);
}

/* Types */
typedef struct {
    PyObject_HEAD
    mongocrypt_ctx_t* v;
} py_mongocrypt_ctx_t;

typedef struct {
    PyObject_HEAD
    mongocrypt_t* v;
} py_mongocrypt_t;

typedef struct {
    PyObject_HEAD
    mongocrypt_binary_t* v;
    PyObject* owned_binary;
} py_mongocrypt_binary_t;

typedef struct {
    PyObject_HEAD
    mongocrypt_kms_ctx_t* v;
} py_mongocrypt_kms_ctx_t;

typedef struct {
    PyObject_HEAD
    mongocrypt_opts_t* v;
} py_mongocrypt_opts_t;


/* mongocrypt_kms_ctx_t */
PyObject* py_mongocrypt_kms_ctx_message (PyObject *self, PyObject *args) {
    py_mongocrypt_kms_ctx_t* ctx = (py_mongocrypt_kms_ctx_t*) self;
    py_mongocrypt_binary_t* bin;

    /* TODO: check type. */
    if (!PyArg_ParseTuple(args, "O", (PyObject**)&bin)) {
        return NULL;
    }

    if (!mongocrypt_kms_ctx_message (ctx->v, bin->v)) {
        mongocrypt_status_t *status = mongocrypt_status_new();
        mongocrypt_kms_ctx_status (ctx->v, status);
        _status_to_exception (status);
        mongocrypt_status_destroy (status);
        return NULL;
    }
    
    Py_RETURN_NONE;
}


PyObject* py_mongocrypt_kms_ctx_bytes_needed (PyObject *self, PyObject *args) {
    py_mongocrypt_kms_ctx_t* ctx = (py_mongocrypt_kms_ctx_t*) self;

    return Py_BuildValue("i", (int) mongocrypt_kms_ctx_bytes_needed (ctx->v));
}


PyObject* py_mongocrypt_kms_ctx_feed (PyObject *self, PyObject *args) {
    py_mongocrypt_kms_ctx_t* ctx = (py_mongocrypt_kms_ctx_t*) self;
    py_mongocrypt_binary_t* bin;

    if (!PyArg_ParseTuple(args, "O", (PyObject**)&bin)) {
        return NULL;
    }

    if (!mongocrypt_kms_ctx_feed (ctx->v, bin->v)) {
        mongocrypt_status_t *status = mongocrypt_status_new();
        mongocrypt_kms_ctx_status (ctx->v, status);
        _status_to_exception (status);
        mongocrypt_status_destroy (status);
        return NULL;
    }
    
    Py_RETURN_NONE;
}


static PyMethodDef py_mongocrypt_kms_ctx_methods[] = {
    { "message", py_mongocrypt_kms_ctx_message, METH_VARARGS, "get message" },
    { "bytes_needed", py_mongocrypt_kms_ctx_bytes_needed, METH_VARARGS, "get bytes needed" },
    { "feed", py_mongocrypt_kms_ctx_feed, METH_VARARGS, "feed data back" },
    {NULL, NULL, 0, NULL} /* sentinel */
};


static PyTypeObject py_mongocrypt_kms_ctx_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "KMSCtx",
    .tp_doc = "For a single KMS request",
    .tp_basicsize = sizeof(py_mongocrypt_kms_ctx_t),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyType_GenericNew,
    .tp_methods = py_mongocrypt_kms_ctx_methods
};


/* mongocrypt_ctx_t */
int
py_mongocrypt_ctx_init (PyObject *self, PyObject *args, PyObject *kwds)
{
    py_mongocrypt_ctx_t* ctx = (py_mongocrypt_ctx_t*) self;
    py_mongocrypt_t* crypt; 

    if (!PyArg_ParseTuple(args, "O", &crypt))
        return -1;

    ctx->v = mongocrypt_ctx_new (crypt->v);
    return 0;
}


void py_mongocrypt_ctx_py_finalize (PyObject *self) {
    py_mongocrypt_ctx_t* ctx = (py_mongocrypt_ctx_t*) self;
    mongocrypt_ctx_destroy (ctx->v);
}


PyObject* py_mongocrypt_ctx_state (PyObject *self, PyObject *args) {
    py_mongocrypt_ctx_t* ctx = (py_mongocrypt_ctx_t*) self;
    return Py_BuildValue("i", (int) mongocrypt_ctx_state (ctx->v));
}


PyObject* py_mongocrypt_ctx_mongo_op (PyObject *self, PyObject *args) {
    py_mongocrypt_ctx_t* ctx = (py_mongocrypt_ctx_t*) self;
    py_mongocrypt_binary_t* bin;

    if (!PyArg_ParseTuple(args, "O", (PyObject**)&bin)) {
        return NULL;
    }

    if (!mongocrypt_ctx_mongo_op (ctx->v, bin->v)) {
        mongocrypt_status_t *status = mongocrypt_status_new();
        mongocrypt_ctx_status (ctx->v, status);
        _status_to_exception (status);
        mongocrypt_status_destroy (status);
        return NULL;
    }
    
    Py_RETURN_NONE;
}


PyObject* py_mongocrypt_ctx_mongo_feed (PyObject *self, PyObject *args) {
    py_mongocrypt_ctx_t* ctx = (py_mongocrypt_ctx_t*) self;
    py_mongocrypt_binary_t* bin;

    if (!PyArg_ParseTuple(args, "O", (PyObject**)&bin)) {
        return NULL;
    }

    Py_INCREF ((PyObject*) bin);

    char* data = (char*)mongocrypt_binary_data(bin->v);
    printf("in python, feeding:\n");
    for (int i = 0; i < mongocrypt_binary_len(bin->v); i++) {
        printf("%c", data[i]);
    }
    fflush (stdout);

    if (!mongocrypt_ctx_mongo_feed (ctx->v, bin->v)) {
        mongocrypt_status_t *status = mongocrypt_status_new();
        mongocrypt_ctx_status (ctx->v, status);
        _status_to_exception (status);
        mongocrypt_status_destroy (status);
        return NULL;
    }

    Py_DECREF ((PyObject*) bin);
    
    Py_RETURN_NONE;
}


PyObject* py_mongocrypt_ctx_mongo_done (PyObject *self, PyObject *args) {
    py_mongocrypt_ctx_t* ctx = (py_mongocrypt_ctx_t*) self;

    if (!mongocrypt_ctx_mongo_done (ctx->v)) {
        mongocrypt_status_t *status = mongocrypt_status_new();
        mongocrypt_ctx_status (ctx->v, status);
        _status_to_exception (status);
        mongocrypt_status_destroy (status);
        return NULL;
    }
    
    Py_RETURN_NONE;
}


static
PyObject* py_mongocrypt_ctx_next_kms_ctx (PyObject* self, PyObject* args) {
    py_mongocrypt_ctx_t* ctx = (py_mongocrypt_ctx_t*) self;
    mongocrypt_kms_ctx_t* kms = mongocrypt_ctx_next_kms_ctx (ctx->v);
    if (!kms) {
        Py_RETURN_NONE;
    }

    py_mongocrypt_kms_ctx_t *kms_ctx = (py_mongocrypt_kms_ctx_t*) PyObject_CallObject((PyObject *) &py_mongocrypt_kms_ctx_type, NULL);
    kms_ctx->v = kms;
    return (PyObject*) kms_ctx;
}


static
PyObject* py_mongocrypt_ctx_kms_done (PyObject* self, PyObject* args) {
    py_mongocrypt_ctx_t* ctx = (py_mongocrypt_ctx_t*) self;

    if (!mongocrypt_ctx_kms_done (ctx->v)) {
        mongocrypt_status_t *status = mongocrypt_status_new();
        mongocrypt_ctx_status (ctx->v, status);
        _status_to_exception (status);
        mongocrypt_status_destroy (status);
        return NULL;
    }

    Py_RETURN_NONE;
}


static
PyObject* py_mongocrypt_ctx_finalize (PyObject* self, PyObject* args) {
    py_mongocrypt_ctx_t* ctx = (py_mongocrypt_ctx_t*) self;
    py_mongocrypt_binary_t* bin;

    if (!PyArg_ParseTuple(args, "O", (PyObject**)&bin)) {
        return NULL;
    }

    if (!mongocrypt_ctx_finalize (ctx->v, bin->v)) {
        mongocrypt_status_t *status = mongocrypt_status_new();
        mongocrypt_ctx_status (ctx->v, status);
        _status_to_exception (status);
        mongocrypt_status_destroy (status);
        return NULL;
    }

    Py_RETURN_NONE;
}


static PyMethodDef py_mongocrypt_ctx_methods[] = {
    { "state", py_mongocrypt_ctx_state, METH_VARARGS, "get state" },
    { "mongo_op", py_mongocrypt_ctx_mongo_op, METH_VARARGS, "get current mongo op" },
    { "mongo_feed", py_mongocrypt_ctx_mongo_feed, METH_VARARGS, "feed data back" },
    { "mongo_done", py_mongocrypt_ctx_mongo_done, METH_VARARGS, "done" },
    { "next_kms_ctx", py_mongocrypt_ctx_next_kms_ctx, METH_VARARGS, "get next KMS context" },
    { "kms_done", py_mongocrypt_ctx_kms_done, METH_VARARGS, "done with all KMS contexts" },
    { "finalize", py_mongocrypt_ctx_finalize, METH_VARARGS, "finalize this bad boy" },
    {NULL, NULL, 0, NULL} /* sentinel */
};


static PyTypeObject py_mongocrypt_ctx_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Ctx",
    .tp_doc = "For encryption/decryption",
    .tp_basicsize = sizeof(py_mongocrypt_ctx_t),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyType_GenericNew,
    .tp_init = py_mongocrypt_ctx_init,
    .tp_finalize = py_mongocrypt_ctx_py_finalize,
    .tp_methods = py_mongocrypt_ctx_methods
};

int py_mongocrypt_opts_init (PyObject *self, PyObject *args, PyObject *kwds) {
    py_mongocrypt_opts_t* opts = (py_mongocrypt_opts_t*) self;
    opts->v = mongocrypt_opts_new ();
    return 0;
}

PyObject* py_mongocrypt_opts_set (PyObject *self, PyObject *args) {
    py_mongocrypt_opts_t* opts = (py_mongocrypt_opts_t*) self;
    int type;
    char* str;

    PyArg_ParseTuple (args, "is", &type, &str); /* only support string. */

    mongocrypt_opts_set_opt (opts->v, (mongocrypt_opt_t)type, (void*) str);

    Py_RETURN_NONE;
}

void py_mongocrypt_opts_finalize (PyObject *self) {
    py_mongocrypt_opts_t* opts = (py_mongocrypt_opts_t*) self;
    mongocrypt_opts_destroy (opts->v);
}

static PyMethodDef py_mongocrypt_opts_methods[] = {
    { "set", py_mongocrypt_opts_set, METH_VARARGS, "set an option" },
    {NULL, NULL, 0, NULL} /* sentinel */
};

/* mongocrypt_opts_t */
static PyTypeObject py_mongocrypt_opts_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Opts",
    .tp_doc = "Options for initialization",
    .tp_basicsize = sizeof(py_mongocrypt_opts_t),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyType_GenericNew,
    .tp_init = py_mongocrypt_opts_init,
    .tp_finalize = py_mongocrypt_opts_finalize,
    .tp_methods = py_mongocrypt_opts_methods
};


/* mongocrypt_t */
int py_mongocrypt_init(PyObject *self, PyObject *args, PyObject *kwds) {
    py_mongocrypt_t* crypt = (py_mongocrypt_t*) self;
    py_mongocrypt_opts_t* opts;
    
    crypt->v = mongocrypt_new ();

    PyArg_ParseTuple (args, "O", (PyObject*)&opts);
    /* TODO: check type. */

    if (!mongocrypt_init (crypt->v, opts->v)) {
        mongocrypt_status_t *status = mongocrypt_status_new();
        mongocrypt_status (crypt->v, status);
        _status_to_exception (status);
        mongocrypt_status_destroy (status);
        return -1;
    }
    return 0;
}


void py_mongocrypt_finalize (PyObject *self) {
    py_mongocrypt_t* crypt = (py_mongocrypt_t*) self;

    mongocrypt_destroy (crypt->v);
}


PyObject* py_mongocrypt_encrypt (PyObject* self, PyObject* args) {
    const char* ns;
    if (!PyArg_ParseTuple (args, "s", &ns)) {
        return NULL;
    }
    PyObject *arg_list = Py_BuildValue("(O)", self);
    py_mongocrypt_ctx_t *ctx = (py_mongocrypt_ctx_t*) PyObject_CallObject((PyObject *) &py_mongocrypt_ctx_type, arg_list);
    mongocrypt_ctx_encrypt_init (ctx->v, ns, strlen(ns));
    Py_DECREF (arg_list);
    return (PyObject*) ctx;
}


PyObject* py_mongocrypt_decrypt (PyObject* self, PyObject* args) {
    py_mongocrypt_binary_t* doc;
    if (!PyArg_ParseTuple (args, "O", (PyObject*)&doc)) {
        return NULL;
    }
    PyObject *arg_list = Py_BuildValue("(O)", self);
    py_mongocrypt_ctx_t *ctx = (py_mongocrypt_ctx_t*) PyObject_CallObject((PyObject *) &py_mongocrypt_ctx_type, arg_list);
    mongocrypt_ctx_decrypt_init (ctx->v, doc->v);
    Py_DECREF (arg_list);
    return (PyObject*) ctx;
}


static PyMethodDef py_mongocrypt_methods[] = {
    { "encrypt", py_mongocrypt_encrypt, METH_VARARGS, "start encryption" },
    { "decrypt", py_mongocrypt_decrypt, METH_VARARGS, "start decryption" },
    {NULL, NULL, 0, NULL} /* sentinel */
};


static PyTypeObject py_mongocrypt_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Mongocrypt",
    .tp_doc = "Top level handle",
    .tp_basicsize = sizeof(py_mongocrypt_t),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_FINALIZE,
    .tp_new = PyType_GenericNew,
    .tp_init = py_mongocrypt_init,
    .tp_finalize = py_mongocrypt_finalize,
    .tp_methods = py_mongocrypt_methods
};


/* mongocrypt_binary_t */
int py_mongocrypt_binary_init(PyObject *self, PyObject *args, PyObject *kwds) {
    py_mongocrypt_binary_t* bin = (py_mongocrypt_binary_t*) self;
    PyObject* input;
    
    if (!PyArg_ParseTuple (args, "|O", &input)) {
        return -1;
    }

    if (input && PyBytes_Check(input)) {
        Py_INCREF (input); /* TODO: store this in my binary object. */
        printf("this is a bytes object\n");
        bin->v = mongocrypt_binary_new_from_data((uint8_t*)PyBytes_AsString (input), (uint32_t)PyBytes_Size (input));
        bin->owned_binary = input;
    } else {
        printf("no\n");
        bin->owned_binary = NULL;
        bin->v = mongocrypt_binary_new ();
    }

    return 0;
}


void py_mongocrypt_binary_finalize (PyObject *self) {
    py_mongocrypt_binary_t* bin = (py_mongocrypt_binary_t*) self;
    if (bin->owned_binary) {
        Py_DECREF(bin->owned_binary);
    }
    mongocrypt_binary_destroy (bin->v);
}


static PyObject* pymongocrypt_binary_len (PyObject* self, PyObject* args) {
    py_mongocrypt_binary_t* bin = (py_mongocrypt_binary_t*) self;

    return Py_BuildValue("i", (int) mongocrypt_binary_len (bin->v));
}


static PyObject* pymongocrypt_binary_data (PyObject* self, PyObject* args) {
    py_mongocrypt_binary_t* bin = (py_mongocrypt_binary_t*) self;

    return Py_BuildValue("y#", (char*) mongocrypt_binary_data (bin->v), (int) mongocrypt_binary_len (bin->v));
}


static PyMethodDef py_mongocrypt_binary_methods[] = {
    { "len", pymongocrypt_binary_len, METH_VARARGS, "get the damn length" },
    { "data", pymongocrypt_binary_data, METH_VARARGS, "get the damn data" },
    {NULL, NULL, 0, NULL} /* sentinel */
};


static PyTypeObject py_mongocrypt_binary_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Binary",
    .tp_doc = "A non-owning view to binary data",
    .tp_basicsize = sizeof(py_mongocrypt_binary_t),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_FINALIZE,
    .tp_new = PyType_GenericNew,
    .tp_init = py_mongocrypt_binary_init,
    .tp_finalize = py_mongocrypt_binary_finalize,
    .tp_methods = py_mongocrypt_binary_methods
};


static PyObject *
_version(PyObject *self, PyObject *args)
{
    return Py_BuildValue("s", mongocrypt_version());
}


static PyMethodDef crypt_methods[] = {
    {"version", _version, METH_VARARGS, "get the version for libmongocrypt"},
    {NULL, NULL, 0, NULL} /* sentinel */
};


static struct PyModuleDef crypt_module = {
    PyModuleDef_HEAD_INIT,
    "pymongocrypt", /* name of module */
    "docs",         /* module documentation, may be NULL */
    -1,             /* size of per-interpreter state of the module, or -1 if the module keeps state in global variables. */
    crypt_methods};


PyMODINIT_FUNC
PyInit_pymongocrypt(void)
{
    PyObject *module;

    module = PyModule_Create(&crypt_module);

    if (PyType_Ready(&py_mongocrypt_type) < 0)
        return NULL;

    PyModule_AddObject(module, "Mongocrypt", (PyObject *) &py_mongocrypt_type);

    if (PyType_Ready(&py_mongocrypt_ctx_type) < 0)
        return NULL;

    PyModule_AddObject(module, "Ctx", (PyObject *) &py_mongocrypt_ctx_type);

    if (PyType_Ready(&py_mongocrypt_binary_type) < 0)
        return NULL;

    PyModule_AddObject(module, "Binary", (PyObject *) &py_mongocrypt_binary_type);

    if (PyType_Ready(&py_mongocrypt_kms_ctx_type) < 0)
        return NULL;

    PyModule_AddObject(module, "KMSCtx", (PyObject *) &py_mongocrypt_kms_ctx_type);

    if (PyType_Ready(&py_mongocrypt_opts_type) < 0)
        return NULL;

    PyModule_AddObject(module, "Opts", (PyObject *) &py_mongocrypt_opts_type);

    return module;
}
