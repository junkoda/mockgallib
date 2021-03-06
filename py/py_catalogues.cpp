#include <iostream>
#include <stdexcept>

#include "catalogue.h"
#include "error.h"
#include "hdf5_io.h"
#include "distance.h"
#include "py_assert.h"
#include "py_sky.h"
#include "py_catalogues.h"


#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include "numpy/arrayobject.h"

using namespace std;

PyMODINIT_FUNC
py_catalogues_module_init()
{
  import_array();

  return NULL;
}

static void py_catalogues_free(PyObject *obj);


PyObject* py_catalogues_alloc(PyObject* self, PyObject* args)
{
  Catalogues* const cats= new Catalogues();

  return PyCapsule_New(cats, "_Catalogues", py_catalogues_free);  
}

void py_catalogues_free(PyObject *obj)
{
  Catalogues* const cats=
    (Catalogues*) PyCapsule_GetPointer(obj, "_Catalogues");
  py_assert_void(cats);

  msg_printf(msg_debug, "freeing catalogues %x\n", cats);
  delete cats;
}

PyObject* py_catalogues_generate(PyObject* self, PyObject* args)
{
  // _catalogues_generate_galaxies(_catalogues, _hod, _lightcone, _sky, z_min, z_max, random)
  // random: 0 for mock catalogue, 1 for randoms
  
  PyObject *py_catalogues, *py_hod, *py_lightcones, *py_sky;
  double z_min, z_max;
  int random;
  if(!PyArg_ParseTuple(args, "OOOOddi", &py_catalogues, &py_hod, &py_lightcones,
		       &py_sky,
		       &z_min, &z_max, &random)) {
    return NULL;
  }

  Catalogues* const cats=
    (Catalogues*) PyCapsule_GetPointer(py_catalogues, "_Catalogues");
  py_assert_ptr(cats);

  Hod* const hod=
    (Hod*) PyCapsule_GetPointer(py_hod, "_HOD");
  py_assert_ptr(hod);

  LightCones* const lightcones=
    (LightCones*) PyCapsule_GetPointer(py_lightcones, "_LightCones");
  py_assert_ptr(lightcones);

  Sky* const sky=
    (Sky*) PyCapsule_GetPointer(py_sky, "_Sky"); py_assert_ptr(sky);

  if(cats->empty())
    cats->allocate(lightcones->size());

  py_assert_ptr(cats->size() == lightcones->size());


  const size_t n= lightcones->size();

  if(random) {
    for(size_t i=0; i<n; ++i) {
      catalogue_generate_random(hod, lightcones->at(i), sky,
				z_min, z_max, cats->at(i));
      msg_printf(msg_verbose,
		 "random catalogue %lu generated with %lu galaxies\n",
		 i, cats->at(i)->size());
      
    }
  }
  else {
    for(size_t i=0; i<n; ++i) {
      catalogue_generate_mock(hod, lightcones->at(i), sky,
			      z_min, z_max, cats->at(i));
      
      msg_printf(msg_verbose,
		 "mock catalogue %lu generated with %lu galaxies\n",
		 i, cats->at(i)->size());
    }
  }

  Py_RETURN_NONE;
}


PyObject* py_catalogues_len(PyObject* self, PyObject* args)
{
  // return number of catalogue in the catalogues
  PyObject* py_catalogues;
  if(!PyArg_ParseTuple(args, "O", &py_catalogues)) {
    return NULL;
  }

  Catalogues* const catalogues=
    (Catalogues*) PyCapsule_GetPointer(py_catalogues, "_Catalogues");
  py_assert_ptr(catalogues);

  return Py_BuildValue("i", (int) catalogues->size());
}


