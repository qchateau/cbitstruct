import time
import cbitstruct
import bitstruct
import timeit
import unittest

import string
import random
import cbitstruct
import bitstruct

fmt = "s12u45p127f32f64s32r13p2P8r32u16u2s12"
nbytes = (bitstruct.calcsize(fmt) + 7) // 8

random.seed(0)
data = bytes([random.randint(0, 255) for _ in range(nbytes)])
dst = bytearray([0] * nbytes)
values = bitstruct.unpack(fmt, data)
names = string.ascii_letters[: len(values)]
values_dict = {n: v for n, v in zip(names, values)}

bs = bitstruct.compile(fmt)
cbs = cbitstruct.compile(fmt)

dbs = bitstruct.compile(fmt, names)
cdbs = cbitstruct.compile(fmt, names)

NBS = 10000
NCBS = 100000


class PerfTest(unittest.TestCase):
    def generic(self, name, bitstruct, cbitstruct):
        bstime = timeit.timeit(bitstruct, number=NBS, globals=globals()) / NBS
        cbstime = timeit.timeit(cbitstruct, number=NCBS, globals=globals()) / NCBS
        improvement = bstime / cbstime

        print(
            "\r{:<25} | x{:>7.3f} ".format(name, improvement)
            + "({:>8.3f}us -> {:>7.3f}us)".format(1e6 * bstime, 1e6 * cbstime)
        )

    def test_unpack(self):
        self.generic(
            "unpack", "bitstruct.unpack(fmt, data)", "cbitstruct.unpack(fmt, data)"
        )

    def test_unpack_from(self):
        self.generic(
            "unpack_from",
            "bitstruct.unpack_from(fmt, data, 3)",
            "cbitstruct.unpack_from(fmt, data, 3)",
        )

    def test_unpack_dict(self):
        self.generic(
            "unpack_dict",
            "bitstruct.unpack_dict(fmt, names, data)",
            "cbitstruct.unpack_dict(fmt, names, data)",
        )

    def test_unpack_from_dict(self):
        self.generic(
            "unpack_from_dict",
            "bitstruct.unpack_from_dict(fmt, names, data, 3)",
            "cbitstruct.unpack_from_dict(fmt, names, data, 3)",
        )

    def test_compiled_unpack(self):
        self.generic("compiled unpack", "bs.unpack(data)", "cbs.unpack(data)")

    def test_compiled_unpack_from(self):
        self.generic(
            "compiled unpack_from",
            "bs.unpack_from(data, 3)",
            "cbs.unpack_from(data, 3)",
        )

    def test_compiled_unpack_dict(self):
        self.generic("compiled unpack_dict", "dbs.unpack(data)", "cdbs.unpack(data)")

    def test_compiled_unpack_from_dict(self):
        self.generic(
            "compiled unpack_from_dict",
            "dbs.unpack_from(data, 3)",
            "cdbs.unpack_from(data, 3)",
        )

    def test_pack(self):
        self.generic(
            "pack", "bitstruct.pack(fmt, *values)", "cbitstruct.pack(fmt, *values)"
        )

    def test_pack_into(self):
        self.generic(
            "pack_into",
            "bitstruct.pack_into(fmt, dst, 3, *values)",
            "cbitstruct.pack_into(fmt, dst, 3, *values)",
        )

    def test_pack_dict(self):
        self.generic(
            "pack_dict",
            "bitstruct.pack_dict(fmt, names, values_dict)",
            "cbitstruct.pack_dict(fmt, names, values_dict)",
        )

    def test_pack_into_dict(self):
        self.generic(
            "pack_into_dict",
            "bitstruct.pack_into_dict(fmt, names, dst, 3, values_dict)",
            "cbitstruct.pack_into_dict(fmt, names, dst, 3, values_dict)",
        )

    def test_compiled_pack(self):
        self.generic("compiled pack", "bs.pack(*values)", "cbs.pack(*values)")

    def test_compiled_pack_into(self):
        self.generic(
            "compiled pack_into",
            "bs.pack_into(dst, 3, *values)",
            "cbs.pack_into(dst, 3, *values)",
        )

    def test_compiled_pack_dict(self):
        self.generic(
            "compiled pack_dict", "dbs.pack(values_dict)", "cdbs.pack(values_dict)"
        )

    def test_compiled_pack_into_dict(self):
        self.generic(
            "compiled pack_into_dict",
            "dbs.pack_into(dst, 3, values_dict)",
            "cdbs.pack_into(dst, 3, values_dict)",
        )

    def test_calcsize(self):
        self.generic("calcsize", "bitstruct.calcsize(fmt)", "cbitstruct.calcsize(fmt)")

    def test_byteswap_str(self):
        self.generic(
            "byteswap str",
            "bitstruct.byteswap('123456789', data, 0)",
            "cbitstruct.byteswap('123456789', data, 0)",
        )

    def test_byteswap_integer(self):
        self.generic(
            "byteswap list of int",
            "bitstruct.byteswap([1,2,3,4,5,6,7,8,9], data, 0)",
            "cbitstruct.byteswap([1,2,3,4,5,6,7,8,9], data, 0)",
        )


if __name__ == "__main__":
    unittest.main()
