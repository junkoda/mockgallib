//
// Generate a lightcone from snapshots
//

#include <cstdio>
#include <iostream>
#include <vector>

#include "msg.h"
#include "util.h"
#include "cola_file.h"
#include "distance.h"
#include "halo_mass.h"
#include "halo_concentration.h"
#include "mf_cumulative.h"
#include "cola_lightcone.h"
#include "rand.h"

//
// Dependence:
//     sigma_init() - power_init()
//     distance(z_max)
//
using namespace std;

static void fill_lightcone_haloes(Snapshot const * const snp,
				  Sky const * const sky,
				  Remap const * const remap,
				  Slice const * const slice,
				  const float M_min,
				  const bool random,
				  LightCones* const lightcones);

static void fill_lightcone_particles(Snapshot const * const snp,
			      Sky const * const sky,
			      Remap const * const remap,
			      Slice const * const slice,
			      const float M_min,
			      const float M_max,
			      const bool random,
			      LightCones* const lightcones);

static inline void
  randomise_position(const float boxsize, float* const x)
{
  x[0]= boxsize*rand_uniform();
  x[1]= boxsize*rand_uniform();
  x[1]= boxsize*rand_uniform();
}

void cola_lightcones_create(Snapshots const * const snapshots,
			    Sky const * const sky,
			    Remap const * const remap,
			    Slice const * const slice,
			    LightCones* const lightcones,
			    const bool random)
{
  // Particle is used for M_min < M < M_max
  // Halo is used M > M_max
  
  // Prerequisite: sigma_init()
  rand_init();
  halo_concentration_init();
  distance_init(sky->z_range[1]);
      
  if(random) {
    rand_init();
    msg_printf(msg_verbose, "Generate random lightcone\n");
  }
  else
    msg_printf(msg_verbose, "Generate halo lightcone\n");    
  
  lightcones->clear();
  
  for(Snapshots::const_iterator snp= snapshots->begin();
      snp != snapshots->end(); ++snp) {
    
    fill_lightcone_haloes(*snp, sky, remap, slice, 
			  (*snp)->M_halo_min, random, lightcones);
    fill_lightcone_particles(*snp, sky, remap, slice, 
			     (*snp)->M_part_min, (*snp)->M_halo_min, 
			     random, lightcones);
  }
}	   

void fill_lightcone_haloes(Snapshot const * const snp,
			   Sky const * const sky,
			   Remap const * const remap,
			   Slice const * const slice,
			   const float M_min,
			   const bool random,
			   LightCones* const lightcones)
{
  // Prerequisite:
  //   
  // halo_concentration_init() -- sigma_init() -- power_init()
  //

  if(lightcones->size() < slice->n) {
    lightcones->resize(slice->n);
  }

  msg_printf(msg_verbose, "filling lightcone from %s, a=%.3f\n",
	     snp->filename_fof, snp->a_snp);

  float boxsize;
  
  cola_halo_file_open(snp->filename_fof, &boxsize);
  // May throw ColaFileError()

  Halo halo;
  Halo* const h= &halo;

  const float r_min= snp->r_min;
  const float r_max= snp->r_max;
  const float ra_min= sky->ra_range[0];
  const float ra_max= sky->ra_range[1];
  const float dec_min= sky->dec_range[0];
  const float dec_max= sky->dec_range[1];

  while(cola_halo_file_read_one(h)) {
    // convert nfof to halo mass
    h->M= snp->halo_mass->mass(h->nfof);
    if(h->M < M_min)
      continue;

    //debug!!!
    //printf("%d %e\n", h->nfof, h->M);

    // randomise the coordinate if random = true
    if(random)
      randomise_position(boxsize, h->x);


    // remap cube to cuboid
    if(!remap->coordinate(h))
      continue;

    // cuboid to sliced cuboid (multiple mock from one cuboid)
    slice->transform(h);

    h->a= snp->a_snp;
    h->r= util::norm(h->x);

    if(h->r < r_min || h->r >= r_max)
	continue;

    // compute sky ra-dec
    sky->compute_radec(h->x, h->radec);

    if(h->radec[0] < ra_min  || h->radec[0] > ra_max ||
       h->radec[1] < dec_min || h->radec[1] > dec_max)
      continue;

    // compute raidial velocity
    h->vr= util::dot(h->x, h->v)/util::norm(h->x);

    // compute redshift at halo position
    h->z= distance_redshift(h->r);

    // set halo concentration / rs
    h->rs= halo_concentration_rs(h);

    (*lightcones)[h->slice]->push_back(*h);
  }
   
  cola_halo_file_close();
}