PyObject* py_catalogues_catalogue(PyObject* self, PyObject* args)
{
  // _catalogues_catalogue(_cats, i)
  // return ith catalogue as an np.array
  
  PyObject* py_catalogues;
  int i;
  if(!PyArg_ParseTuple(args, "Oi", &py_catalogues, &i)) {
    return NULL;
  }

  Catalogues* const catalogues=
    (Catalogues*) PyCapsule_GetPointer(py_catalogues, "_Catalogues");
  py_assert_ptr(catalogues);

  py_assert_ptr(sizeof(Particle) % sizeof(double) == 0);

  Catalogue* cat= 0;
  try {
    cat= catalogues->at(i);
  }
  catch(const std::out_of_range) {
    PyErr_SetNone(PyExc_IndexError);
    return NULL;
  }

  
  int nd=2;
  int ncol= sizeof(Particle)/sizeof(float);
  npy_intp dims[]= {(npy_intp) cat->size(), ncol};

  return PyArray_SimpleNewFromData(nd, dims, NPY_FLOAT, &(cat->front()));
}


PyObject* py_catalogues_append(PyObject* self, PyObject* args)
{
  // _catalogues_append(_catalogues, array)
  // array must have 5 or 6 columns, x,y,z,ra,dec,w
  
  PyObject *py_catalogues, *py_array;
  double z_min, z_max;
  if(!PyArg_ParseTuple(args, "OOdd",
		       &py_catalogues, &py_array, &z_min, &z_max)) {
    return NULL;
  }

  Catalogues* const cats=
    (Catalogues*) PyCapsule_GetPointer(py_catalogues, "_Catalogues");
  py_assert_ptr(cats);

  Py_buffer view;

  if(PyObject_GetBuffer(py_array, &view, PyBUF_FORMAT | PyBUF_FULL_RO) == -1) {
    return NULL;
  }

  //
  // Decode array information
  //
  if(view.ndim != 2) {
    PyErr_SetString(PyExc_TypeError, "Expected a 2-dimensional array");
    PyBuffer_Release(&view);
    return NULL;
  }

  if(strcmp(view.format, "d") != 0) {
    PyErr_SetString(PyExc_TypeError, "Expected an array of doubles");
    PyBuffer_Release(&view);
    return NULL;
  }

  if(view.suboffsets) {
    PyErr_SetString(PyExc_TypeError,
		    "Array suboffsets not handled in catalogues_append");

    PyBuffer_Release(&view);
    return NULL;
  }

  
  const int n= view.shape[0];
  const int ncol= view.shape[1];

  if(n == 0) {
    PyErr_SetString(PyExc_TypeError, "no data in nbar_obs array");
    PyBuffer_Release(&view);
    return NULL;
  }
  
  if(!(ncol == 5 || ncol == 6)) {
    PyErr_SetString(PyExc_TypeError,
	      "Expected 5 or 6 columns x,y,z,ra,dec,[weight] for a catalogue");
    PyBuffer_Release(&view);
    return NULL;
  }

  double* a= (double*) view.buf;
  
  const size_t next_row= view.strides[0]/sizeof(double);
  const size_t next_col= view.strides[1]/sizeof(double);

  // New Catalogue
  Catalogue* const cat= new Catalogue();
  cat->reserve(n);
  Particle p;
  p.vr= 0; p.M= 0; p.flag= 0; p.w= 1;
  p.flag= 0.0f; p.rsat= 0.0f, p.vsat= 0.0f;
  
  
  for(int i=0; i<n; ++i) {
    p.x[0]= *a;
    p.x[1]= *(a + next_col);
    p.x[2]= *(a + 2*next_col);
    p.radec[0]= *(a + 3*next_col);
    p.radec[1]= *(a + 4*next_col);

    if(ncol >= 6)
      p.w= *(a + 5*next_col);

    //fprintf(stderr, "%e %e %e %e %e %e\n", p.x[0], p.x[1], p.x[2], p.radec[0], p.radec[1], p.w);

    float r= sqrt(p.x[0]*p.x[0] + p.x[1]*p.x[2] + p.x[2]*p.x[2]);
    p.z= distance_redshift(r);
    
    if(z_min >= 0 && z_min <= p.z && p.z < z_max)
      cat->push_back(p);
  
    a += next_row;
  }

  cats->push_back(cat);

  msg_printf(msg_verbose, "%lu points added to catalogue\n", cat->size());

  PyBuffer_Release(&view);
  
  Py_RETURN_NONE;
}

