#ifndef PY_CATALOGUES_H
#define PY_CATALOGUES_H 1

#include "Python.h"
#include "catalogue.h"

PyMODINIT_FUNC
py_catalogues_module_init();

PyObject* py_catalogue_len(PyObject* self, PyObject* args);

PyObject* py_catalogues_alloc(PyObject* self, PyObject* args);
PyObject* py_catalogues_generate(PyObject* self, PyObject* args);
PyObject* py_catalogues_len(PyObject* self, PyObject* args);
PyObject* py_catalogues_catalogue(PyObject* self, PyObject* args);

#endif

