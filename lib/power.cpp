#ifdef WITHMPI
#include <mpi.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>


#include "power.h"
#include "msg.h"
#include "error.h"
#include "comm.h"

static int n=0;
static double* k= 0;
static double* P;

static double* read_powerspectrum_file(const char filename[], int* const nlines_out);

void power_init(const char filename[])
{
  if(k) return;
  
  // Read k P from filename
  double* buf= 0;
  int nlines= 0;
  if(comm_this_rank() == 0) {
    try {
      buf= read_powerspectrum_file(filename, &nlines);
    }
    catch(FileNotFoundError e) {
      nlines= 0;
    }
  }

  n= comm_bcast_int(nlines);
  if(n == 0)
    throw FileNotFoundError();
  
  k= (double*) malloc(2*n*sizeof(double)); assert(k);
  P= k + n;

  if(comm_this_rank() == 0) {
    for(int j=0; j<n; j++) {
      k[j] = buf[2*j];
      P[j] = buf[2*j + 1];
    }
    free(buf);
  }

#ifdef WITHMPI
  msg_printf(msg_debug, "MPI broadcast power spectrum data %d\n", n);
  MPI_Bcast(k, 2*n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
#endif
}

void power_free()
{
  free(k);
}

double* read_powerspectrum_file(const char filename[], int* const nlines_out)
{
  FILE* const fp= fopen(filename, "r");
  if(fp == 0) {
    msg_printf(msg_error,
	       "Error: Unable to open input power spectrum file: %s\n",
	       filename);
    throw FileNotFoundError();
  }

  int nalloc= 1000;
  double* buf= (double*) malloc(sizeof(double)*2*nalloc);

  char line[128];
  int nlines= 0;
  double kk, PP;

  // Read lines and push to buf as k1,P1,k2,P2, ...
  // Doubles the length of buf when the length of the array is not enough
  while(fgets(line, 127, fp)) {
    if(nlines == nalloc) {
      msg_printf(msg_debug, "Reallocating power spectrum table %d -> %d\n",
		 nalloc, 2*nalloc);
      nalloc *= 2;
      buf= (double*) realloc(buf, sizeof(double)*2*nalloc); assert(buf);
    }
    
    if(line[0] == '#')
      continue;
    else if(sscanf(line, "%lg %lg", &kk, &PP) == 2) {
      buf[2*nlines    ]= kk;
      buf[2*nlines + 1]= PP;
      
      nlines++;
    }
    else {
      msg_printf(msg_warn,
	    "Warning: Unable to understand a line in the power spectrum file;"
	    "following data are ignored: %s", line);
      break;
    }
  }

  int ret= fclose(fp); assert(ret == 0);
  
  msg_printf(msg_verbose, "Read %d pairs of k P(k) from %s\n", nlines, filename);
  
  *nlines_out= nlines;
  return buf;
}

double power_compute_sigma(const double R)
{
  // Computes sigma (rms amplituede) smoothed on scale R
  // R: smoothing length [/h Mpc] (8 for sigma_8)
  // 1/(2*pi^2) \int P(k) W(k*R) k^2 dk
  assert(k);
  
  double k0= k[0];
  double f0= 0.0;
  
  double sigma2= 0.0;
  for(int i=0; i<n; i++) {
    double kk= k[i];
    double x= kk*R;
    double w= 3.0*(sin(x)-x*cos(x))/(x*x*x);
    double f1= P[i]*kk*kk*w*w;
    
    sigma2 += 0.5*(f0 + f1)*(kk - k0);

    k0= kk;
    f0= f1;
  }

  return sqrt(1.0/(2.0*M_PI*M_PI)*sigma2);
}

int power_n()
{
  return n;
}

double * power_P()
{
  assert(k);
  return P;
}

double * power_k()
{
  assert(k);
  return k;
}
