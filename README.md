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
byteswap list of int      | x  8.779 (   8.638us ->   0.984us)
byteswap str              | x 17.466 (   9.158us ->   0.524us)
calcsize                  | x139.330 (  61.060us ->   0.438us)
compiled pack             | x 47.389 (  35.968us ->   0.759us)
compiled pack_dict        | x 27.184 (  34.588us ->   1.272us)
compiled pack_into        | x 32.037 (  38.650us ->   1.206us)
compiled pack_into_dict   | x 27.343 (  37.718us ->   1.379us)
compiled unpack           | x 33.928 (  31.278us ->   0.922us)
compiled unpack_dict      | x 21.627 (  31.597us ->   1.461us)
compiled unpack_from      | x 30.622 (  29.977us ->   0.979us)
compiled unpack_from_dict | x 20.479 (  30.936us ->   1.511us)
pack                      | x 77.003 ( 103.030us ->   1.338us)
pack_dict                 | x 53.254 ( 103.255us ->   1.939us)
pack_into                 | x 82.829 ( 119.373us ->   1.441us)
pack_into_dict            | x 52.173 ( 108.135us ->   2.073us)
unpack                    | x 78.459 (  91.896us ->   1.171us)
unpack_dict               | x 40.287 (  89.300us ->   2.217us)
unpack_from               | x 77.027 (  91.202us ->   1.184us)
unpack_from_dict          | x 39.467 (  88.043us ->   2.231us)
```

*Disclaimer:* these results may and will vary largely depending on the number of elements and types you pack/unpack. This script is provided as-is, and I will gladly accept an improved script providing more reliable results.


## The dict API
The `dict` API is marginally slower than the traditional one. As the packing/unpacking performance is quite high, the overhead of performing dictionary lookups and hashing significantly increas pack and unpacking duration.
