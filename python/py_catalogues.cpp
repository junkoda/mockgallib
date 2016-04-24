#include "py_assert.h"
#include "hod.h"
#include "py_catalogues.h"

static void py_catalogues_free(PyObject *obj);

PyObject* py_catalogues_alloc(PyObject* self, PyObject* args)
{
  Catalogues* const cats= new Catalogues;

  catalogue_init();

  return PyCapsule_New(cats, "_Catalogues", py_catalogues_free);  
}

void py_catalogues_free(PyObject *obj)
{
  Catalogues* const cats=
    (Catalogues*) PyCapsule_GetPointer(obj, "_Catalogues"); py_assert(cats);

  msg_printf(msg_debug, "freeing catalogues %x\n", cats);
  delete cats;
}

PyObject* py_catalogues_generate(PyObject* self, PyObject* args)
{
  // _catalogues_generate(_catalogues, _hod, _lightcone, z_min, z_max)
  PyObject *py_catalogues, *py_hod, *py_lightcones;
  double z_min, z_max;
  if(!PyArg_ParseTuple(args, "OOOdd", &py_catalogues, &py_hod, &py_lightcones,
		       &z_min, &z_max)) {
    return NULL;
  }

  Catalogues* const cats=
    (Catalogues*) PyCapsule_GetPointer(py_catalogues, "_Catalogues");
  py_assert(cats);

  Hod* const hod=
    (Hod*) PyCapsule_GetPointer(py_hod, "_HOD");
  py_assert(hod);

  LightCones* const lightcones=
    (LightCones*) PyCapsule_GetPointer(py_lightcones, "_LightCones");
  py_assert(lightcones);

  if(cats->empty())
    cats->allocate(lightcones->size());

  py_assert(cats->size() == lightcones->size());


  const size_t n= lightcones->size();

  for(size_t i=0; i<n; ++i) {
    catalogue_generate_mock(hod, lightcones->at(i),
			    z_min, z_max, cats->at(i));

    msg_printf(msg_verbose, "mock catalogue %lu generated with %lu galaxies\n",
	       i, cats->at(i)->size());
  }

  Py_RETURN_NONE;
}

PyObject* py_catalogues_len(PyObject* self, PyObject* args)
{
  PyObject* py_catalogues;
  if(!PyArg_ParseTuple(args, "O", &py_catalogues)) {
    return NULL;
  }

  Catalogues* const catalogues=
    (Catalogues*) PyCapsule_GetPointer(py_catalogues, "_Catalogues");
  py_assert(catalogues);

  return Py_BuildValue("i", (int) catalogues->size());
}