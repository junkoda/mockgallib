//
// Information for Python
//
#include <iostream>
#include "Python.h"

#include "py_msg.h"
#include "py_const.h"
#include "py_cosmology.h"
#include "py_power.h"
#include "py_growth.h"
#include "py_sigma.h"
#include "py_mf.h"
#include "py_mf_cumulative.h"
#include "py_hod.h"
#include "py_nbar.h"
#include "py_nbar_fitting.h"
#include "py_lightcones.h"
#include "py_catalogues.h"
#include "py_sky.h"
#include "py_distance.h"
#include "py_remap.h"
#include "py_slice.h"
#include "py_halo_mass.h"
#include "py_snapshots.h"
#include "py_cola_lightcones.h"
#include "py_halo_concentration.h"
#include "py_rand.h"
#include "py_corr_projected.h"
#include "py_corr_multipole.h"
#include "py_minimise.h"
#include "py_comm.h"
#include "py_array.h"
#include "py_callback.h"

using namespace std;

static PyMethodDef methods[] = {
  {"set_loglevel", py_msg_set_loglevel, METH_VARARGS,
   "set loglevel: 0=debug, 1=verbose, ..."},

  {"_const_G", py_const_G, METH_VARARGS, "return G in internal unit"},
  {"_const_rhocrit0", py_const_rhocrit0, METH_VARARGS,
   "return critical density at z=0"},
  {"_const_deltac", py_const_deltac, METH_VARARGS,
   "return critial overdensity"},

  {"_cosmology_set", py_cosmology_set, METH_VARARGS,
   "set omega_m0"},
  {"_cosmology_rhom", py_cosmology_rhom, METH_VARARGS,
   "get mean comoving matter density"},
  {"_cosmology_compute_comoving_distance",
   py_cosmology_compute_comoving_distance, METH_VARARGS,
   "compute comoving distance corresponds to scale factor a"},
   
  {"_power_init", py_power_init, METH_VARARGS,
   "read power spectrum from file"},
  {"_power_free", py_power_free, METH_VARARGS,
   "free power spectrum module"},
  {"_power_sigma", py_power_sigma, METH_VARARGS,
   "compute sigma(R); density fluctuation smoothed with radius R"},
  {"_power_n", py_power_n, METH_VARARGS,
   "return number of power spectrum rows"},
  {"_power_k", py_power_k, METH_VARARGS,
   "return wavenumber array k"},
  {"_power_P", py_power_P, METH_VARARGS,
   "return power spectrum array P"},
  {"_power_ki", py_power_ki, METH_VARARGS,
   "return k[i]"},
  {"_power_Pi", py_power_Pi, METH_VARARGS,
   "return P[i]"},

  {"growth_D", py_growth_D, METH_VARARGS,
   "growth_D(a); linear growth factor at scale factor a"},


  {"_sigma_init", py_sigma_init, METH_VARARGS,
   "allocate sigma(M) module"},
  {"_sigma_M", py_sigma_M, METH_VARARGS,
   "get sigma0(M)"},
  {"_sigma_n", py_sigma_n, METH_VARARGS,
   "get number of M/sigma0 array"},
  {"_sigma_M_range", py_sigma_M_range, METH_VARARGS,
   "get range in M"},
  {"_sigma_0inv", py_sigma_0inv, METH_VARARGS,
   "get 1/sigma0(M)"},
  {"_sigma_M_array", py_sigma_M_array, METH_VARARGS,
   "get an array of M"},
  {"_sigma_sinv_array", py_sigma_sinv_array, METH_VARARGS,
   "get an array of 1/sigma0"},

  {"_mf_alloc", py_mf_alloc, METH_VARARGS,
   "allocate mass function module"},
  {"_mf_set_redshift", py_mf_set_redshift, METH_VARARGS,
   "set redshift for the mass function"},   
  {"_mf_f", py_mf_f, METH_VARARGS,
   "compute f(nu)"},

  {"_mf_cumulative_alloc", py_mf_cumulative_alloc, METH_VARARGS,
   "allocate cumulative mass function"},

  {"_hod_alloc", py_hod_alloc, METH_VARARGS, 
   "allocate an hod object"},
  {"_hod_get_coef_all", py_hod_get_coef_all, METH_VARARGS,
   "get all HOD coefficients c[]"},
  {"_hod_get_coef", py_hod_get_coef,  METH_VARARGS,
   "get HOD coefficient c[i]; _hod_set_coef(_hod, i)"},
  {"_hod_set_coef", py_hod_set_coef,  METH_VARARGS,
   "set HOD coefficient c[i]; _hod_set_coef(_hod, i, ci)"},
  {"_hod_ncen", py_hod_ncen, METH_VARARGS,
   "py_hod_ncen(_hod, M, z); mean number of central galaxies"},
  {"_hod_nsat", py_hod_nsat, METH_VARARGS,
   "_hod_nsat(_hod, M, z); mean number of satellite galaxies"},
  {"_hod_get_z0", py_hod_get_z0, METH_VARARGS,
   "_hod_get_z0(_hod)"},
  {"_hod_compute_logMmin", py_hod_compute_logMmin, METH_VARARGS,
   "_hod_compute_logMmin(_hod, z)"},
  {"_hod_compute_M1", py_hod_compute_M1, METH_VARARGS,
   "_hod_compute_M1(_hod, z)"},
  {"_hod_compute_sigma", py_hod_compute_sigma, METH_VARARGS,
   "_hod_compute_sigma(_hod, z)"},
  {"_hod_compute_alpha", py_hod_compute_alpha, METH_VARARGS,
   "_hod_compute_alpha(_hod, z)"},
  
  {"_nbar_alloc", py_nbar_alloc, METH_VARARGS,
   "_nbar_alloc(_ps, _hod); allocate an nbar integration object"},
  {"_nbar_compute", py_nbar_compute, METH_VARARGS,
   "_nbar_compute(_ni, z); compute nbar(z)"},
  {"_nbar_ncen_compute", py_nbar_ncen_compute, METH_VARARGS,
   "_nbar_ncen_compute(_ni, z); compute ncen_bar(z)"},
  {"_nbar_nsat_compute", py_nbar_nsat_compute, METH_VARARGS,
   "_nbar_nsat_compute(_ni, z); compute nsat_bar(z)"},

  {"_nbar_fitting_alloc", py_nbar_fitting_alloc, METH_VARARGS,
   "_nbar_fitting_alloc(_ps, _hod, array_obs, z_min, z_max); allocate an nbar fitting object"},
  {"_nbar_fitting_len", py_nbar_fitting_len, METH_VARARGS,
   "number of nbar_obs in given redshift range"},
  {"_nbar_fitting_z", py_nbar_fitting_z, METH_VARARGS,
   "_nbar_fitting_z(_fitting, z); get z[i]"},
  {"_nbar_fitting_nbar_obs", py_nbar_fitting_nbar_obs, METH_VARARGS,
   "_nbar_fitting_nbar_obs(_fitting, z); get nbar_obs[i]"},
  {"_nbar_fitting_nbar_hod", py_nbar_fitting_nbar_hod, METH_VARARGS,
   "_nbar_fitting_nbar_hod(_fitting, z); get nbar_hod[i]"},
  {"_nbar_fitting_hod", py_nbar_fitting_hod, METH_VARARGS,
   "return pointer to _hod"},

  {"_nbar_fitting_compute", py_nbar_fitting_compute, METH_VARARGS,
   "_nbar_fitting_compute(_fitting); perform fitting"},

  {"_lightcones_alloc", py_lightcones_alloc, METH_VARARGS,
   "allocate a new _LightCones"},
  {"_lightcones_load_h5", py_lightcones_load_h5, METH_VARARGS,
   "_lightcones_load_h5(_lightcones, filename); read a lightcone from file"},
  {"_lightcones_len", py_lightcones_len, METH_VARARGS,
   "number of lightcones in _LightCones"},
  {"_lightcones_lightcone", py_lightcones_lighcone, METH_VARARGS,
   "_lightcones_lightcone(_lightcones, i); get ith lightcone"},
  {"_lightcones_clear", py_lightcones_clear, METH_VARARGS,
   "_lightcones_clear(_lightcones); clear all lightcone in _lightcones"},

  {"_lightcone_len", py_lightcone_len, METH_VARARGS,
   "_lightcone_len(_lightcone); number of haloes in _lightcone"},
  {"_lightcone_as_array", py_lightcone_as_array, METH_VARARGS,
   "_lightcone_as_array(_lightcone); return haloes as a np array"},
  {"_lightcone_save_h5", py_lightcone_save_h5, METH_VARARGS,
   "_lightcone_save_h5(_lightcone, filename); save lightcone as HDF5 file"},
  {"_lightcone_load_h5", py_lightcone_load_h5, METH_VARARGS,
   "_lightcone_load_h5(_lightcone, filename); load lightcone from HDF file}"},
  
  {"_catalogues_alloc", py_catalogues_alloc, METH_VARARGS,
   "allocate a new _Catalogues"},
  {"_catalogues_load_h5", py_catalogues_load_h5, METH_VARARGS,
   "append a catalogue from a HDF5 file"},
  {"_catalogues_generate", py_catalogues_generate, METH_VARARGS,
   "_catalogues_generate(_catalogues, _hod, _lightcone, z_min, z_max, random)"},
  {"_catalogues_len", py_catalogues_len, METH_VARARGS,
   "number of catalogues in _Catalogues"},
  {"_catalogues_catalogue", py_catalogues_catalogue, METH_VARARGS,
   "_catalogues_catalogue(i); return ith _Catalogue"},
  {"_catalogues_append", py_catalogues_append, METH_VARARGS,
   "append a catalogue to catalogues"},   
  {"_catalogues_ngal", py_catalogues_ngal, METH_VARARGS,
   "return average number of galaxies in catalogues"},
  {"_catalogues_nz", py_catalogues_nz, METH_VARARGS,
   "return number of galaxies dN/dz in catalogues"},
  {"_catalogues_subsample", py_catalogues_subsample, METH_VARARGS,
   "_catalogues_subsample(_catalogues, n)"},
  
  {"_sky_alloc", py_sky_alloc, METH_VARARGS,
   "allocate a new _Sky"},
  {"_sky_boxsize", py_sky_boxsize, METH_VARARGS,
   "get bounding box"},
  {"_sky_left", py_sky_left, METH_VARARGS,
   "get left corner (x0, y0, z0) of the bounding box"},
  {"_sky_right", py_sky_left, METH_VARARGS,
   "get right corner (x1, y1, z1) of the bounding box"},
  {"_sky_r_range", py_sky_r_range, METH_VARARGS,
   "get r range"},
  {"_sky_compute_radec", py_sky_compute_radec, METH_VARARGS,
   "(ra, dec) = _sky_compute_radec(_sky, x,y,z)"},
  {"_sky_compute_x", py_sky_compute_x, METH_VARARGS,
   "(x, y, z) = _sky_compute_radec(_sky, r, ra, dec)"},

  {"_distance_init", py_distance_init, METH_VARARGS,
   "initialise distance - redshift module"},
  {"_distance_redshift", py_distance_redshift, METH_VARARGS,
   "_distance_redshift(d); redshift corresponds to comoving distance d"},

  {"_remap_alloc", py_remap_alloc, METH_VARARGS,
   "allocate _Remapping object"},
  {"_remap_boxsize", py_remap_boxsize, METH_VARARGS,
   "_remap_boxsize(_remap); get cuboid size after remapping"},

  {"_slice_alloc", py_slice_alloc, METH_VARARGS, "allocate a slice object"},
  {"_slice_len", py_slice_len, METH_VARARGS, "number of slices in _slice"},
  {"_slice_boxsize", py_slice_boxsize, METH_VARARGS, "boxsize of the slice"},

  {"_halo_mass_alloc", py_halo_mass_alloc, METH_VARARGS,
   "_halo_mass_alloc(filename); allocate _HaloMass object"},
  
  {"_snapshots_alloc", py_snapshots_alloc, METH_VARARGS,
   "allocate _Snapshots object"},
  {"_snapshots_insert", py_snapshots_insert, METH_VARARGS,
   "_snapshots_insert(_snps, a_snp, a_min, a_max, halo_mass)"},
  {"_snapshots_len", py_snapshots_len, METH_VARARGS,
   "_snapshots_len(_snps); get the number of snapshot in _snps"},
  {"_snapshots_get", py_snapshots_get, METH_VARARGS,
   "_snapshot_get(_snps, i); get ith snapshot as a tuble"},
  {"_snapshots_clear", py_snapshots_clear, METH_VARARGS,
   "clear snapshot deque"},

  {"_cola_lightcones_create", py_cola_lightcones_create, METH_VARARGS,
   "_cola_lightcones_create(_snapshots, _sky, _remap, _slice, _lightcones)"},

  {"_halo_concentration_init", py_halo_concentration_init, METH_VARARGS,
   "_halo_concentration_init(_sigma)"},

  {"_rand_init", py_rand_init, METH_VARARGS,
   "initialise random number generator"},
  {"_rand_free", py_rand_free, METH_VARARGS,
   "free random number generator"},
  
  {"_corr_projected_alloc", py_corr_projected_alloc, METH_VARARGS,
   "allocate corr projected"},
  {"_corr_set_radec_min", py_corr_set_radec_min, METH_VARARGS,
   "set minimum pair RA and Dec deparation"},
  {"_corr_set_pair_correction", py_corr_set_pair_correction, METH_VARARGS,
   "_corr_set_pair_correction(filename)"},
  {"_corr_projected_compute", py_corr_projected_compute, METH_VARARGS,
   "compute correlation function from catalogues"},
  {"_corr_as_array", py_corr_as_array, METH_VARARGS,
   "return correlation function as np.array"},
  {"_corr_rp", py_corr_rp, METH_VARARGS, "return rp array"},
  {"_corr_wp", py_corr_wp, METH_VARARGS, "return wp array"},
  {"_corr_dwp", py_corr_dwp, METH_VARARGS, "return dwp array"},

  {"_corr_projected_hist2d_alloc", py_corr_projected_hist2d_alloc, METH_VARARGS,
   "_corr_projected_hist2d_alloc(rp_min, rp_max, rp_nbin, pi_max, pi_nbin)"},
  {"_corr_projected_hist2d_as_array", py_corr_projected_hist2d_as_array,
   METH_VARARGS, "_corr_projected_hist2d_as_array(_hist2d)"},
  {"_corr_projected_hist2d_set", py_corr_projected_hist2d_set, METH_VARARGS,
   "_corr_projected_hist2d_set(_hist2d, hist, npairs)"},
  {"_corr_rp_i", py_corr_rp_i, METH_VARARGS, "return rp array of vcorr[i]"},
  {"_corr_wp_i", py_corr_wp_i, METH_VARARGS, "return wp array of vcorr[i]"},
  {"_corr_projected_compute_rr", py_corr_projected_compute_rr, METH_VARARGS,
   "_corr_projected_compute_rr"},
  {"_corr_projected_compute_with_rr", py_corr_projected_compute_with_rr,
   METH_VARARGS, "_corr_projected_compute_with_rr"},

  {"_corr_projected_compute_direct", py_corr_projected_compute_direct,
   METH_VARARGS, "_corr_projected_compute_direct"},
  {"_corr_projected_compute_rr_direct", py_corr_projected_compute_rr_direct,
   METH_VARARGS, "_corr_projected_compute_rr_direct"},
  {"_corr_projected_compute_all", py_corr_projected_compute_all,
   METH_VARARGS, "_corr_projected_compute_all"},

  
  {"_corr_multipole_hist2d_alloc", py_corr_multipole_hist2d_alloc,
    METH_VARARGS, "_corr_multipole_hist2d_alloc"},

  {"_corr_multipole_hist2d_alloc", py_corr_multipole_hist2d_alloc,
    METH_VARARGS, "_corr_multipole_hist2d_alloc"},
  {"_corr_multipole_hist2d_as_array", py_corr_multipole_hist2d_as_array,
    METH_VARARGS, "_corr_multipole_hist2d_as_array"},
  {"_corr_multipole_hist2d_set", py_corr_multipole_hist2d_set,
    METH_VARARGS, "_corr_multipole_hist2d_set"},
  {"_corr_multipole_alloc", py_corr_multipole_alloc,
    METH_VARARGS, "_corr_multipole_alloc"},
  {"_corr_multipole_set_radec_min", py_corr_multipole_set_radec_min,
    METH_VARARGS, "_corr_multipole_set_radec_min"},
  {"_corr_multipole_set_pair_correction", py_corr_multipole_set_pair_correction,
    METH_VARARGS, "_corr_multipole_set_pair_correction"},
  {"_corr_multipole_compute", py_corr_multipole_compute,
   METH_VARARGS, "_corr_multipole_compute"},
  {"_corr_multipole_as_array", py_corr_multipole_as_array,
    METH_VARARGS, "_corr_multipole_as_array"},
  {"_corr_multipole_r_i", py_corr_multipole_r_i,
   METH_VARARGS, "_corr_multipole_r_i"},
  {"_corr_multipole_xi0_i", py_corr_multipole_xi0_i,
   METH_VARARGS, "_corr_multipole_xi0_i"},
  {"_corr_multipole_xi2_i", py_corr_multipole_xi2_i,
   METH_VARARGS, "_corr_multipole_xi2_i"},
  {"_corr_multipole_compute_rr", py_corr_multipole_compute_rr,
   METH_VARARGS, "_corr_multipole_compute_rr"},
  {"_corr_multipole_compute_with_rr", py_corr_multipole_compute_with_rr,
   METH_VARARGS, "_corr_multipole_compute_with_rr"},

  {"_minimise", py_minimise, METH_VARARGS, "minimise funtion"},

  {"_comm_init", py_comm_init, METH_VARARGS, "Initialize MPI"},
  {"_comm_finalise", py_comm_finalise, METH_VARARGS, "Finalise MPI"},
  {"_comm_this_rank", py_comm_this_rank, METH_VARARGS, "MPI rank"},
  {"_comm_n_nodes", py_comm_n_nodes, METH_VARARGS, "Number of MPI nodes"},
  {"_comm_bcast_str", py_comm_bcast_str, METH_VARARGS, "Broadcast string"},
  {"_comm_bcast_int", py_comm_bcast_int, METH_VARARGS, "Broadcast int"},

  {"_array_loadtxt", py_array_loadtxt, METH_VARARGS, "Broadcast string"},

  {"_callback_standby", py_callback_standby, METH_VARARGS,
   "_callback_standby(function)"},
  {"_callback_sync", py_callback_sync, METH_VARARGS,
   "_callback_sync(x)"},
  {"_callback_release", py_callback_release, METH_VARARGS,
   "_callback_release"},
  
  {NULL, NULL, 0, NULL}
};


static struct PyModuleDef module = {
  PyModuleDef_HEAD_INIT,
  "_mockgallib", // name of this module
  "A package for mock galaxy generation", // Doc String
  -1,
  methods
};

PyMODINIT_FUNC
PyInit__mockgallib(void) {
  py_power_module_init();  
  py_sigma_module_init();
  py_catalogues_module_init();
  py_lightcones_module_init();
  py_corr_projected_module_init();
  py_corr_multipole_module_init();
  py_array_module_init();
  
  return PyModule_Create(&module);
}
