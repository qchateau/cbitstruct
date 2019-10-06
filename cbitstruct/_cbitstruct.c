#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define PY_SSIZE_T_CLEAN

#include <Python.h>

#define BIT_MASK(__TYPE__, __ONE_COUNT__)  \
    (((__TYPE__)(-((__ONE_COUNT__) != 0))) \
     & (((__TYPE__)-1) >> ((sizeof(__TYPE__) * CHAR_BIT) - (__ONE_COUNT__))))

#define BIT_SET(__TYPE__, __ONE_POS__) ((__TYPE__)(1) << (__ONE_POS__))

#define BAD_FORMAT ('\0')
#define FILL_PADDING ("fill_padding")
#define SMALL_FORMAT_OPTIMIZATION (16)

// #define PRINT_DEBUG
#ifdef PRINT_DEBUG
#define DEBUG(...) (printf(__VA_ARGS__))
#else
#define DEBUG(...)
#endif

typedef struct {
    int bits;
    char type;
    bool msb_first;
} Desc;

typedef struct {
    Desc* descs;
    int ndescs;
    int npadding;
    int nbits;
    bool be;
    bool valid;
} CompiledFormat;

typedef union {
    uint8_t raw[8];
    uint64_t uint64;
    int64_t int64;
    float fp32;
    double fp64;
} ParsedElement;

static int fast_strtoi(const char* str, const char** end)
{
    int val = 0;

    while (*str && *str >= '0' && *str <= '9') {
        val = val * 10 + (*str++ - '0');
    }

    *end = str;
    return val;
}

static const char* c_find_next(const char* fmt)
{
    while (*fmt == ' ') {
        ++fmt;
    }
    return fmt;
}

static Desc c_parse_one(const char* fmt, const char** end, Desc* previous)
{
    Desc desc;
    if (*fmt == '<') {
        desc.msb_first = false;
        ++fmt;
    }
    else if (*fmt == '>') {
        desc.msb_first = true;
        ++fmt;
    }
    else if (previous) {
        desc.msb_first = previous->msb_first;
    }
    else {
        desc.msb_first = true;
    }
    desc.type = *fmt++;
    desc.bits = fast_strtoi(fmt, end);

    switch (desc.type) {
    case 'u':
    case 's':
    case 'b':
    case 't':
    case 'r':
        if (desc.bits < 1 || desc.bits > 64) {
            // bits > 64 is only supported for padding
            desc.type = BAD_FORMAT;
        }
        break;
    case 'p':
    case 'P':
        break;
    case 'f':
        if (desc.bits != 16 && desc.bits != 32 && desc.bits != 64) {
            desc.type = BAD_FORMAT;
        }
        break;
    default:
        desc.type = BAD_FORMAT;
        break;
    }
    return desc;
}

static int c_count_elements(const char* fmt)
{
    int nr = 0;
    while (*fmt != '\0') {
        if (*fmt >= 'A' && *fmt <= 'z') {
            // Assume this is an element
            ++nr;
        }
        ++fmt;
    }
    return nr;
}

static CompiledFormat c_compile_format(const char* fmt)
{
    CompiledFormat compiled = {NULL, 0, 0, 0, true, true};

    const int n = c_count_elements(fmt);
    if (n <= 0 && *fmt != '>' && *fmt != '<' && *fmt != '\0') {
        compiled.valid = false;
        goto invalid;
    }

    compiled.descs = PyMem_RawMalloc(n * sizeof(Desc));
    if (!compiled.descs) {
        PyErr_NoMemory();
        goto invalid;
    }
    compiled.ndescs = n;

    for (int i = 0; i < n; ++i) {
        fmt = c_find_next(fmt);
        const char* end;
        compiled.descs[i] =
            c_parse_one(fmt, &end, i > 0 ? &compiled.descs[i - 1] : NULL);
        if (compiled.descs[i].type == BAD_FORMAT) {
            goto invalid;
        }
        if (compiled.descs[i].type == 'p' || compiled.descs[i].type == 'P') {
            compiled.npadding++;
        }
        compiled.nbits += compiled.descs[i].bits;
        fmt = end;
    }

    fmt = c_find_next(fmt);
    if (*fmt == '<') {
        compiled.be = false;
    }

    return compiled;

invalid:
    compiled.valid = false;
    return compiled;
}

static void c_release_compiled_format(CompiledFormat* compiled)
{
    if (compiled->descs) {
        PyMem_RawFree(compiled->descs);
        compiled->descs = NULL;
        compiled->ndescs = 0;
    }
}

static uint64_t c_bitswitch(uint64_t data, int nbits)
{
    uint64_t out = 0;
    for (int i = 0; i < nbits; ++i) {
        int bit = data & 1;
        out <<= 1;
        out |= bit;
        data >>= 1;
    }
    return out;
}

static uint64_t c_partial_c_byteswitch(uint64_t data, int nbits, int bits_in_lsb)
{
    uint64_t out = 0;
    int bits = bits_in_lsb;

    while (nbits > 0) {
        int byte = data & BIT_MASK(uint8_t, bits);
        out <<= bits;
        out |= byte;
        data >>= bits;
        nbits -= bits;
        bits = nbits > 8 ? 8 : nbits;
    }

    return out;
}

static void c_byteswitch(uint8_t* data, int nbytes)
{
    uint8_t* end = data + nbytes - 1;
    for (int i = 0; i < nbytes / 2; ++i) {
        uint8_t tmp = end[-i];
        end[-i] = data[i];
        data[i] = tmp;
    }
}

static void c_bitset(uint8_t* dst, int dst_bit_offset, int nbits)
{
    dst += dst_bit_offset / 8;
    dst_bit_offset %= 8;
    int first_byte_bits = 8 - dst_bit_offset;
    int n_full_bytes = (nbits - first_byte_bits) / 8;
    int last_byte_bits = (nbits - first_byte_bits) % 8;

    uint8_t first_byte = BIT_MASK(uint8_t, first_byte_bits);
    if (last_byte_bits < 0) {
        // when the elements fits on a single byte and does not
        // fill all bits, "-last_byte_bits" is the right padding
        first_byte &= ~BIT_MASK(uint8_t, -last_byte_bits);
    }
    *dst++ |= first_byte;

    for (int n = 0; n < n_full_bytes; ++n) {
        *dst++ = 0xff;
    }

    if (last_byte_bits > 0) {
        uint8_t last_byte = BIT_MASK(uint8_t, last_byte_bits);
        last_byte <<= 8 - last_byte_bits;
        *dst |= last_byte;
    }
}

