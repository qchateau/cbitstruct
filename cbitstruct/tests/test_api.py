import unittest
import inspect
import bitstruct
import cbitstruct


NAMES = ["foo"]
FMT = "u32"
DICT = {"foo": 42}
DATA = b"\x00\x00\x00\x00"
BUF = bytearray(4)
ARGS = [12]
CF = cbitstruct.CompiledFormat(FMT)
CFD = cbitstruct.CompiledFormatDict(FMT, NAMES)


class BitstructApiTest(unittest.TestCase):
    def test_no_args(self):
        self.assertRaises(Exception, cbitstruct.byteswap)
        self.assertRaises(Exception, cbitstruct.calcsize)
        self.assertRaises(Exception, cbitstruct.pack)
        self.assertRaises(Exception, cbitstruct.pack_into)
        self.assertRaises(Exception, cbitstruct.pack_dict)
        self.assertRaises(Exception, cbitstruct.pack_into_dict)
        self.assertRaises(Exception, cbitstruct.unpack)
        self.assertRaises(Exception, cbitstruct.unpack_from)
        self.assertRaises(Exception, cbitstruct.unpack_dict)
        self.assertRaises(Exception, cbitstruct.unpack_from_dict)
        self.assertRaises(Exception, cbitstruct.CompiledFormat)
        self.assertRaises(Exception, cbitstruct.CompiledFormatDict)
        cf = cbitstruct.CompiledFormat(FMT)
        self.assertRaises(Exception, cf.pack)
        self.assertRaises(Exception, cf.pack_into)
        self.assertRaises(Exception, cf.unpack)
        self.assertRaises(Exception, cf.unpack_from)
        cfd = cbitstruct.CompiledFormatDict(FMT, NAMES)
        self.assertRaises(Exception, cf.pack)
        self.assertRaises(Exception, cf.pack_into)
        self.assertRaises(Exception, cf.unpack)
        self.assertRaises(Exception, cf.unpack_from)

    def test_compiled_format(self):
        cbitstruct.CompiledFormat(fmt=FMT)
        cbitstruct.CompiledFormat(FMT)

    def test_compiled_format_dict(self):
        cbitstruct.CompiledFormatDict(fmt=FMT, names=NAMES)
        cbitstruct.CompiledFormatDict(FMT, names=NAMES)
        cbitstruct.CompiledFormatDict(FMT, NAMES)

    def test_pack(self):
        cbitstruct.pack(fmt=FMT, *ARGS)
        cbitstruct.pack(FMT, *ARGS)

    def test_compiled_pack(self):
        CF.pack(*ARGS)

    def test_pack_into(self):
        cbitstruct.pack_into(*ARGS, fmt=FMT, buf=BUF, offset=0)
        cbitstruct.pack_into(FMT, *ARGS, buf=BUF, offset=0)
        cbitstruct.pack_into(FMT, BUF, *ARGS, offset=0)
        cbitstruct.pack_into(FMT, BUF, 0, *ARGS)

        cbitstruct.pack_into(*ARGS, fmt=FMT, buf=BUF, offset=0, fill_padding=False)
        cbitstruct.pack_into(FMT, *ARGS, buf=BUF, offset=0, fill_padding=False)
        cbitstruct.pack_into(FMT, BUF, *ARGS, offset=0, fill_padding=False)
        cbitstruct.pack_into(FMT, BUF, 0, *ARGS, fill_padding=False)

    def test_compiled_pack_into(self):
        CF.pack_into(*ARGS, buf=BUF, offset=0)
        CF.pack_into(BUF, *ARGS, offset=0)
        CF.pack_into(BUF, 0, *ARGS)

        CF.pack_into(*ARGS, buf=BUF, offset=0, fill_padding=False)
        CF.pack_into(BUF, *ARGS, offset=0, fill_padding=False)
        CF.pack_into(BUF, 0, *ARGS, fill_padding=False)

    def test_pack_dict(self):
        cbitstruct.pack_dict(fmt=FMT, names=NAMES, data=DICT)
        cbitstruct.pack_dict(FMT, names=NAMES, data=DICT)
        cbitstruct.pack_dict(FMT, NAMES, data=DICT)
        cbitstruct.pack_dict(FMT, NAMES, DICT)

    def test_compiled_pack_dict(self):
        CFD.pack(data=DICT)
        CFD.pack(DICT)

    def test_pack_into_dict(self):
        cbitstruct.pack_into_dict(fmt=FMT, names=NAMES, buf=BUF, offset=0, data=DICT)
        cbitstruct.pack_into_dict(FMT, names=NAMES, buf=BUF, offset=0, data=DICT)
        cbitstruct.pack_into_dict(FMT, NAMES, buf=BUF, offset=0, data=DICT)
        cbitstruct.pack_into_dict(FMT, NAMES, BUF, offset=0, data=DICT)
        cbitstruct.pack_into_dict(FMT, NAMES, BUF, 0, data=DICT)
        cbitstruct.pack_into_dict(FMT, NAMES, BUF, 0, DICT)

        cbitstruct.pack_into_dict(
            fmt=FMT, names=NAMES, buf=BUF, offset=0, data=DICT, fill_padding=False
        )
        cbitstruct.pack_into_dict(
            FMT, names=NAMES, buf=BUF, offset=0, data=DICT, fill_padding=False
        )
        cbitstruct.pack_into_dict(
            FMT, NAMES, buf=BUF, offset=0, data=DICT, fill_padding=False
        )
        cbitstruct.pack_into_dict(
            FMT, NAMES, BUF, offset=0, data=DICT, fill_padding=False
        )
        cbitstruct.pack_into_dict(FMT, NAMES, BUF, 0, data=DICT, fill_padding=False)
        cbitstruct.pack_into_dict(FMT, NAMES, BUF, 0, DICT, fill_padding=False)

    def test_compiled_pack_into_dict(self):
        CFD.pack_into(buf=BUF, offset=0, data=DICT)
        CFD.pack_into(BUF, offset=0, data=DICT)
        CFD.pack_into(BUF, 0, data=DICT)
        CFD.pack_into(BUF, 0, DICT)

        CFD.pack_into(buf=BUF, offset=0, data=DICT, fill_padding=False)
        CFD.pack_into(BUF, offset=0, data=DICT, fill_padding=False)
        CFD.pack_into(BUF, 0, data=DICT, fill_padding=False)
        CFD.pack_into(BUF, 0, DICT, fill_padding=False)

    def test_unpack(self):
        cbitstruct.unpack(fmt=FMT, data=DATA)
        cbitstruct.unpack(FMT, data=DATA)
        cbitstruct.unpack(FMT, DATA)

    def test_compiled_unpack(self):
        CF.unpack(data=DATA)
        CF.unpack(DATA)

    def test_unpack_from(self):
        cbitstruct.unpack_from(fmt=FMT, data=DATA, offset=0)
        cbitstruct.unpack_from(FMT, data=DATA, offset=0)
        cbitstruct.unpack_from(FMT, DATA, offset=0)
        cbitstruct.unpack_from(FMT, DATA, 0)

        cbitstruct.unpack_from(fmt=FMT, data=DATA)
        cbitstruct.unpack_from(FMT, data=DATA)
        cbitstruct.unpack_from(FMT, DATA)

    def test_compiled_unpack_from(self):
        CF.unpack_from(data=DATA, offset=0)
        CF.unpack_from(DATA, offset=0)
        CF.unpack_from(DATA, 0)

        CF.unpack_from(data=DATA)
        CF.unpack_from(DATA)

    def test_unpack_dict(self):
        cbitstruct.unpack_dict(fmt=FMT, names=NAMES, data=DATA)
        cbitstruct.unpack_dict(FMT, names=NAMES, data=DATA)
        cbitstruct.unpack_dict(FMT, NAMES, data=DATA)
        cbitstruct.unpack_dict(FMT, NAMES, DATA)

    def test_compiled_unpack_dict(self):
        CFD.unpack(data=DATA)
        CFD.unpack(DATA)

    def test_unpack_from_dict(self):
        cbitstruct.unpack_from_dict(fmt=FMT, names=NAMES, data=DATA, offset=0)
        cbitstruct.unpack_from_dict(FMT, names=NAMES, data=DATA, offset=0)
        cbitstruct.unpack_from_dict(FMT, NAMES, data=DATA, offset=0)
        cbitstruct.unpack_from_dict(FMT, NAMES, DATA, offset=0)
        cbitstruct.unpack_from_dict(FMT, NAMES, DATA, 0)

        cbitstruct.unpack_from_dict(fmt=FMT, names=NAMES, data=DATA)
        cbitstruct.unpack_from_dict(FMT, names=NAMES, data=DATA)
        cbitstruct.unpack_from_dict(FMT, NAMES, data=DATA)
        cbitstruct.unpack_from_dict(FMT, NAMES, DATA)

    def test_compiled_unpack_from_dict(self):
        CFD.unpack_from(data=DATA, offset=0)
        CFD.unpack_from(DATA, offset=0)
        CFD.unpack_from(DATA, 0)

        CFD.unpack_from(data=DATA)
        CFD.unpack_from(DATA)

    def test_compile(self):
        cbitstruct.compile(fmt=FMT, names=NAMES)
        cbitstruct.compile(FMT, names=NAMES)
        cbitstruct.compile(FMT, NAMES)

        cbitstruct.compile(fmt=FMT)
        cbitstruct.compile(FMT)

    def test_calcsize(self):
        cbitstruct.calcsize(fmt=FMT)
        cbitstruct.calcsize(FMT)

    def test_byteswap(self):
        cbitstruct.byteswap(fmt="1", data=DATA, offset=0)
        cbitstruct.byteswap("1", data=DATA, offset=0)
        cbitstruct.byteswap("1", DATA, offset=0)
        cbitstruct.byteswap("1", DATA, 0)

        cbitstruct.byteswap(fmt="1", data=DATA)
        cbitstruct.byteswap("1", data=DATA)
        cbitstruct.byteswap("1", DATA)


if __name__ == "__main__":
    unittest.main()
