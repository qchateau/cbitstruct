![License](https://img.shields.io/badge/license-GPLv3-brightgreen)  [![Build Status](https://travis-ci.com/qchateau/cbitstruct.svg?branch=master)](https://travis-ci.com/qchateau/cbitstruct) [![Coverage Status](https://coveralls.io/repos/github/qchateau/cbitstruct/badge.svg)](https://coveralls.io/github/qchateau/cbitstruct)

# About

`cbitstruct` is a C implementation of the `bitstruct` library. Credits to Erik Movist for the original `bitstruct` library available on [Github](https://github.com/eerimoq/bitstruct) and [pip](https://pypi.org/project/bitstruct/).

The goal is to provide the same API an idealy to be able to interchange `import bitstruct` and `import cbitstruct as bitstruct`.

Obvious increased performance comes with limitations described below.

# Installation

Coming soon

# Documentation

Please refer to the [`bitstruct` documentation](https://bitstruct.readthedocs.io/en/latest/) as the aim of this library is to provide the same API with increased performance.

If you are not used to `bitstruct`, you should seriously consider using it first, before migrating to this library if you require higher performance.

# Limitations

| Limitation | Will it be lifted ? |
|------------|---------------------|
| Only tested on linux | Probably |
| All types except padding are limited to 64 bits | Maybe for 'raw' and 'text' types |
| Exceptions differ from `bitstruct` | Probably not |
| Python >= 3.4 | Probably not |
| CPython only | Probably not |
| May not work on big-endian architectures | Probably not |
| Out-of-range numbers in packing operations are not detected | Probably not |
| Error messages are unclear | Will never be as clear as `bitstruct` |

Some limitations are there because I did not get the time or motivation to lift them up. Some other are deeply rooted into this library and may never be lifted.

Note that since this library is performance oriented, I will refuse changes that degrade performance (except bugfixes).

# Performance

## Comparing to `bitstruct`

The script available in `tests/test_perf.py` measures performance comparing to the `bitstruct` library.

Here are the result "on my machine" (Ubuntu in Virtualbox on a laptop):
```
byteswap list of int      | x  7.774 (   8.634us ->   1.111us)
byteswap str              | x  8.828 (  12.992us ->   1.472us)
calcsize                  | x150.863 (  60.566us ->   0.401us)
compiled pack             | x 46.257 (  35.994us ->   0.778us)
compiled pack_dict        | x 26.440 (  34.221us ->   1.294us)
compiled pack_into        | x 35.690 (  39.932us ->   1.119us)
compiled pack_into_dict   | x 25.579 (  38.404us ->   1.501us)
compiled unpack           | x 35.285 (  32.051us ->   0.908us)
compiled unpack_dict      | x 21.984 (  32.342us ->   1.471us)
compiled unpack_from      | x 32.752 (  31.353us ->   0.957us)
compiled unpack_from_dict | x 21.354 (  32.016us ->   1.499us)
pack                      | x 84.085 ( 103.511us ->   1.231us)
pack_dict                 | x 47.052 (  92.523us ->   1.966us)
pack_into                 | x 76.317 ( 105.994us ->   1.389us)
pack_into_dict            | x 41.617 (  96.210us ->   2.312us)
unpack                    | x 85.326 (  94.035us ->   1.102us)
unpack_dict               | x 41.138 (  90.074us ->   2.190us)
unpack_from               | x 82.550 (  94.934us ->   1.150us)
unpack_from_dict          | x 40.659 (  89.523us ->   2.202us)
```

*Disclaimer:* these results may and will vary largely depending on the number of elements and types you pack/unpack. This script is provided as-is, and I will gladly accept an improved script providing more reliable results.


## The dict API
The `dict` API is marginally slower than the traditional one. As the packing/unpacking performance is quite high, the overhead of performing dictionary lookups and hashing significantly increas pack and unpacking duration.