static void c_bitcpy(uint8_t* dst, int dst_bit_offset, uint64_t data, int nbits)
{
    dst += dst_bit_offset / 8;
    dst_bit_offset %= 8;
    int first_byte_bits = 8 - dst_bit_offset;
    int n_full_bytes = (nbits - first_byte_bits) / 8;
    int last_byte_bits = (nbits - first_byte_bits) % 8;

    dst += n_full_bytes + (last_byte_bits > 0 ? 1 : 0);

    if (last_byte_bits > 0) {
        uint8_t last_byte_mask = BIT_MASK(uint8_t, last_byte_bits);
        uint8_t last_byte = data & last_byte_mask;

        last_byte <<= 8 - last_byte_bits;
        last_byte_mask <<= 8 - last_byte_bits;
        *dst &= ~last_byte_mask;
        *dst |= last_byte & last_byte_mask;
        data >>= last_byte_bits;
        --dst;
    }

    for (int n = 0; n < n_full_bytes; ++n) {
        uint8_t byte = data & 0xff;
        *dst-- = byte;
        data >>= 8;
    }

    uint8_t first_byte_mask = BIT_MASK(uint8_t, first_byte_bits);
    uint8_t first_byte = data & first_byte_mask;

    if (last_byte_bits < 0) {
        // when the elements fits on a single byte and does not
        // fill all bits, "-last_byte_bits" is the right padding
        first_byte_mask &= ~BIT_MASK(uint8_t, -last_byte_bits);
        first_byte <<= -last_byte_bits;
    }

    *dst &= ~first_byte_mask;
    *dst |= first_byte & first_byte_mask;
}

static void c_pack(
    uint8_t* out,
    const ParsedElement* elements,
    CompiledFormat fmt,
    int offset,
    bool fill_padding)
{
    int bit = offset;
    bool wrong_endianness = fmt.be ^ PY_LITTLE_ENDIAN;

    for (int n = 0; n < fmt.ndescs; ++n) {
        const ParsedElement* el = elements + n;
        const Desc* desc = fmt.descs + n;
        const int bitoff = bit;
        uint64_t data = el->uint64;
        bit += desc->bits;

        if (desc->type == 'p') {
            if (fill_padding) {
                c_bitcpy(out, bitoff, 0, desc->bits);
            }
            continue;
        }
        else if (desc->type == 'P') {
            if (fill_padding) {
                c_bitset(out, bitoff, desc->bits);
            }
            continue;
        }

        if (desc->type == 'r' || desc->type == 't') {
            int nbytes = (desc->bits + 7) / 8;
            int padding = nbytes * 8 - desc->bits;
#if PY_LITTLE_ENDIAN
            assert(nbytes <= sizeof(data));
            c_byteswitch((uint8_t*)&data, nbytes);
#endif
            data >>= padding;
        }

        // Switch bits if necessary
        if (!desc->msb_first) {
            data = c_bitswitch(data, desc->bits);
        }

        // Correct source data endianness, except for raw and text
        // which are special cases, handled later.
        if (desc->type != 'r' && desc->type != 't' && wrong_endianness) {
            // bs is the first byte
            int bits_in_lsb = 8 - (bitoff % 8);
            data = c_partial_c_byteswitch(data, desc->bits, bits_in_lsb);
        }

        c_bitcpy(out, bitoff, data, desc->bits);
    }
}

static void c_unpack(
    ParsedElement* out,
    const uint8_t* bytes,
    CompiledFormat fmt,
    int offset)
{
    int bit = offset;
    bool wrong_endianness = fmt.be ^ PY_LITTLE_ENDIAN;

    for (int n = 0; n < fmt.ndescs; ++n) {
        const Desc* desc = fmt.descs + n;

        const uint8_t* src = bytes + bit / 8;
        int first_byte_bits = 8 - (bit % 8);
        int n_full_bytes = (desc->bits - first_byte_bits) / 8;
        int last_byte_bits = (desc->bits - first_byte_bits) % 8;
        bit += desc->bits;

        if (desc->type == 'p' || desc->type == 'P') {
            // padding, ignore
            continue;
        }

        uint64_t data = 0;
        {
            uint8_t byte = *src++ & BIT_MASK(uint8_t, first_byte_bits);
            data |= byte;
        }

        for (int i = 0; i < n_full_bytes; ++i) {
            data <<= 8;
            data |= *src++;
        }

        if (last_byte_bits > 0) {
            uint8_t byte = *src;
            byte >>= 8 - last_byte_bits;
            data <<= last_byte_bits;
            data |= byte;
        }
        else {
            data >>= -last_byte_bits;
        }

        // Correct source data endianness, except for raw and text
        // which are special cases, handled later.
        if (desc->type != 'r' && desc->type != 't' && wrong_endianness) {
            // lsb is the "last_byte", the lsb of the host system
            // +16 because last_byte_bits can go down to -7, and -(offset%8) as well
            int bits_in_lsb = (last_byte_bits + 16 - (offset % 8)) % 8;
            data = c_partial_c_byteswitch(data, desc->bits, bits_in_lsb);
        }

        // Switch bits if necessary
        if (!desc->msb_first) {
            data = c_bitswitch(data, desc->bits);
        }

        // For raw and text, add right padding (as bitstruct) and
        // correct endianness. These types are not influenced
        // by the source data endianness, only the host system
        if (desc->type == 'r' || desc->type == 't') {
            int nbytes = (desc->bits + 7) / 8;
            int padding = nbytes * 8 - desc->bits;
            data <<= padding;
#if PY_LITTLE_ENDIAN
            assert(nbytes <= sizeof(data));
            c_byteswitch((uint8_t*)&data, nbytes);
#endif
        }

        if (desc->type == 's' && desc->bits < 64) {
            uint64_t sign_bit = BIT_SET(uint64_t, desc->bits - 1);
            if (data & sign_bit) {
                // two's complement: replace high bits by ones
                data |= BIT_MASK(uint64_t, 64) << desc->bits;
            }
        }

        out[n].uint64 = data;
    }
}

/* Python conversions */

static bool unsigned_in_range(uint64_t n, int bits)
{
    if (bits == 64) {
        return true;
    }
    return n < (1ull << bits);
}

static bool signed_in_range(int64_t n, int bits)
{
    if (bits == 64) {
        return true;
    }
    if (n > 0) {
        return n < (1ll << (bits - 1));
    }
    else {
        return -n <= (1ll << (bits - 1));
    }
}

