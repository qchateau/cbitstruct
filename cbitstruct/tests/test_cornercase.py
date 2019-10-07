import unittest
import cbitstruct


NAMES = ["foo"]
FMT = "u32"
DICT = {"foo": 42}


class BitstructApiTest(unittest.TestCase):
    def test_no_args(self):
        self.assertRaises(Exception, cbitstruct.CompiledFormat)
        self.assertRaises(Exception, cbitstruct.CompiledFormatDict)
        self.assertRaises(Exception, cbitstruct.compile)

    def test_not_unicode(self):
        self.assertRaises(UnicodeDecodeError, cbitstruct.unpack, "t8", b"\xff")

    def test_unpack_too_small(self):
        self.assertRaises(TypeError, cbitstruct.unpack, FMT, b"")

    def test_unpack_bad_args(self):
        self.assertRaises(TypeError, cbitstruct.unpack)
        self.assertRaises(TypeError, cbitstruct.unpack, None)
        self.assertRaises(TypeError, cbitstruct.unpack, FMT)
        self.assertRaises(TypeError, cbitstruct.unpack, FMT, None)
        self.assertRaises(TypeError, cbitstruct.unpack, None, bytes(4))

    def test_unpack_dict_too_small(self):
        self.assertRaises(TypeError, cbitstruct.unpack_dict, FMT, NAMES, b"")

    def test_unpack_dict_bad_args(self):
        self.assertRaises(TypeError, cbitstruct.unpack_dict)
        self.assertRaises(TypeError, cbitstruct.unpack_dict, None)
        self.assertRaises(TypeError, cbitstruct.unpack_dict, FMT, None)
        self.assertRaises(TypeError, cbitstruct.unpack_dict, FMT, NAMES, None)
        self.assertRaises(TypeError, cbitstruct.unpack_dict, None, NAMES, bytes(4))
        self.assertRaises(
            TypeError, cbitstruct.unpack_dict, FMT, NAMES + ["extra"], bytes(4)
        )

    def test_unpack_from_too_small(self):
        self.assertRaises(TypeError, cbitstruct.unpack_from, FMT, b"")

    def test_unpack_from_bad_args(self):
        self.assertRaises(TypeError, cbitstruct.unpack_from)
        self.assertRaises(TypeError, cbitstruct.unpack_from, FMT)
        self.assertRaises(TypeError, cbitstruct.unpack_from, None, bytes(4))
        self.assertRaises(TypeError, cbitstruct.unpack_from, FMT, None)

    def test_unpack_from_dict_too_small(self):
        self.assertRaises(TypeError, cbitstruct.unpack_from_dict, FMT, NAMES, b"")

    def test_unpack_from_dict_bad_args(self):
        self.assertRaises(TypeError, cbitstruct.unpack_from_dict)
        self.assertRaises(TypeError, cbitstruct.unpack_from_dict, FMT)
        self.assertRaises(TypeError, cbitstruct.unpack_from_dict, FMT, None)
        self.assertRaises(TypeError, cbitstruct.unpack_from_dict, None, NAMES)
        self.assertRaises(TypeError, cbitstruct.unpack_from_dict, FMT, None, bytes(4))
        self.assertRaises(TypeError, cbitstruct.unpack_from_dict, None, NAMES, bytes(4))
        self.assertRaises(
            TypeError, cbitstruct.unpack_from_dict, "g32", NAMES, bytes(4)
        )
        self.assertRaises(
            TypeError, cbitstruct.unpack_from_dict, FMT, NAMES + ["extra"], bytes(4)
        )
        self.assertRaises(TypeError, cbitstruct.unpack_from_dict, FMT, NAMES, None)

    def test_pack_bad_args(self):
        self.assertRaises(TypeError, cbitstruct.pack)
        self.assertRaises(TypeError, cbitstruct.pack, 0)

    def test_pack_into_too_small(self):
        self.assertRaises(TypeError, cbitstruct.pack_into, FMT, b"", 12)

    def test_pack_into_bad_args(self):
        self.assertRaises(TypeError, cbitstruct.pack_into)
        self.assertRaises(TypeError, cbitstruct.pack_into, FMT)
        self.assertRaises(TypeError, cbitstruct.pack_into, FMT, bytes(4))
        self.assertRaises(TypeError, cbitstruct.pack_into, FMT, bytes(4), 0)
        self.assertRaises(TypeError, cbitstruct.pack_into, None, bytes(4), 0, 12)
        self.assertRaises(TypeError, cbitstruct.pack_into, "g32", bytes(4), 0, 12)

    def test_pack_into_dict_too_small(self):
        self.assertRaises(
            TypeError, cbitstruct.pack_into_dict, FMT, NAMES, bytes(0), 0, DICT
        )

    def test_pack_into_dict_bad_args(self):
        self.assertRaises(TypeError, cbitstruct.pack_into_dict)
        self.assertRaises(TypeError, cbitstruct.pack_into_dict, FMT)
        self.assertRaises(TypeError, cbitstruct.pack_into_dict, FMT, None)
        self.assertRaises(TypeError, cbitstruct.pack_into_dict, None, NAMES)
        self.assertRaises(TypeError, cbitstruct.pack_into_dict, FMT, NAMES, bytes(4), 0)
        self.assertRaises(
            TypeError, cbitstruct.pack_into_dict, FMT, None, bytes(4), 0, DICT
        )
        self.assertRaises(
            TypeError, cbitstruct.pack_into_dict, None, NAMES, bytes(4), 0, DICT
        )
        self.assertRaises(
            TypeError, cbitstruct.pack_into_dict, "g32", NAMES, bytes(4), 0, DICT
        )

    def test_pack_dict_bad_args(self):
        self.assertRaises(TypeError, cbitstruct.pack_dict)
        self.assertRaises(TypeError, cbitstruct.pack_dict, FMT)
        self.assertRaises(TypeError, cbitstruct.pack_dict, FMT, None)
        self.assertRaises(TypeError, cbitstruct.pack_dict, None, NAMES)
        self.assertRaises(TypeError, cbitstruct.pack_dict, FMT, NAMES)
        self.assertRaises(TypeError, cbitstruct.pack_dict, FMT, None, DICT)
        self.assertRaises(TypeError, cbitstruct.pack_dict, None, NAMES, DICT)
        self.assertRaises(TypeError, cbitstruct.pack_dict, "g32", NAMES, DICT)

    def test_byteswap_bad_args(self):
        self.assertRaises(TypeError, cbitstruct.byteswap, "23", b"")
        self.assertRaises(TypeError, cbitstruct.byteswap, None, b"\xff")
        self.assertRaises(TypeError, cbitstruct.byteswap, "23")
        self.assertRaises(TypeError, cbitstruct.byteswap)
        self.assertRaises(ValueError, cbitstruct.byteswap, "\x02\x02", b"z")

    def test_calcsize_bad_args(self):
        self.assertRaises(TypeError, cbitstruct.calcsize, "g32")
        self.assertRaises(TypeError, cbitstruct.calcsize, None)
        self.assertRaises(TypeError, cbitstruct.calcsize)


if __name__ == "__main__":
    unittest.main()
