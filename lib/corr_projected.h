#ifndef CORR_PROJECTED_H
#define CORR_PROJECTED_H 1

#include <vector>
#include "particle.h"
#include "catalogue.h"
//#include "kdtree.h"

struct CorrProjected {
  CorrProjected(const int nbin);
  ~CorrProjected();
  void print(FILE* fp);
  int n;
  double* rp;
  double* wp;
  double* dwp;
};

void corr_projected_init(const float rp_min_, const float rp_max_, const int nbin_, const float pi_max_, const int nbin_pi_);

void set_radec_min(const float ra_min_, const float dec_min_);

void corr_projected_compute(Catalogues* const cats_data,
			    Catalogues* const cats_rand,
			    CorrProjected* const corr);

/*
struct Catalogue: std::vector<Particle>
{
  KDTree* tree;
  size_t ntree;
  int ncen, nsat;
};


void corr_projected_init(const float rp_min_, const float rp_max_, const int nbin_, const float pi_max_, const int pi_nbin_);
void corr_projected_free();


void corr_alloc(const int n_data_cat, const int nbin, std::vector<CorrProjected*>& vcorr);

void corr_projected_write(const int index, const std::vector<CorrProjected*>& vcorr);

void corr_projected_summarise(const std::vector<CorrProjected*>& vcorr,
			      CorrProjected* corr_out);

*/
  

#endif
