#ifndef PY_NBAR_FITTING_H
#define PY_NBAR_FITTING_H 1

#include "Python.h"

PyObject* py_nbar_fitting_alloc(PyObject* self, PyObject* args);
PyObject* py_nbar_fitting_len(PyObject* self, PyObject* args);
PyObject* py_nbar_fitting_z(PyObject* self, PyObject* args);
PyObject* py_nbar_fitting_nbar_obs(PyObject* self, PyObject* args);
PyObject* py_nbar_fitting_nbar_hod(PyObject* self, PyObject* args);
PyObject* py_nbar_fitting_hod(PyObject* self, PyObject* args);
PyObject* py_nbar_fitting_compute(PyObject* self, PyObject* args);

#endif