static bool python_to_parsed_elements(
    ParsedElement* elements,
    PyObject** data,
    Py_ssize_t data_size,
    CompiledFormat fmt)
{
    assert(data_size >= fmt.ndescs);

    int n = 0;
    for (int i = 0; i < fmt.ndescs; ++i) {
        Desc* desc = fmt.descs + i;
        ParsedElement* el = elements + i;
        // zero all bits
        el->uint64 = 0;

        if (desc->type == 'p' || desc->type == 'P') {
            continue;
        }

        PyObject* v = data[n++];

        switch (desc->type) {
        case 'u':
#if SIZEOF_LONG >= 8
            el->uint64 = PyLong_AsUnsignedLong(v);
#else
            el->uint64 = PyLong_AsUnsignedLongLong(v);
#endif // SIZEOF_LONG >= 8
            if (!unsigned_in_range(el->uint64, desc->bits)) {
                PyErr_SetString(PyExc_TypeError, "integer is out of range");
            }
            break;
        case 's':
#if SIZEOF_LONG >= 8
            el->int64 = PyLong_AsLong(v);
#else
            el->int64 = PyLong_AsLongLong(v);
#endif // SIZEOF_LONG >= 8
            if (!signed_in_range(el->int64, desc->bits)) {
                PyErr_SetString(PyExc_TypeError, "integer is out of range");
            }
            break;
        case 'f':
#if PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 6
            if (desc->bits == 16) {
                double cv = PyFloat_AsDouble(v);
                _PyFloat_Pack2(cv, el->raw, PY_LITTLE_ENDIAN);
            }
            else
#endif // PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 6
                if (desc->bits == 32) {
                el->fp32 = PyFloat_AsDouble(v);
            }
            else if (desc->bits == 64) {
                el->fp64 = PyFloat_AsDouble(v);
            }
            break;
        case 'b':
            el->uint64 = PyObject_IsTrue(v);
            break;
        case 't': {
            Py_ssize_t size;
            const char* data = PyUnicode_AsUTF8AndSize(v, &size);
            if (data) {
                int max_size = sizeof(el->raw);
                int cpysize = size < max_size ? size : max_size;
                memcpy(el->raw, data, cpysize);
            }
        } break;
        case 'r': {
            Py_ssize_t size = PyBytes_Size(v);
            char* data = PyBytes_AsString(v);
            if (data) {
                int max_size = sizeof(el->raw);
                int cpysize = size < max_size ? size : max_size;
                memcpy(el->raw, data, cpysize);
            }
        } break;
        default:
            return false;
        };

        if (PyErr_Occurred()) {
            return false;
        }
    }

    return true;
}

static PyObject* parsed_elements_to_python(ParsedElement* elements, CompiledFormat fmt)
{
    PyObject* result = PyTuple_New(fmt.ndescs - fmt.npadding);
    if (result == NULL) {
        PyErr_NoMemory();
        return NULL;
    }

    int j = 0;
    for (int i = 0; i < fmt.ndescs; ++i) {
        PyObject* v = NULL;
        ParsedElement* el = elements + i;
        Desc* desc = fmt.descs + i;

        switch (desc->type) {
        case 'u':
#if SIZEOF_LONG >= 8
            v = PyLong_FromUnsignedLong(el->uint64);
#else
            v = PyLong_FromUnsignedLongLong(el->uint64);
#endif // SIZEOF_LONG >= 8
            break;
        case 's':
#if SIZEOF_LONG >= 8
            v = PyLong_FromLong(el->int64);
#else
            v = PyLong_FromLongLong(el->int64);
#endif // SIZEOF_LONG >= 8
            break;
        case 'f':
#if PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 6
            if (desc->bits == 16) {
                double cv = _PyFloat_Unpack2(el->raw, PY_LITTLE_ENDIAN);
                if (cv == -1.0 && PyErr_Occurred()) {
                    break;
                }
                v = PyFloat_FromDouble(cv);
            }
            else
#endif // PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 6
                if (desc->bits == 32) {
                v = PyFloat_FromDouble(el->fp32);
            }
            else if (desc->bits == 64) {
                v = PyFloat_FromDouble(el->fp64);
            }
            break;
        case 'b':
            v = PyBool_FromLong(!!el->uint64);
            break;
        case 't':
            v = PyUnicode_FromStringAndSize(
                (const char*)el->raw, (desc->bits + 7) / 8);
            break;
        case 'r':
            v = PyBytes_FromStringAndSize(
                (const char*)el->raw, (desc->bits + 7) / 8);
            break;
        case 'p':
        case 'P':
            // Padding, ignore
            continue;
        };

        if (!v) {
            if (!PyErr_Occurred()) {
                // Set our own exception
                PyErr_SetString(PyExc_TypeError, "to-python conversion error");
            }
            Py_DECREF(result);
            return NULL;
        }

        PyTuple_SET_ITEM(result, j++, v);
    }

    return result;
}

// Modified version of PyArg_ParseTupleAndKeywords
// to parse 'n' arguments from args or kwargs and return
// the number of arguments parsed from 'args'
static Py_ssize_t PyArg_ParseTupleAndKeywordsFirstN(
    PyObject* args,
    PyObject* kwargs,
    const char* format,
    char* keywords[],
    int n,
    ...)
{
    va_list varargs;
    va_start(varargs, n);

    Py_ssize_t return_value = -1;
    Py_ssize_t nkwargs = kwargs ? PyObject_Length(kwargs) : 0;
    Py_ssize_t n_actual_args = n - nkwargs;

    if (PyTuple_GET_SIZE(args) < n_actual_args) {
        PyErr_SetString(PyExc_TypeError, "Not enough arguments");
        goto exit;
    }

    PyObject* actual_args = PyTuple_GetSlice(args, 0, n_actual_args);
    if (!actual_args) {
        PyErr_NoMemory();
        goto exit;
    }

    if (PyArg_VaParseTupleAndKeywords(
            actual_args, kwargs, format, keywords, varargs)) {
    }
    Py_DECREF(actual_args);
    return_value = n_actual_args;

exit:
    va_end(varargs);
    return return_value;
}

static bool PopFillPadding(PyObject* kwargs)
{
    // get the fill_padding value and remove
    // it from kwargs as it makes parsing painful
    bool fill_padding = true;
    if (kwargs) {
        PyObject* py_fill_padding = PyDict_GetItemString(kwargs, FILL_PADDING);
        if (py_fill_padding) {
            fill_padding = PyObject_IsTrue(py_fill_padding);
            PyDict_DelItemString(kwargs, FILL_PADDING);
        }
    }
    return fill_padding;
}

