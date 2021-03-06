#ifndef CATALOGUE
#define CATALOGUE 1

#include <vector>
#include <deque>
#include "particle.h"
#include "kdtree.h"
#include "hod.h"
#include "lightcone.h"
#include "sky.h"

class Catalogue: public std::vector<Particle>
{
 public:
  Catalogue();
  
  KDTree* tree;
  size_t ntree;
  double wsum, w2sum;
  int ncen, nsat;
};


class Catalogues: public std::deque<Catalogue*> {
 public:
  Catalogues();
  Catalogues(const size_t ncat);
  ~Catalogues();
  void allocate(const size_t ncat);
  //double ngal(double* ncen= 0, double* nsat= 0) const;
};

void catalogue_init();
void catalogue_free();

void catalogue_generate_mock(Hod* const hod,
			     LightCone const * const lightcone,
			     Sky const * const sky,
			     const double z_min, const double z_max,
			     Catalogue * const cat);

void catalogue_generate_random(Hod* const hod,
			       LightCone const * const lightcone,
			       Sky const * const sky,
			       const double z_min, const double z_max,
			       Catalogue * const cat);

double* catalogue_compute_nz(Catalogue const * const cat,
		     const double z_min, const double z_max, const int nbin);

void catalogue_subsample(Catalogue* const cat, const size_t n);

#endif