/*
static inline double rad(const double deg)
{
  return M_PI/180.0*deg;
}

static double volume(const double r_min, const double r_max,
		     const double ra_min, const double ra_max,
		     const double dec_min, const double dec_max)
{
  const double dra= rad(ra_max - ra_min);
  const double dcosO= sin(rad(ra_max)) - sin(rad(ra_min));
  return dra*dcosO*(r_max*r_max*r_max - r_min*r_min*r_min)/3.0;
}
*/

void fill_lightcone_particles(Snapshot const * const snp,
			      Sky const * const sky,
			      Remap const * const remap,
			      Slice const * const slice,
			      const float M_min, const float M_max,
			      const bool random,
			      LightCones* const lightcones)
{
  // Prerequisite:
  //   
  // halo_concentration_init() -- sigma_init() -- power_init()
  //
  if(M_min >= M_max) {
    cerr << "return: a= " << snp->a_snp << endl;
    return;
  }

  if(lightcones->size() < slice->n) {
    lightcones->resize(slice->n);
  }

  msg_printf(msg_verbose, "filling lightcone from %s, a=%.3f log10M (%e %e)\n",
	     snp->filename_part, snp->a_snp,
	     log10(snp->M_part_min), log10(snp->M_halo_min));

  float boxsize;
  int np;
  
  cola_part_file_open(snp->filename_part, &boxsize, &np);
  // May throw ColaFileError()

  Halo halo;
  Halo* const h= &halo;
  MfCumulative const * const mfc= snp->mfc;

  const float r_min= snp->r_min;
  const float r_max= snp->r_max;
  const float ra_min= sky->ra_range[0];
  const float ra_max= sky->ra_range[1];
  const float dec_min= sky->dec_range[0];
  const float dec_max= sky->dec_range[1];

  //cerr << "boxsize= " << boxsize << endl;
  const double nM_min= mfc->n_cumulative(M_max);
  const double nM_max= mfc->n_cumulative(M_min);

  const double frac= (nM_max - nM_min)/(np/(boxsize*boxsize*boxsize));
    // fraction of particle necessary / particle in the file
  msg_printf(msg_verbose, "particle using fraction %e\n", frac);
  
  while(cola_part_file_read_one(h)) {
    // convert nfof to halo mass
    if(rand_uniform() > frac) continue;

    double nM= nM_min + (nM_max - nM_min)*rand_uniform();
    h->M= mfc->M(nM);

    // randomise the coordinate if random = true
    if(random)
      randomise_position(boxsize, h->x);

    // remap cube to cuboid
    if(!remap->coordinate(h))
      continue;

    // cuboid to sliced cuboid (multiple mock from one cuboid)
    slice->transform(h);

    h->a= snp->a_snp;
    h->r= util::norm(h->x);

    if(h->r < r_min || h->r >= r_max)
	continue;

    // compute sky ra-dec
    sky->compute_radec(h->x, h->radec);

    if(h->radec[0] < ra_min  || h->radec[0] > ra_max ||
       h->radec[1] < dec_min || h->radec[1] > dec_max)
      continue;

    // compute raidial velocity
    h->vr= util::dot(h->x, h->v)/util::norm(h->x);

    // compute redshift at halo position
    h->z= distance_redshift(h->r);

    // set halo concentration / rs
    h->rs= halo_concentration_rs(h);

    (*lightcones)[h->slice]->push_back(*h);
  }
   
  cola_part_file_close();
}

