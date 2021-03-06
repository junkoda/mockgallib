import mockgallib._mockgallib as c
from mockgallib.hod import Hod

class Nbar:
    """Nbar(ps, hod): compute nbar(z) from power spectrum ps and HOD hod"""
    def __init__(self, hod):
        self._ni= c._nbar_alloc(hod._hod)

    def __repr__(self):
        return "nbar integration object"

    def __call__(self, z):        
        """compute nbar(z)"""
        return c._nbar_compute(self._ni, z)

    def n(self, z):
        return c._nbar_compute(self._ni, z)

    def ncen(self, z):
        return c._nbar_ncen_compute(self._ni, z)

    def nsat(self, z):
        return c._nbar_nsat_compute(self._ni, z)

class NbarFitting:
    """Class for fitting HOD logMmin(z) coefficients to fit nbar(z)"""
    def __init__(self, hod, array_obs, z_min, z_max):
        """NbarFitting(hod, nbar_obs, z_min, z_max)"""
        self.z_min= z_min
        self.z_max= z_max
        self._fitting= c._nbar_fitting_alloc(hod._hod, array_obs,
                                             z_min, z_max)
        self.n = c._nbar_fitting_len(self._fitting)
        self.z = [ c._nbar_fitting_z(self._fitting, i) for i in range(self.n) ]
        self.nbar_obs = [ c._nbar_fitting_nbar_obs(self._fitting, i)
                          for i in range(self.n) ]
        self.nbar_hod= [0]*self.n

    def fit(self):
        """execute fitting"""
        self.iter, self.chi2 = c._nbar_fitting_compute(self._fitting)
        self.nbar_hod = [ c._nbar_fitting_nbar_hod(self._fitting, i)
                          for i in range(self.n) ]

    def __repr__(self):
        return "NbarFitting: %d data in range %.3f <= z <= %.3f" % (self.n, self.z_min, self.z_max)

    def __len__(self):
        """number of data"""
        return self.n

    def __getitem__(self, i):
        """(z, nbar_obs, nbar_hod)"""
        return (self.z[i], self.nbar_obs[i], self.nbar_hod[i])




    
        
    
