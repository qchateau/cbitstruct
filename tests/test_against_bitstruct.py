import unittest
import random
import cbitstruct
import bitstruct
import string
import math
import copy
import sys


MAX_BITS = 64
MAX_OFFSET = 16
MAX_ELEMENTS = 100
ENDIANNESS = ["", ">", "<"]
BITORDER = ["", ">", "<"]
FLOAT_BITS = (
    [16, 32, 64] if sys.version_info[0] == 3 and sys.version_info[1] >= 6 else [32, 64]
)
TEXT_CHOICES = string.ascii_letters + string.digits


def random_bits(nbits):
    nbytes = (nbits + 7) // 8
    return bytes([random.randint(0, 255) for _ in range(nbytes)])


NBITS_GENERATORS = {
    "u": lambda: random.randint(1, MAX_BITS),
    "s": lambda: random.randint(1, MAX_BITS),
    "f": lambda: random.choice(FLOAT_BITS),
    # bitstruct does not handle incomplete bytes and text very consitently
    # and has no unit tests against it, so just don't check these cases
    "t": lambda: 8 * random.randint(1, MAX_BITS // 8),
    "r": lambda: 8 * random.randint(1, MAX_BITS // 8),
    "p": lambda: random.randint(1, 1000),
    "P": lambda: random.randint(1, 1000),
}
VALUE_GENERATORS = {
    "u": lambda n: random.randint(0, 2 ** n - 1),
    "s": lambda n: random.randint(-2 ** (n - 1), 2 ** (n - 1) - 1),
    "f": lambda n: random.uniform(-100, 100),
    "t": lambda n: "".join([random.choice(TEXT_CHOICES) for _ in range(n // 8)]),
    "r": lambda n: random_bits(n),
    "p": lambda n: None,
    "P": lambda n: None,
}


def generate_fmt():
    nelements = random.randint(1, MAX_ELEMENTS)
    endianness = random.choice(ENDIANNESS)

    fmt = ""
    values = list()
    for _ in range(nelements):
        one_fmt, one_value = generate_one()
        fmt += one_fmt
        if one_value is not None:
            values.append(one_value)
    fmt += endianness
    return fmt, values


def generate_one():
    bits_order = random.choice(BITORDER)
    letter = random.choice(list(NBITS_GENERATORS.keys()))
    nbits = NBITS_GENERATORS[letter]()
    value = VALUE_GENERATORS[letter](nbits)
    return bits_order + letter + str(nbits), value


class BitstructConformityTest(unittest.TestCase):
    def test_random_pack_unpack(self):
        for _ in range(1000):
            self.assert_pack_unpack()
            self.assert_pack_into_unpack_from()

    def test_byteswap(self):
        for _ in range(1000):
            self.assert_byteswap_str()
            self.assert_byteswap_integer()

    def assert_pack_unpack(self):
        fmt, values = generate_fmt()

        expected_data = bitstruct.pack(fmt, *values)
        result_data = cbitstruct.pack(fmt, *values)
        self.assertEqual(expected_data, result_data, "bad pack for '{}'".format(fmt))

        expected_values = bitstruct.unpack(fmt, expected_data)
        result_values = cbitstruct.unpack(fmt, expected_data)
        self.assertValuesEqual(
            expected_values, result_values, "bad unpack for '{}'".format(fmt)
        )

    def assert_pack_into_unpack_from(self):
        fmt, values = generate_fmt()
        offset = random.randint(0, MAX_OFFSET)
        fill_padding = random.choice([True, False])

        dst = bytearray(random_bits(bitstruct.calcsize(fmt) + offset))
        expected_data = dst.copy()
        result_data = dst.copy()

        bitstruct.pack_into(
            fmt, expected_data, offset, *values, fill_padding=fill_padding
        )
        cbitstruct.pack_into(
            fmt, result_data, offset, *values, fill_padding=fill_padding
        )
        self.assertEqual(expected_data, result_data, "bad pack_into for '{}'".format(fmt))

        expected_values = bitstruct.unpack_from(fmt, expected_data, offset)
        result_values = cbitstruct.unpack_from(fmt, expected_data, offset)
        self.assertValuesEqual(
            expected_values, result_values, "bad unpack_from for '{}'".format(fmt)
        )

    def assertValuesEqual(self, l1, l2, msg):
        for v1, v2 in zip(l1, l2):
            if (
                isinstance(v1, float)
                and isinstance(v2, float)
                and math.isnan(v1)
                and math.isnan(v2)
            ):
                continue
            self.assertAlmostEqual(v1, v2, msg=msg)

    def assert_byteswap_str(self):
        MAX_BYTES = 300
        offset = random.randint(0, MAX_OFFSET)
        nbytes = random.randint(1, MAX_BYTES)
        data = random_bits((nbytes + offset) * 8)

        fmt = ""
        while nbytes > 0:
            n = random.randint(1, min(nbytes, 9))
            fmt += str(n)
            nbytes -= n

        expected = bitstruct.byteswap(fmt, data, offset)
        result = cbitstruct.byteswap(fmt, data, offset)
        self.assertEqual(expected, result, "bad byteswap for '{}'".format(fmt))

    def assert_byteswap_integer(self):
        MAX_BYTES = 300
        offset = random.randint(0, MAX_OFFSET)
        nbytes = random.randint(1, MAX_BYTES)
        data = random_bits((nbytes + offset) * 8)

        fmt = list()
        while nbytes > 0:
            n = random.randint(1, nbytes)
            fmt.append(n)
            nbytes -= n

        expected = bitstruct.byteswap(fmt, data, offset)
        result = cbitstruct.byteswap(fmt, data, offset)
        self.assertEqual(expected, result, "bad byteswap for '{}'".format(fmt))


if __name__ == "__main__":
    unittest.main()
