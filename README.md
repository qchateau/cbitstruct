![License](https://img.shields.io/badge/license-GPLv3-brightgreen)  [![Build Status](https://travis-ci.com/qchateau/cbitstruct.svg?branch=master)](https://travis-ci.com/qchateau/cbitstruct) [![Coverage Status](https://coveralls.io/repos/github/qchateau/cbitstruct/badge.svg)](https://coveralls.io/github/qchateau/cbitstruct)

# About

`cbitstruct` is a C implementation of the `bitstruct` library. Credits to Erik Moqvist for the original `bitstruct` library available on [Github](https://github.com/eerimoq/bitstruct) and [pip](https://pypi.org/project/bitstruct/).

The goal is to provide the same API an idealy to be able to interchange `import bitstruct` and `import cbitstruct as bitstruct`.

Obvious increased performance comes with limitations described below.

# Installation

```bash
pip3 install cbitstruct
```

# Documentation

Please refer to the [`bitstruct` documentation](https://bitstruct.readthedocs.io/en/latest/) as the aim of this library is to provide the same API with increased performance.

If you are not used to `bitstruct`, you should seriously consider using it first, before migrating to this library if you require higher performance.

# Limitations

| Limitation | Will it be lifted ? |
|------------|---------------------|
| All types except padding are limited to 64 bits | Maybe for 'raw' and 'text' types |
| May not work on big-endian architectures | Maybe |
| Exceptions differ from `bitstruct` | Probably not |
| CPython only | Probably not |
| Error messages are unclear | Will never be as clear as `bitstruct` |
| Python >= 3.5 | No |

Some limitations are there because I did not get the time or motivation to lift them up. Some other are deeply rooted into this library and may never be lifted.

# Performance

## Comparing to `bitstruct`

The script available in `tests/test_perf.py` measures performance comparing to the `bitstruct` library.

Here are the result "on my machine" (Ubuntu in Virtualbox on a laptop):
```
byteswap list of int      | x  8.204 (   9.208us ->   1.122us)
byteswap str              | x  6.433 (   9.689us ->   1.506us)
calcsize                  | x149.423 (  61.967us ->   0.415us)
compiled pack             | x 43.227 (  34.758us ->   0.804us)
compiled pack_dict        | x 26.490 (  34.951us ->   1.319us)
compiled pack_into        | x 32.017 (  39.522us ->   1.234us)
compiled pack_into_dict   | x 26.817 (  38.984us ->   1.454us)
compiled unpack           | x 34.454 (  31.814us ->   0.923us)
compiled unpack_dict      | x 23.534 (  34.071us ->   1.448us)
compiled unpack_from      | x 27.170 (  31.884us ->   1.174us)
compiled unpack_from_dict | x 22.600 (  33.927us ->   1.501us)
pack                      | x 78.314 ( 105.593us ->   1.348us)
pack_dict                 | x 52.916 ( 106.748us ->   2.017us)
pack_into                 | x 82.233 ( 119.950us ->   1.459us)
pack_into_dict            | x 45.214 ( 111.338us ->   2.462us)
unpack                    | x 82.712 (  93.686us ->   1.133us)
unpack_dict               | x 41.064 (  91.473us ->   2.228us)
unpack_from               | x 81.678 (  95.729us ->   1.172us)
unpack_from_dict          | x 40.379 (  90.430us ->   2.240us)
```

*Disclaimer:* these results may and will vary largely depending on the number of elements and types you pack/unpack. This script is provided as-is, and I will gladly accept an improved script providing more reliable results.


## The dict API
The `dict` API is marginally slower than the traditional one. As the packing/unpacking performance is quite high, the overhead of performing dictionary lookups and hashing significantly increas pack and unpacking duration.