PyObject* py_catalogues_ngal(PyObject* self, PyObject* args)
{
  // _catalogues_nz(_catalogues)
  // return average number of centrals and satellites
  
  PyObject *py_catalogues;
  if(!PyArg_ParseTuple(args, "O", &py_catalogues)) {
    return NULL;
  }

  Catalogues* const cats=
    (Catalogues*) PyCapsule_GetPointer(py_catalogues, "_Catalogues");
  py_assert_ptr(cats);

  double ncen_sum= 0.0;
  double nsat_sum= 0.0;
  const int n= cats->size();

  for(Catalogues::iterator c= cats->begin(); c != cats->end(); ++c) {
    ncen_sum += (*c)->ncen;
    nsat_sum += (*c)->nsat;
  }

  return Py_BuildValue("(dd)", ncen_sum/n, nsat_sum/n);
}

PyObject* py_catalogues_nz(PyObject* self, PyObject* args)
{
  // _catalogues_nz(_catalogues, z_min, z_max, nbin)
  // return an array of [z, dN/dz] average over all catalogues
  //     z:     midpoint of redshift bin
  //     dN/dz: number of galaxies / dz
  
  PyObject *py_catalogues;
  double z_min, z_max;
  int nbin;
  if(!PyArg_ParseTuple(args, "Oddi", &py_catalogues,
		       &z_min, &z_max, &nbin)) {
    return NULL;
  }

  Catalogues* const cats=
    (Catalogues*) PyCapsule_GetPointer(py_catalogues, "_Catalogues");
  py_assert_ptr(cats);

  npy_intp dims[]= {nbin, 2};
  PyObject* arr= PyArray_SimpleNew(2, dims, NPY_DOUBLE);
  double* a= (double*) PyArray_DATA((PyArrayObject*) arr);

  for(int i=0; i<nbin; ++i) {
    a[2*i]= z_min + (z_max - z_min)/nbin*(i + 0.5);
    a[2*i + 1]= 0.0;
  }

  double ncat= cats->size();
  for(Catalogues::iterator cat= cats->begin(); cat != cats->end(); ++cat) {
    double* nz= catalogue_compute_nz(*cat, z_min, z_max, nbin);

    for(int i=0; i<nbin; ++i)
      a[2*i + 1]= nz[i]/ncat;
    
    free(nz);
  }

  return arr;
}

PyObject* py_catalogues_load_h5(PyObject* self, PyObject* args)
{
  PyObject *py_catalogues, *bytes;;
  
  if(!PyArg_ParseTuple(args, "OO&",
		       &py_catalogues, PyUnicode_FSConverter, &bytes)) {
    return NULL;
  }

  Catalogues* const cats=
    (Catalogues*) PyCapsule_GetPointer(py_catalogues, "_Catalogues");
  py_assert_ptr(cats);

  char* filename;
  Py_ssize_t len;
  PyBytes_AsStringAndSize(bytes, &filename, &len);

  Catalogue* const cat= new Catalogue();
  
  try {
    hdf5_read_catalogue(filename, cat);
  }
  catch(IOError) {
    Py_DECREF(bytes);
    PyErr_SetNone(PyExc_FileNotFoundError);
    return NULL;
  }
  
  if(!cats->empty())
    cats->push_back(cat);
  else
    delete cats;

  Py_DECREF(bytes);

  Py_RETURN_NONE;
}

PyObject* py_catalogues_subsample(PyObject* self, PyObject* args)
{
  // _catalogues_subample(_cats, n)
  // subsample catalogue to n points
  
  PyObject *py_catalogues;
  int n;
  if(!PyArg_ParseTuple(args, "Oi", &py_catalogues, &n)) {
    return NULL;
  }

  Catalogues* const cats=
    (Catalogues*) PyCapsule_GetPointer(py_catalogues, "_Catalogues");
  py_assert_ptr(cats);

  for(Catalogues::iterator c= cats->begin(); c != cats->end(); ++c)
    catalogue_subsample(*c, n);

  Py_RETURN_NONE;
}