static PyObject* CompiledFormat_pack_raw(
    CompiledFormat compiled_fmt,
    PyObject** data,
    Py_ssize_t n_data)
{
    assert(PyTuple_Check(args));

    ParsedElement elements_stack[SMALL_FORMAT_OPTIMIZATION];
    ParsedElement* elements = elements_stack;
    bool use_stack = compiled_fmt.ndescs <= SMALL_FORMAT_OPTIMIZATION;
    PyObject* bytes = NULL;

    int expected_size = compiled_fmt.ndescs - compiled_fmt.npadding;
    if (n_data < expected_size) {
        PyErr_Format(
            PyExc_TypeError,
            "pack() expected %d arguments (got %ld)",
            expected_size,
            n_data);
        return NULL;
    }

    if (!use_stack) {
        elements = PyMem_RawMalloc(compiled_fmt.ndescs * sizeof(ParsedElement));
        if (!elements) {
            PyErr_NoMemory();
            return NULL;
        }
    }

    if (!python_to_parsed_elements(elements, data, n_data, compiled_fmt)) {
        PyErr_SetString(PyExc_TypeError, "failed to parse arguments");
        goto exit;
    }

    int nbytes = (compiled_fmt.nbits + 7) / 8;
    bytes = PyBytes_FromStringAndSize(NULL, nbytes);
    if (!bytes) {
        PyErr_NoMemory();
        goto exit;
    }

    PyBytes_AS_STRING(bytes)[nbytes - 1] = 0;
    c_pack((uint8_t*)PyBytes_AS_STRING(bytes), elements, compiled_fmt, 0, true);

exit:
    if (!use_stack) {
        PyMem_RawFree(elements);
    }

    return bytes;
}

static PyObject* CompiledFormat_pack_into_raw(
    CompiledFormat compiled_fmt,
    Py_buffer* buffer,
    Py_ssize_t offset,
    PyObject** data_args,
    Py_ssize_t n_data_args,
    bool fill_padding)
{
    ParsedElement elements_stack[SMALL_FORMAT_OPTIMIZATION];
    ParsedElement* elements = elements_stack;
    bool use_stack = compiled_fmt.ndescs <= SMALL_FORMAT_OPTIMIZATION;
    PyObject* return_value = NULL;

    int expected_size = compiled_fmt.ndescs - compiled_fmt.npadding;
    if (n_data_args < expected_size) {
        PyErr_Format(
            PyExc_TypeError,
            "expected %d data arguments (got %ld)",
            expected_size,
            n_data_args);
        goto exit;
    }

    if (!PyBuffer_IsContiguous(buffer, 'C')) {
        PyErr_Format(PyExc_TypeError, "required a contiguous buffer");
        goto exit;
    }

    int nbytes = (compiled_fmt.nbits + 7) / 8;
    if (buffer->len < nbytes) {
        PyErr_Format(
            PyExc_TypeError, "required a buffer of at least %d bytes", nbytes);
        goto exit;
    }

    if (!use_stack) {
        elements = PyMem_RawMalloc(compiled_fmt.ndescs * sizeof(ParsedElement));
        if (!elements) {
            PyErr_NoMemory();
            goto exit;
        }
    }

    if (!python_to_parsed_elements(elements, data_args, n_data_args, compiled_fmt)) {
        // python_to_parsed_elements should set the exception
        goto exit;
    }

    c_pack((uint8_t*)buffer->buf, elements, compiled_fmt, offset, fill_padding);

    return_value = Py_None;
    Py_INCREF(Py_None);

exit:
    if (!use_stack && elements) {
        PyMem_RawFree(elements);
    }

    return return_value;
}

/* Python methods */

// clang-format off
/*[clinic input]
class CompiledFormat "PyCompiledFormatObject *" "&PyStructType"
[clinic start generated code]*/
/*[clinic end generated code: output=da39a3ee5e6b4b0d input=470ab77e2b50e7be]*/
// clang-format on

// clang-format off
typedef struct {
    PyObject_HEAD
    CompiledFormat compiled_fmt;
} PyCompiledFormatObject;
// clang-format on

// clang-format off
/*[clinic input]
class CompiledFormatDict "PyCompiledFormatDictObject *" "&PyStructType"
[clinic start generated code]*/
/*[clinic end generated code: output=da39a3ee5e6b4b0d input=fd49c75ad758a8d9]*/
// clang-format on

typedef struct {
    PyCompiledFormatObject super;
    PyObject* names;
} PyCompiledFormatDictObject;

#if PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 7
#include "clinic/_cbitstruct.c.37.h"
#elif PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION == 6
#include "clinic/_cbitstruct.c.36.h"
#elif PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION <= 5
#include "clinic/_cbitstruct.c.35.h"
#else
#error "Unsupported python version"
#endif

/* CompiledFormat */

// clang-format off
/*[clinic input]
CompiledFormat.__init__

    fmt: str

Create a compiled bitstruct object.

Return a new CompiledFormat object which writes and reads binary data
according to the format string.
[clinic start generated code]*/

static int
CompiledFormat___init___impl(PyCompiledFormatObject *self, const char *fmt)
/*[clinic end generated code: output=cd0e3acd08d00e64 input=2beef0b1a8e9bed3]*/
// clang-format on
{
    self->compiled_fmt = c_compile_format(fmt);
    if (!self->compiled_fmt.valid) {
        PyErr_SetString(PyExc_TypeError, "bad or unsupported format");
        return -1;
    }

    return 0;
}

static PyObject* CompiledFormat_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    return type->tp_alloc(type, 0);
}

static void CompiledFormat_deinit(PyCompiledFormatObject* self)
{
    c_release_compiled_format(&self->compiled_fmt);
}

