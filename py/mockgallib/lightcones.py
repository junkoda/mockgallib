import mockgallib._mockgallib as c

class LightCone:
    """A Light Cone of haloes"""
    def __init__(self, _lc):
        self._lightcone = _lc

    def __len__(self, _lc):
        return c._lightcone_len(self._lightcone)

    def as_array(self):
        return c._lightcone_as_array(self._lightcone)

    def save_h5(self, filename):
        c._lightcone_save_h5(self._lightcone, filename)

    def load_h5(self, filename):
        c._lightcone_load_h5(self._lightcone, filename)


class LightCones:
    """A collection of halo light cones"""
    def __init__(self):
        self._lt = c._lightcones_alloc()


    def __len__(self):
        return c._lightcones_len(self._lt)


    def __repr__(self):
        return "A collection of %d lightcones" % len(self)


    def __getitem__(self, i):
        return LightCone(c._lightcones_lightcone(self._lt, i))


    def clear(self):
        c._lightcones_clear(self._lt)


    def load_h5(self, filenames):
        """load([filename1, filename2]); load lightcones from files"""
        for filename in filenames:
            c._lightcones_load_h5(self._lt, filename)


    def create_from_snapshots(self, snapshots, sky, remap, slice,random=False):
        """create_from_snapshots(snapshots, sky, remap); create lightcones from halo snapshots"""
        c._cola_lightcones_create(snapshots._snps, sky._sky, remap._remap,
                                  slice._slice, self._lt, int(random));
