"""
This script computes correlation function multipoles from given files of
galalxy and random catalgues

python3 compute_corr_multipole.py

Args:
    n: index of mock

Options:
    --igalaxies=1:1
    --irandoms=1:1

Input:
    mocks/mock_<n>.txt
    randoms/random_<n>.txt
    
    File format is:
    x y z
    in each line

Output:
    Standard output
    Column 1: r
    Column 2: xi0 (monopole)
    Column 3: xi2 (quadrupole)
"""

import os
import argparse
import json
import signal
import numpy as np
import h5py
import mockgallib as mock

signal.signal(signal.SIGINT, signal.SIG_DFL) # stop with ctrl-c


#
# Command-line options
#
parser = argparse.ArgumentParser()
parser.add_argument('reg', help='region w1 or w4')
parser.add_argument('--igalaxies', default='1:1',
                    help='index range of galaxy catalogues')
parser.add_argument('--irandoms', default='1:1',
                    help='index range of random catalogues')
parser.add_argument('--pair-correction', default=None,
                    help='pair correction file')
parser.add_argument('--param', default='param.json',
                    help='parameter json file')
parser.add_argument('--dir', default='.', help='data directory')
parser.add_argument('--rr', default=None, help='precomputed RR filename')
parser.add_argument('-o', default='.', help='output directory')

parser.add_argument('--zmin', type=float, default=0.5, help='minimum redshift')
parser.add_argument('--zmax', type=float,  default=1.2, help='minimum redshift')

arg = parser.parse_args()

igalaxies = arg.igalaxies.split(':')
irandoms = arg.irandoms.split(':')

#
# Read parameter file
#
print('# Parameter file: %s' % arg.param)

with open(arg.param, 'r') as f:
    param = json.load(f)

omega_m = param['omega_m']
print('# Setting cosmology: omega_m= %.4f' % omega_m)
print('# redshift-range %f %f' % (arg.zmin, arg.zmax))

#
# Initilise
#
mock.set_loglevel(0)
mock.cosmology.set(omega_m)
mock.distance.init(1.2)

def read_catalogues(filebase, irange):
     cats = mock.Catalogues()
     for i in range(int(irange[0]), int(irange[1]) + 1):
         filename = '%s%05d.txt' % (filebase, i)
         a = np.loadtxt(filename, delimiter=' ', usecols=[1,2,3,7,8,12])
         w = np.loadtxt(filename, delimiter=' ', usecols=[12, 13])
         a[:, 5] = 1.0/(w[:, 0]*w[:, 1])
         cats.append(a, z_min=arg.zmin, z_max= arg.zmax)
     return cats

galaxies = read_catalogues('%s/mocks/%s/mock_%s_' % (arg.dir, arg.reg, arg.reg), igalaxies)
randoms  = read_catalogues('%s/rands/%s/rand_%s_' % (arg.dir, arg.reg, arg.reg), irandoms)


corr = mock.CorrelationFunctionMultipole(r_min=0.08912509, r_max=141.25375446,
                                         nbin=32,
                                         mu_nbin=40,
                                         ra_min=0.0, dec_min=0.0,
                                         pair_correction=arg.pair_correction)

if arg.rr:
    rr = mock.corr_multipole.Hist2D(r_min=0.08912509, r_max=141.25375446,
                                    r_nbin=32, mu_nbin=40)
    rr.load(arg.rr)

    print('computing corr multipole with rr: %s' % arg.rr)

    corr.compute_corr_multipole_with_rr(galaxies, randoms, rr)
else:
    corr.compute_corr_multipole(galaxies, randoms)

i0 = int(igalaxies[0])
i1 = int(igalaxies[1]) + 1

for i in range(i0, i1):
    ii= i - i0
    r = corr.r_i(ii)
    xi0 = corr.xi0_i(ii)
    xi2 = corr.xi2_i(ii)

    nrow= len(r)
    filename = '%s/corr_multipole_%05d.txt' % (arg.o, i)
    with open(filename, 'w') as f:
        for irow in range(nrow):
            f.write('%e %e %e\n' % (r[irow], xi0[irow], xi2[irow]))

    print('%s written' % filename)