static void CompiledFormat_dealloc(PyCompiledFormatObject* self)
{
    CompiledFormat_deinit(self);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

// clang-format off
/*[clinic input]
CompiledFormat.calcsize -> Py_ssize_t

Return size in bits of the bitDescribed by the format string.
[clinic start generated code]*/

static Py_ssize_t
CompiledFormat_calcsize_impl(PyCompiledFormatObject *self)
/*[clinic end generated code: output=6370799a72a08ece input=3eb320f6268226f2]*/
// clang-format on
{
    return self->compiled_fmt.nbits;
}

// clang-format off
PyDoc_STRVAR(CompiledFormat_pack__doc__,
"pack($self, *args)\n"
"--\n"
"\n"
"Pack args into a bytes object.");
// clang-format on
static PyObject* CompiledFormat_pack(PyCompiledFormatObject* self, PyObject* args)
{
    PyObject** data = PySequence_Fast_ITEMS(args);
    Py_ssize_t n_data = PyTuple_GET_SIZE(args);
    return CompiledFormat_pack_raw(self->compiled_fmt, data, n_data);
}

// clang-format off
PyDoc_STRVAR(CompiledFormat_pack_into__doc__,
"pack_into($self, buf, offset, *args, **kwargs)\n"
"--\n"
"\n"
"Pack args into a bytes object, starting at bit offset given by the\n"
"offset argument. An optional 'fill_padding=False' argument can be given\n"
"to keep padding bits from 'buf' as-is.");
// clang-format on
static PyObject* CompiledFormat_pack_into(
    PyCompiledFormatObject* self,
    PyObject* args,
    PyObject* kwargs)
{
    PyObject* return_value = NULL;
    Py_buffer buffer = {NULL, NULL};
    Py_ssize_t offset = 0;

    bool fill_padding = PopFillPadding(kwargs);
    Py_ssize_t n_args = PyTuple_GET_SIZE(args);
    PyObject** data = PySequence_Fast_ITEMS(args);

    static char* _keywords[] = {"buf", "offset", NULL};
    // custom (and vague) error message as all other 'pack_into'
    // versions are processed by this function. Using the default
    // error message would give bad information to the user
    Py_ssize_t n_args_parsed = PyArg_ParseTupleAndKeywordsFirstN(
        args, kwargs, "y*n:pack_into", _keywords, 2, &buffer, &offset);
    if (n_args_parsed < 0) {
        goto exit;
    }

    return_value = CompiledFormat_pack_into_raw(
        self->compiled_fmt,
        &buffer,
        offset,
        data + n_args_parsed,
        n_args - n_args_parsed,
        fill_padding);

exit:
    if (buffer.obj) {
        PyBuffer_Release(&buffer);
    }
    return return_value;
}

// clang-format off
/*[clinic input]
CompiledFormat.unpack

    data: Py_buffer

Return a tuple containing unpacked values.
[clinic start generated code]*/

static PyObject *
CompiledFormat_unpack_impl(PyCompiledFormatObject *self, Py_buffer *data)
/*[clinic end generated code: output=489f0a43ef4c99ec input=2377fa8de58bde66]*/
// clang-format on
{
    return CompiledFormat_unpack_from_impl(self, data, 0);
}

// clang-format off
/*[clinic input]
CompiledFormat.unpack_from

    data: Py_buffer
    offset: Py_ssize_t = 0

Return a tuple containing unpacked values starting at 'offset' bits.
[clinic start generated code]*/

static PyObject *
CompiledFormat_unpack_from_impl(PyCompiledFormatObject *self,
                                Py_buffer *data, Py_ssize_t offset)
/*[clinic end generated code: output=b9d1446fc0990bef input=9ea43549ede42f94]*/
// clang-format on
{
    ParsedElement elements_stack[SMALL_FORMAT_OPTIMIZATION];
    ParsedElement* elements = elements_stack;
    bool use_stack = self->compiled_fmt.ndescs <= SMALL_FORMAT_OPTIMIZATION;

    if (!PyBuffer_IsContiguous(data, 'C')) {
        PyErr_Format(PyExc_TypeError, "unpack() expects a contiguous buffer");
        return NULL;
    }

    int nbytes = (self->compiled_fmt.nbits + offset + 7) / 8;
    if (data->len < nbytes) {
        PyErr_Format(
            PyExc_TypeError,
            "unpack() requires a buffer of at least %d bytes",
            nbytes);
        return NULL;
    }

    if (!use_stack) {
        elements = PyMem_RawMalloc(
            self->compiled_fmt.ndescs * sizeof(ParsedElement));
        if (!elements) {
            PyErr_NoMemory();
            return NULL;
        }
    }

    c_unpack(elements, data->buf, self->compiled_fmt, offset);
    PyObject* return_value =
        parsed_elements_to_python(elements, self->compiled_fmt);

    if (!use_stack) {
        PyMem_RawFree(elements);
    }

    return return_value;
}

// clang-format off
static struct PyMethodDef CompiledFormat_methods[] = {
    {
        "pack",
        (PyCFunction)CompiledFormat_pack,
        METH_VARARGS,
        CompiledFormat_pack__doc__
    },
    {
        "pack_into",
        (PyCFunction)CompiledFormat_pack_into,
        METH_VARARGS|METH_KEYWORDS,
        CompiledFormat_pack_into__doc__
    },
    COMPILEDFORMAT_UNPACK_METHODDEF
    COMPILEDFORMAT_CALCSIZE_METHODDEF
    COMPILEDFORMAT_UNPACK_FROM_METHODDEF
    {NULL, NULL}
};

static PyTypeObject PyCompiledFormatType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "_cbitstruct.CompiledFormat",
    .tp_doc = CompiledFormat___init____doc__,
    .tp_basicsize = sizeof(PyCompiledFormatObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_init = (initproc) CompiledFormat___init__,
    .tp_new = CompiledFormat_new,
    .tp_dealloc = (destructor) CompiledFormat_dealloc,
    .tp_methods = CompiledFormat_methods,
};
// clang-format on

/* CompiledFormatDict */

// clang-format off
/*[clinic input]
CompiledFormatDict.__init__

    fmt: str
    names: object

Create a compiled bitstruct object.

Return a new CompiledFormatDict object which writes and reads binary data
according to the format string. The names list 'name' will be used
as keys in data dictionaries.
[clinic start generated code]*/

static int
CompiledFormatDict___init___impl(PyCompiledFormatDictObject *self,
                                 const char *fmt, PyObject *names)
/*[clinic end generated code: output=1d94d08e0ab9bdba input=9964c21a875025a8]*/
// clang-format on
{
    self->names = NULL;

    if (CompiledFormat___init___impl((PyCompiledFormatObject*)self, fmt)) {
        // CompiledFormat___init___impl has set the exception
        return -1;
    }

    self->names = PySequence_Fast(names, "names must be a sequence");
    if (!self->names) {
        // PySequence_Fast sets the exception
        return -1;
    }

    return 0;
}

static void CompiledFormatDict_deinit(PyCompiledFormatDictObject* self)
{
    if (self->names) {
        Py_DECREF(self->names);
    }
    CompiledFormat_deinit((PyCompiledFormatObject*)self);
}

static void CompiledFormatDict_dealloc(PyCompiledFormatDictObject* self)
{
    CompiledFormatDict_deinit(self);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

// clang-format off
/*[clinic input]
CompiledFormatDict.pack

    data: object

Pack values from a dict into a bytes object

Return a tuple containing unpacked values.
'data' is a dictionary containing values whic keys are the 'names'
used when constructing this object.
[clinic start generated code]*/

static PyObject *
CompiledFormatDict_pack_impl(PyCompiledFormatDictObject *self,
                             PyObject *data)
/*[clinic end generated code: output=8c26c6e4fc16088c input=afba13ca6085dd00]*/
// clang-format on
{
    PyObject* return_value = NULL;

    Py_ssize_t nnames = PySequence_Fast_GET_SIZE(self->names);
    PyObject** names = PySequence_Fast_ITEMS(self->names);
    PyObject* values = PyTuple_New(nnames);
    if (!values) {
        PyErr_NoMemory();
        return NULL;
    }

    for (int i = 0; i < nnames; ++i) {
        PyObject* v = PyObject_GetItem(data, names[i]);
        if (!v) {
            // PyObject_GetItem sets KeyError
            goto exit;
        }
        PyTuple_SET_ITEM(values, i, v);
    }

    return_value = CompiledFormat_pack((PyCompiledFormatObject*)self, values);

exit:
    if (values) {
        Py_DECREF(values);
    }

    return return_value;
}

// clang-format off
/*[clinic input]
CompiledFormatDict.pack_into

    buf: Py_buffer
    offset: Py_ssize_t
    data: object
    *
    fill_padding: bool = True

Pack data into a bytes object, starting at bit offset given by the offset argument.

With fill_padding=False, passing bits in 'buf' will not be modified.
[clinic start generated code]*/

static PyObject *
CompiledFormatDict_pack_into_impl(PyCompiledFormatDictObject *self,
                                  Py_buffer *buf, Py_ssize_t offset,
                                  PyObject *data, int fill_padding)
/*[clinic end generated code: output=ee246de261e9c699 input=290a9a4a3e3ed942]*/
// clang-format on
{
    assert(PyTuple_Check(args));

    PyObject* return_value = NULL;

    Py_ssize_t nnames = PySequence_Fast_GET_SIZE(self->names);
    PyObject** names = PySequence_Fast_ITEMS(self->names);

    PyObject* data_tuple = PyTuple_New(nnames);
    if (!data_tuple) {
        PyErr_NoMemory();
        goto exit;
    }

    for (int i = 0; i < nnames; ++i) {
        PyObject* v = PyObject_GetItem(data, names[i]);
        if (!v) {
            // PyObject_GetItem sets KeyError
            goto exit;
        }
        PyTuple_SET_ITEM(data_tuple, i, v);
    }

    PyObject** data_array = PySequence_Fast_ITEMS(data_tuple);
    return_value = CompiledFormat_pack_into_raw(
        self->super.compiled_fmt, buf, offset, data_array, nnames, fill_padding);

exit:
    if (data_tuple) {
        Py_DECREF(data_tuple);
    }

    return return_value;
}

// clang-format off
/*[clinic input]
CompiledFormatDict.unpack

    data: Py_buffer

Unpack data into a dict which keys are the 'names' used when constructing this object.

Return a dict containing unpacked values.
[clinic start generated code]*/

static PyObject *
CompiledFormatDict_unpack_impl(PyCompiledFormatDictObject *self,
                               Py_buffer *data)
/*[clinic end generated code: output=647eec29c90d0c63 input=40d648e22f9a7249]*/
// clang-format on
{
    return CompiledFormatDict_unpack_from_impl(self, data, 0);
}

// clang-format off
/*[clinic input]
CompiledFormatDict.unpack_from

    data: Py_buffer
    offset: Py_ssize_t = 0

Unpack data into a dict starting at 'offset' bits.

Return a dict containing unpacked values.
[clinic start generated code]*/

static PyObject *
CompiledFormatDict_unpack_from_impl(PyCompiledFormatDictObject *self,
                                    Py_buffer *data, Py_ssize_t offset)
/*[clinic end generated code: output=5f5d4987c9da42fe input=f7e4af99e1650077]*/
// clang-format on
{
    PyObject* return_value = NULL;
    PyObject* tuple_res = NULL;
    PyObject* tuple_res_seq = NULL;

    Py_ssize_t len = PySequence_Fast_GET_SIZE(self->names);
    tuple_res = CompiledFormat_unpack_from_impl(
        (PyCompiledFormatObject*)self, data, offset);
    if (!tuple_res) {
        // We expect CompiledFormat_unpack_impl to set the exception
        goto exit;
    }

    tuple_res_seq = PySequence_Fast(tuple_res, "");
    if (!tuple_res_seq) {
        // PySequence_Fast sets the exception
        goto exit;
    }

    if (PySequence_Fast_GET_SIZE(tuple_res_seq) != len) {
        PyErr_Format(
            PyExc_TypeError,
            "unpacked %d values, but have %d names",
            PySequence_Fast_GET_SIZE(tuple_res_seq),
            len);
        goto exit;
    }

    return_value = PyDict_New();
    if (!return_value) {
        PyErr_NoMemory();
        goto exit;
    }

    PyObject** names = PySequence_Fast_ITEMS(self->names);
    PyObject** values = PySequence_Fast_ITEMS(tuple_res_seq);
    for (int i = 0; i < len; ++i) {
        if (PyDict_SetItem(return_value, names[i], values[i])) {
            // PyDict_SetItem sets TypeError
            goto exit;
        }
    }

exit:
    if (tuple_res_seq) {
        Py_DECREF(tuple_res_seq);
    }
    if (tuple_res) {
        Py_DECREF(tuple_res);
    }

    return return_value;
}

// clang-format off
static struct PyMethodDef CompiledFormatDict_methods[] = {
    COMPILEDFORMATDICT_PACK_METHODDEF
    COMPILEDFORMATDICT_UNPACK_METHODDEF
    COMPILEDFORMATDICT_UNPACK_FROM_METHODDEF
    COMPILEDFORMATDICT_PACK_INTO_METHODDEF
    {NULL, NULL}
};

static PyTypeObject PyCompiledFormatDictType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "_cbitstruct.CompiledFormatDict",
    .tp_doc = CompiledFormatDict___init____doc__,
    .tp_basicsize = sizeof(PyCompiledFormatDictObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_base = &PyCompiledFormatType,
    .tp_init = (initproc) CompiledFormatDict___init__,
    .tp_dealloc = (destructor) CompiledFormatDict_dealloc,
    .tp_methods = CompiledFormatDict_methods,
};
// clang-format on

/* Functions */

// clang-format off
PyDoc_STRVAR(pack__doc__,
"pack(fmt, *args)\n"
"--\n"
"\n"
"Pack args into a bytes object according to fmt");
// clang-format on
static PyObject* pack(PyObject* module, PyObject* args, PyObject* kwargs)
{
    PyObject* return_value = NULL;
    const char* fmt = NULL;
    PyCompiledFormatObject self;
    memset(&self, 0, sizeof(self));

    static char* _keywords[] = {"fmt", NULL};
    Py_ssize_t n_args_parsed = PyArg_ParseTupleAndKeywordsFirstN(
        args, kwargs, "s:pack", _keywords, 1, &fmt);
    if (n_args_parsed < 0) {
        goto exit;
    }

    if (CompiledFormat___init___impl(&self, fmt)) {
        // CompiledFormat___init___impl has set the exception
        goto exit;
    }

    Py_ssize_t n_args = PyTuple_GET_SIZE(args);
    PyObject** data = PySequence_Fast_ITEMS(args);
    return_value = CompiledFormat_pack_raw(
        self.compiled_fmt, data + n_args_parsed, n_args - n_args_parsed);

exit:
    CompiledFormat_deinit(&self);
    return return_value;
}

// clang-format off
PyDoc_STRVAR(pack_into__doc__,
"pack_into(fmt, buf, offset, *args, **kwargs)\n"
"--\n"
"\n"
"Pack args into a bytes object according to fmt, starting at bit offset given by the\n"
"offset argument. An optional 'fill_padding=False' argument can be given\n"
"to keep padding bits from 'buf' as-is.");
// clang-format on
static PyObject* pack_into(PyObject* module, PyObject* args, PyObject* kwargs)
{
    PyObject* return_value = NULL;
    Py_buffer buffer = {NULL, NULL};
    Py_ssize_t offset = 0;
    const char* fmt = NULL;
    PyCompiledFormatObject self;
    memset(&self, 0, sizeof(self));
    bool fill_padding = PopFillPadding(kwargs);

    static char* _keywords[] = {"fmt", "buf", "offset", NULL};
    Py_ssize_t n_args_parsed = PyArg_ParseTupleAndKeywordsFirstN(
        args, kwargs, "sy*n:pack_into", _keywords, 3, &fmt, &buffer, &offset);
    if (n_args_parsed < 0) {
        goto exit;
    }

    if (CompiledFormat___init___impl(&self, fmt)) {
        // CompiledFormat___init___impl has set the exception
        goto exit;
    }

    Py_ssize_t n_args = PyTuple_GET_SIZE(args);
    PyObject** data = PySequence_Fast_ITEMS(args);
    return_value = CompiledFormat_pack_into_raw(
        self.compiled_fmt,
        &buffer,
        offset,
        data + n_args_parsed,
        n_args - n_args_parsed,
        fill_padding);

exit:
    CompiledFormat_deinit(&self);
    return return_value;
}

// clang-format off
/*[clinic input]
pack_dict

    fmt: str
    names: object
    data: object


Pack the dict data into a bytes object according to format.

The order of value is determines by the list 'names'.
[clinic start generated code]*/

static PyObject *
pack_dict_impl(PyObject *module, const char *fmt, PyObject *names,
               PyObject *data)
/*[clinic end generated code: output=b44920272f0e4e79 input=3222bbb02f49b7e9]*/
// clang-format on
{
    PyObject* return_value = NULL;
    PyCompiledFormatDictObject self;
    memset(&self, 0, sizeof(self));

    if (CompiledFormatDict___init___impl(&self, fmt, names)) {
        // CompiledFormatDict___init___impl has set the exception
        goto exit;
    }
    return_value = CompiledFormatDict_pack_impl(&self, data);

exit:
    CompiledFormatDict_deinit(&self);
    return return_value;
}

// clang-format off
/*[clinic input]
pack_into_dict

    fmt: str
    names: object
    buf: Py_buffer
    offset: Py_ssize_t
    data: object
    *
    fill_padding: bool = True

Pack data into a bytes object, starting at bit offset given by the offset argument.

With fill_padding=False, passing bits in 'buf' will not be modified.
[clinic start generated code]*/

static PyObject *
pack_into_dict_impl(PyObject *module, const char *fmt, PyObject *names,
                    Py_buffer *buf, Py_ssize_t offset, PyObject *data,
                    int fill_padding)
/*[clinic end generated code: output=619b415fc187011b input=e72dec46484ec66f]*/
// clang-format on
{
    assert(PyTuple_Check(args));

    PyObject* return_value = NULL;
    PyCompiledFormatDictObject self;
    memset(&self, 0, sizeof(self));

    if (CompiledFormatDict___init___impl(&self, fmt, names)) {
        // CompiledFormatDict___init___impl has set the exception
        goto exit;
    }

    return_value =
        CompiledFormatDict_pack_into_impl(&self, buf, offset, data, fill_padding);

exit:
    CompiledFormatDict_deinit(&self);
    return return_value;
}

// clang-format off
/*[clinic input]
unpack

    fmt: str
    data: Py_buffer


Unpack data according to the format 'fmt'. Returns a tuple.
[clinic start generated code]*/

static PyObject *
unpack_impl(PyObject *module, const char *fmt, Py_buffer *data)
/*[clinic end generated code: output=a143faad32f38aba input=50b83d898a3818c2]*/
// clang-format on
{
    return unpack_from_impl(module, fmt, data, 0);
}

// clang-format off
/*[clinic input]
unpack_from

    fmt: str
    data: Py_buffer
    offset: Py_ssize_t = 0


Unpack data according to the format 'fmt', starting at bit offset 'offset.

Returns a tuple.
[clinic start generated code]*/

static PyObject *
unpack_from_impl(PyObject *module, const char *fmt, Py_buffer *data,
                 Py_ssize_t offset)
/*[clinic end generated code: output=ae5caee332fca9c6 input=0b5f695460c5fc88]*/
// clang-format on
{
    PyObject* return_value = NULL;
    PyCompiledFormatObject self;
    memset(&self, 0, sizeof(self));

    if (CompiledFormat___init___impl(&self, fmt)) {
        // CompiledFormat___init___impl has set the exception
        goto exit;
    }
    return_value = CompiledFormat_unpack_from_impl(&self, data, offset);

exit:
    CompiledFormat_deinit(&self);
    return return_value;
}

// clang-format off
/*[clinic input]
unpack_dict

    fmt: str
    names: object
    data: Py_buffer

Unpack data according to 'fmt'.

Returns a dict which keys are 'names'.
[clinic start generated code]*/

static PyObject *
unpack_dict_impl(PyObject *module, const char *fmt, PyObject *names,
                 Py_buffer *data)
/*[clinic end generated code: output=5ab92004f3aeb22e input=2fc9ff0adb4d3763]*/
// clang-format on
{
    return unpack_from_dict_impl(module, fmt, names, data, 0);
}

// clang-format off
/*[clinic input]
unpack_from_dict

    fmt: str
    names: object
    data: Py_buffer
    offset: Py_ssize_t = 0

Unpack data according to 'fmt' starting at bit offset 'offset'.

Returns a dict which keys are 'names'.
[clinic start generated code]*/

static PyObject *
unpack_from_dict_impl(PyObject *module, const char *fmt, PyObject *names,
                      Py_buffer *data, Py_ssize_t offset)
/*[clinic end generated code: output=047df6a3b1645e39 input=a8a1a55238a5174c]*/
// clang-format on
{
    PyObject* return_value = NULL;
    PyCompiledFormatDictObject self;
    memset(&self, 0, sizeof(self));

    if (CompiledFormatDict___init___impl(&self, fmt, names)) {
        // CompiledFormatDict___init___impl has set the exception
        goto exit;
    }
    return_value = CompiledFormatDict_unpack_from_impl(&self, data, offset);

exit:
    CompiledFormatDict_deinit(&self);
    return return_value;
}

// clang-format off
/*[clinic input]
compile

    fmt: str
    names: object = None

Returns a compiled object for the format 'fmt'.
[clinic start generated code]*/

static PyObject *
compile_impl(PyObject *module, const char *fmt, PyObject *names)
/*[clinic end generated code: output=cd39debf9cc766ce input=fbc275638eafb2e9]*/
// clang-format on
{
    PyObject *args = NULL, *type = NULL;
    if (names == Py_None) {
        args = Py_BuildValue("(s)", fmt);
        type = (PyObject*)&PyCompiledFormatType;
    }
    else {
        args = Py_BuildValue("(sO)", fmt, names);
        type = (PyObject*)&PyCompiledFormatDictType;
    }
    PyObject* result = PyObject_CallObject(type, args);
    Py_DECREF(args);
    return result;
}

// clang-format off
/*[clinic input]
calcsize -> Py_ssize_t

    fmt: str

Return size in bits of the bit described by the format string.
[clinic start generated code]*/

static Py_ssize_t
calcsize_impl(PyObject *module, const char *fmt)
/*[clinic end generated code: output=3eb5ae42af038a45 input=b0a27d37ccb4dd03]*/
// clang-format on
{
    Py_ssize_t return_value = -1;
    PyCompiledFormatObject self;
    memset(&self, 0, sizeof(self));

    if (CompiledFormat___init___impl(&self, fmt)) {
        // CompiledFormat___init___impl has set the exception
        goto exit;
    }
    return_value = CompiledFormat_calcsize_impl(&self);

exit:
    CompiledFormat_deinit(&self);
    return return_value;
}

// clang-format off
/*[clinic input]
byteswap

    fmt: object
    data: Py_buffer
    offset: Py_ssize_t = 0


Swap bytes in `data` according to `fmt`, starting at byte `offset` and return the result.

`fmt` must be an iterable, iterating over
number of bytes to swap. For example, the format string ``'24'``
applied to the bytes ``b'\x00\x11\x22\x33\x44\x55'`` will
produce the result ``b'\x11\x00\x55\x44\x33\x22'``.
[clinic start generated code]*/

static PyObject *
byteswap_impl(PyObject *module, PyObject *fmt, Py_buffer *data,
              Py_ssize_t offset)
/*[clinic end generated code: output=3a2ad2de3d5d61ab input=54efc0a1db975ba3]*/
// clang-format on
{
    PyObject* return_value = NULL;
    int* count_iter = NULL;
    Py_ssize_t length = -1;

    if (!PyBuffer_IsContiguous(data, 'C')) {
        PyErr_Format(PyExc_TypeError, "byteswap() expects a contiguous buffer");
        goto exit;
    }

    length = PyObject_Length(fmt);
    if (length < 0) {
        goto exit;
    }

    return_value = PyBytes_FromStringAndSize(
        ((const char*)data->buf) + offset, data->len - offset);
    if (!return_value) {
        PyErr_NoMemory();
        goto exit;
    }

    count_iter = PyMem_RawMalloc(length * sizeof(int));
    if (!count_iter) {
        PyErr_NoMemory();
        goto exit;
    }

    int sum = 0;
    for (int i = 0; i < length; ++i) {
        PyObject* item = PySequence_GetItem(fmt, i);
        if (!item) {
            goto exit;
        }

        long len = -1;
        if (PyUnicode_Check(item)) {
            PyObject* pylong = PyLong_FromUnicodeObject(item, 10);
            len = PyLong_AsLong(pylong);
            Py_DECREF(pylong);
        }
        else {
            len = PyLong_AsLong(item);
        }

        sum += len;
        count_iter[i] = len;
        Py_DECREF(item);
        if (len < 0 || PyErr_Occurred()) {
            goto exit;
        }
    }

    if (sum > PyBytes_Size(return_value)) {
        PyErr_Format(
            PyExc_TypeError,
            "byteswap() requires a buffer of at least %d bytes",
            sum);
        goto exit;
    }

    uint8_t* buf = (uint8_t*)PyBytes_AS_STRING(return_value);
    for (int i = 0; i < length; ++i) {
        int nbytes = count_iter[i];
        c_byteswitch(buf, nbytes);
        buf += nbytes;
    }

exit:
    if (count_iter) {
        PyMem_RawFree(count_iter);
    }

    return return_value;
}

// clang-format off
static struct PyMethodDef py_module_functions[] = {
    {
        "pack",
        (PyCFunction)pack,
        METH_VARARGS|METH_KEYWORDS,
        pack__doc__
    },
    {
        "pack_into",
        (PyCFunction)pack_into,
        METH_VARARGS|METH_KEYWORDS,
        pack_into__doc__
    },
    PACK_DICT_METHODDEF
    PACK_INTO_DICT_METHODDEF
    UNPACK_METHODDEF
    UNPACK_FROM_METHODDEF
    UNPACK_DICT_METHODDEF
    UNPACK_FROM_DICT_METHODDEF
    COMPILE_METHODDEF
    CALCSIZE_METHODDEF
    BYTESWAP_METHODDEF
    {NULL, NULL, 0, NULL}, /* sentinel */
};
// clang-format on

/* Module initialization */

// clang-format off
PyDoc_STRVAR(cbitstructmodule__doc__,
"A fast implementation of the bitstruct.\n"
"\n"
"Check https://bitstruct.readthedocs.io/en/latest/ for documentation.");
// clang-format on
static struct PyModuleDef _cbitstructmodule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "_cbitstruct",
    .m_doc = cbitstructmodule__doc__,
    .m_methods = py_module_functions,
    .m_size = -1,
};

PyMODINIT_FUNC PyInit__cbitstruct(void)
{
    PyObject* m;

    if (PyType_Ready(&PyCompiledFormatType) < 0) {
        return NULL;
    }

    if (PyType_Ready(&PyCompiledFormatDictType) < 0) {
        return NULL;
    }

    m = PyModule_Create(&_cbitstructmodule);
    if (!m) {
        return NULL;
    }

    Py_INCREF(&PyCompiledFormatType);
    Py_INCREF(&PyCompiledFormatDictType);

    PyModule_AddObject(m, "CompiledFormat", (PyObject*)&PyCompiledFormatType);
    PyModule_AddObject(m, "CompiledFormatDict", (PyObject*)&PyCompiledFormatDictType);

    return m;
}
