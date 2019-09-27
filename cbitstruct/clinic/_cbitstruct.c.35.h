/*[clinic input]
preserve
[clinic start generated code]*/

PyDoc_STRVAR(CompiledFormat___init____doc__,
"CompiledFormat(fmt)\n"
"--\n"
"\n"
"Create a compiled bitstruct object.\n"
"\n"
"Return a new CompiledFormat object which writes and reads binary data\n"
"according to the format string.");

static int
CompiledFormat___init___impl(PyCompiledFormatObject *self, const char *fmt);

static int
CompiledFormat___init__(PyObject *self, PyObject *args, PyObject *kwargs)
{
    int return_value = -1;
    static char *_keywords[] = {"fmt", NULL};
    const char *fmt;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s:CompiledFormat", _keywords,
        &fmt))
        goto exit;
    return_value = CompiledFormat___init___impl((PyCompiledFormatObject *)self, fmt);

exit:
    return return_value;
}

PyDoc_STRVAR(CompiledFormat_calcsize__doc__,
"calcsize($self, /)\n"
"--\n"
"\n"
"Return size in bits of the bitDescribed by the format string.");

#define COMPILEDFORMAT_CALCSIZE_METHODDEF    \
    {"calcsize", (PyCFunction)CompiledFormat_calcsize, METH_NOARGS, CompiledFormat_calcsize__doc__},

static Py_ssize_t
CompiledFormat_calcsize_impl(PyCompiledFormatObject *self);

static PyObject *
CompiledFormat_calcsize(PyCompiledFormatObject *self, PyObject *Py_UNUSED(ignored))
{
    PyObject *return_value = NULL;
    Py_ssize_t _return_value;

    _return_value = CompiledFormat_calcsize_impl(self);
    if ((_return_value == -1) && PyErr_Occurred())
        goto exit;
    return_value = PyLong_FromSsize_t(_return_value);

exit:
    return return_value;
}

PyDoc_STRVAR(CompiledFormat_unpack__doc__,
"unpack($self, /, data)\n"
"--\n"
"\n"
"Return a tuple containing unpacked values.");

#define COMPILEDFORMAT_UNPACK_METHODDEF    \
    {"unpack", (PyCFunction)CompiledFormat_unpack, METH_VARARGS|METH_KEYWORDS, CompiledFormat_unpack__doc__},

static PyObject *
CompiledFormat_unpack_impl(PyCompiledFormatObject *self, Py_buffer *data);

static PyObject *
CompiledFormat_unpack(PyCompiledFormatObject *self, PyObject *args, PyObject *kwargs)
{
    PyObject *return_value = NULL;
    static char *_keywords[] = {"data", NULL};
    Py_buffer data = {NULL, NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "y*:unpack", _keywords,
        &data))
        goto exit;
    return_value = CompiledFormat_unpack_impl(self, &data);

exit:
    /* Cleanup for data */
    if (data.obj)
       PyBuffer_Release(&data);

    return return_value;
}

PyDoc_STRVAR(CompiledFormat_unpack_from__doc__,
"unpack_from($self, /, data, offset=0)\n"
"--\n"
"\n"
"Return a tuple containing unpacked values starting at \'offset\' bits.");

#define COMPILEDFORMAT_UNPACK_FROM_METHODDEF    \
    {"unpack_from", (PyCFunction)CompiledFormat_unpack_from, METH_VARARGS|METH_KEYWORDS, CompiledFormat_unpack_from__doc__},

static PyObject *
CompiledFormat_unpack_from_impl(PyCompiledFormatObject *self,
                                Py_buffer *data, Py_ssize_t offset);

static PyObject *
CompiledFormat_unpack_from(PyCompiledFormatObject *self, PyObject *args, PyObject *kwargs)
{
    PyObject *return_value = NULL;
    static char *_keywords[] = {"data", "offset", NULL};
    Py_buffer data = {NULL, NULL};
    Py_ssize_t offset = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "y*|n:unpack_from", _keywords,
        &data, &offset))
        goto exit;
    return_value = CompiledFormat_unpack_from_impl(self, &data, offset);

exit:
    /* Cleanup for data */
    if (data.obj)
       PyBuffer_Release(&data);

    return return_value;
}

PyDoc_STRVAR(CompiledFormatDict___init____doc__,
"CompiledFormatDict(fmt, names)\n"
"--\n"
"\n"
"Create a compiled bitstruct object.\n"
"\n"
"Return a new CompiledFormatDict object which writes and reads binary data\n"
"according to the format string. The names list \'name\' will be used\n"
"as keys in data dictionaries.");

static int
CompiledFormatDict___init___impl(PyCompiledFormatDictObject *self,
                                 const char *fmt, PyObject *names);

static int
CompiledFormatDict___init__(PyObject *self, PyObject *args, PyObject *kwargs)
{
    int return_value = -1;
    static char *_keywords[] = {"fmt", "names", NULL};
    const char *fmt;
    PyObject *names;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "sO:CompiledFormatDict", _keywords,
        &fmt, &names))
        goto exit;
    return_value = CompiledFormatDict___init___impl((PyCompiledFormatDictObject *)self, fmt, names);

exit:
    return return_value;
}

PyDoc_STRVAR(CompiledFormatDict_pack__doc__,
"pack($self, /, data)\n"
"--\n"
"\n"
"Pack values from a dict into a bytes object\n"
"\n"
"Return a tuple containing unpacked values.\n"
"\'data\' is a dictionary containing values whic keys are the \'names\'\n"
"used when constructing this object.");

#define COMPILEDFORMATDICT_PACK_METHODDEF    \
    {"pack", (PyCFunction)CompiledFormatDict_pack, METH_VARARGS|METH_KEYWORDS, CompiledFormatDict_pack__doc__},

static PyObject *
CompiledFormatDict_pack_impl(PyCompiledFormatDictObject *self,
                             PyObject *data);

static PyObject *
CompiledFormatDict_pack(PyCompiledFormatDictObject *self, PyObject *args, PyObject *kwargs)
{
    PyObject *return_value = NULL;
    static char *_keywords[] = {"data", NULL};
    PyObject *data;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:pack", _keywords,
        &data))
        goto exit;
    return_value = CompiledFormatDict_pack_impl(self, data);

exit:
    return return_value;
}

PyDoc_STRVAR(CompiledFormatDict_pack_into__doc__,
"pack_into($self, /, buf, offset, data, *, fill_padding=True)\n"
"--\n"
"\n"
"Pack data into a bytes object, starting at bit offset given by the offset argument.\n"
"\n"
"With fill_padding=False, passing bits in \'buf\' will not be modified.");

#define COMPILEDFORMATDICT_PACK_INTO_METHODDEF    \
    {"pack_into", (PyCFunction)CompiledFormatDict_pack_into, METH_VARARGS|METH_KEYWORDS, CompiledFormatDict_pack_into__doc__},

static PyObject *
CompiledFormatDict_pack_into_impl(PyCompiledFormatDictObject *self,
                                  Py_buffer *buf, Py_ssize_t offset,
                                  PyObject *data, int fill_padding);

static PyObject *
CompiledFormatDict_pack_into(PyCompiledFormatDictObject *self, PyObject *args, PyObject *kwargs)
{
    PyObject *return_value = NULL;
    static char *_keywords[] = {"buf", "offset", "data", "fill_padding", NULL};
    Py_buffer buf = {NULL, NULL};
    Py_ssize_t offset;
    PyObject *data;
    int fill_padding = 1;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "y*nO|$p:pack_into", _keywords,
        &buf, &offset, &data, &fill_padding))
        goto exit;
    return_value = CompiledFormatDict_pack_into_impl(self, &buf, offset, data, fill_padding);

exit:
    /* Cleanup for buf */
    if (buf.obj)
       PyBuffer_Release(&buf);

    return return_value;
}

PyDoc_STRVAR(CompiledFormatDict_unpack__doc__,
"unpack($self, /, data)\n"
"--\n"
"\n"
"Unpack data into a dict which keys are the \'names\' used when constructing this object.\n"
"\n"
"Return a dict containing unpacked values.");

#define COMPILEDFORMATDICT_UNPACK_METHODDEF    \
    {"unpack", (PyCFunction)CompiledFormatDict_unpack, METH_VARARGS|METH_KEYWORDS, CompiledFormatDict_unpack__doc__},

static PyObject *
CompiledFormatDict_unpack_impl(PyCompiledFormatDictObject *self,
                               Py_buffer *data);

static PyObject *
CompiledFormatDict_unpack(PyCompiledFormatDictObject *self, PyObject *args, PyObject *kwargs)
{
    PyObject *return_value = NULL;
    static char *_keywords[] = {"data", NULL};
    Py_buffer data = {NULL, NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "y*:unpack", _keywords,
        &data))
        goto exit;
    return_value = CompiledFormatDict_unpack_impl(self, &data);

exit:
    /* Cleanup for data */
    if (data.obj)
       PyBuffer_Release(&data);

    return return_value;
}

PyDoc_STRVAR(CompiledFormatDict_unpack_from__doc__,
"unpack_from($self, /, data, offset=0)\n"
"--\n"
"\n"
"Unpack data into a dict starting at \'offset\' bits.\n"
"\n"
"Return a dict containing unpacked values.");

#define COMPILEDFORMATDICT_UNPACK_FROM_METHODDEF    \
    {"unpack_from", (PyCFunction)CompiledFormatDict_unpack_from, METH_VARARGS|METH_KEYWORDS, CompiledFormatDict_unpack_from__doc__},

static PyObject *
CompiledFormatDict_unpack_from_impl(PyCompiledFormatDictObject *self,
                                    Py_buffer *data, Py_ssize_t offset);

static PyObject *
CompiledFormatDict_unpack_from(PyCompiledFormatDictObject *self, PyObject *args, PyObject *kwargs)
{
    PyObject *return_value = NULL;
    static char *_keywords[] = {"data", "offset", NULL};
    Py_buffer data = {NULL, NULL};
    Py_ssize_t offset = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "y*|n:unpack_from", _keywords,
        &data, &offset))
        goto exit;
    return_value = CompiledFormatDict_unpack_from_impl(self, &data, offset);

exit:
    /* Cleanup for data */
    if (data.obj)
       PyBuffer_Release(&data);

    return return_value;
}

PyDoc_STRVAR(pack_dict__doc__,
"pack_dict($module, /, fmt, names, data)\n"
"--\n"
"\n"
"Pack the dict data into a bytes object according to format.\n"
"\n"
"The order of value is determines by the list \'names\'.");

#define PACK_DICT_METHODDEF    \
    {"pack_dict", (PyCFunction)pack_dict, METH_VARARGS|METH_KEYWORDS, pack_dict__doc__},

static PyObject *
pack_dict_impl(PyObject *module, const char *fmt, PyObject *names,
               PyObject *data);

static PyObject *
pack_dict(PyObject *module, PyObject *args, PyObject *kwargs)
{
    PyObject *return_value = NULL;
    static char *_keywords[] = {"fmt", "names", "data", NULL};
    const char *fmt;
    PyObject *names;
    PyObject *data;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "sOO:pack_dict", _keywords,
        &fmt, &names, &data))
        goto exit;
    return_value = pack_dict_impl(module, fmt, names, data);

exit:
    return return_value;
}

PyDoc_STRVAR(pack_into_dict__doc__,
"pack_into_dict($module, /, fmt, names, buf, offset, data, *,\n"
"               fill_padding=True)\n"
"--\n"
"\n"
"Pack data into a bytes object, starting at bit offset given by the offset argument.\n"
"\n"
"With fill_padding=False, passing bits in \'buf\' will not be modified.");

#define PACK_INTO_DICT_METHODDEF    \
    {"pack_into_dict", (PyCFunction)pack_into_dict, METH_VARARGS|METH_KEYWORDS, pack_into_dict__doc__},

static PyObject *
pack_into_dict_impl(PyObject *module, const char *fmt, PyObject *names,
                    Py_buffer *buf, Py_ssize_t offset, PyObject *data,
                    int fill_padding);

static PyObject *
pack_into_dict(PyObject *module, PyObject *args, PyObject *kwargs)
{
    PyObject *return_value = NULL;
    static char *_keywords[] = {"fmt", "names", "buf", "offset", "data", "fill_padding", NULL};
    const char *fmt;
    PyObject *names;
    Py_buffer buf = {NULL, NULL};
    Py_ssize_t offset;
    PyObject *data;
    int fill_padding = 1;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "sOy*nO|$p:pack_into_dict", _keywords,
        &fmt, &names, &buf, &offset, &data, &fill_padding))
        goto exit;
    return_value = pack_into_dict_impl(module, fmt, names, &buf, offset, data, fill_padding);

exit:
    /* Cleanup for buf */
    if (buf.obj)
       PyBuffer_Release(&buf);

    return return_value;
}

PyDoc_STRVAR(unpack__doc__,
"unpack($module, /, fmt, data)\n"
"--\n"
"\n"
"Unpack data according to the format \'fmt\'. Returns a tuple.");

#define UNPACK_METHODDEF    \
    {"unpack", (PyCFunction)unpack, METH_VARARGS|METH_KEYWORDS, unpack__doc__},

static PyObject *
unpack_impl(PyObject *module, const char *fmt, Py_buffer *data);

static PyObject *
unpack(PyObject *module, PyObject *args, PyObject *kwargs)
{
    PyObject *return_value = NULL;
    static char *_keywords[] = {"fmt", "data", NULL};
    const char *fmt;
    Py_buffer data = {NULL, NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "sy*:unpack", _keywords,
        &fmt, &data))
        goto exit;
    return_value = unpack_impl(module, fmt, &data);

exit:
    /* Cleanup for data */
    if (data.obj)
       PyBuffer_Release(&data);

    return return_value;
}

PyDoc_STRVAR(unpack_from__doc__,
"unpack_from($module, /, fmt, data, offset=0)\n"
"--\n"
"\n"
"Unpack data according to the format \'fmt\', starting at bit offset \'offset.\n"
"\n"
"Returns a tuple.");

#define UNPACK_FROM_METHODDEF    \
    {"unpack_from", (PyCFunction)unpack_from, METH_VARARGS|METH_KEYWORDS, unpack_from__doc__},

static PyObject *
unpack_from_impl(PyObject *module, const char *fmt, Py_buffer *data,
                 Py_ssize_t offset);

static PyObject *
unpack_from(PyObject *module, PyObject *args, PyObject *kwargs)
{
    PyObject *return_value = NULL;
    static char *_keywords[] = {"fmt", "data", "offset", NULL};
    const char *fmt;
    Py_buffer data = {NULL, NULL};
    Py_ssize_t offset = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "sy*|n:unpack_from", _keywords,
        &fmt, &data, &offset))
        goto exit;
    return_value = unpack_from_impl(module, fmt, &data, offset);

exit:
    /* Cleanup for data */
    if (data.obj)
       PyBuffer_Release(&data);

    return return_value;
}

PyDoc_STRVAR(unpack_dict__doc__,
"unpack_dict($module, /, fmt, names, data)\n"
"--\n"
"\n"
"Unpack data according to \'fmt\'.\n"
"\n"
"Returns a dict which keys are \'names\'.");

#define UNPACK_DICT_METHODDEF    \
    {"unpack_dict", (PyCFunction)unpack_dict, METH_VARARGS|METH_KEYWORDS, unpack_dict__doc__},

static PyObject *
unpack_dict_impl(PyObject *module, const char *fmt, PyObject *names,
                 Py_buffer *data);

static PyObject *
unpack_dict(PyObject *module, PyObject *args, PyObject *kwargs)
{
    PyObject *return_value = NULL;
    static char *_keywords[] = {"fmt", "names", "data", NULL};
    const char *fmt;
    PyObject *names;
    Py_buffer data = {NULL, NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "sOy*:unpack_dict", _keywords,
        &fmt, &names, &data))
        goto exit;
    return_value = unpack_dict_impl(module, fmt, names, &data);

exit:
    /* Cleanup for data */
    if (data.obj)
       PyBuffer_Release(&data);

    return return_value;
}

PyDoc_STRVAR(unpack_from_dict__doc__,
"unpack_from_dict($module, /, fmt, names, data, offset=0)\n"
"--\n"
"\n"
"Unpack data according to \'fmt\' starting at bit offset \'offset\'.\n"
"\n"
"Returns a dict which keys are \'names\'.");

#define UNPACK_FROM_DICT_METHODDEF    \
    {"unpack_from_dict", (PyCFunction)unpack_from_dict, METH_VARARGS|METH_KEYWORDS, unpack_from_dict__doc__},

static PyObject *
unpack_from_dict_impl(PyObject *module, const char *fmt, PyObject *names,
                      Py_buffer *data, Py_ssize_t offset);

static PyObject *
unpack_from_dict(PyObject *module, PyObject *args, PyObject *kwargs)
{
    PyObject *return_value = NULL;
    static char *_keywords[] = {"fmt", "names", "data", "offset", NULL};
    const char *fmt;
    PyObject *names;
    Py_buffer data = {NULL, NULL};
    Py_ssize_t offset = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "sOy*|n:unpack_from_dict", _keywords,
        &fmt, &names, &data, &offset))
        goto exit;
    return_value = unpack_from_dict_impl(module, fmt, names, &data, offset);

exit:
    /* Cleanup for data */
    if (data.obj)
       PyBuffer_Release(&data);

    return return_value;
}

PyDoc_STRVAR(compile__doc__,
"compile($module, /, fmt, names=None)\n"
"--\n"
"\n"
"Returns a compiled object for the format \'fmt\'.");

#define COMPILE_METHODDEF    \
    {"compile", (PyCFunction)compile, METH_VARARGS|METH_KEYWORDS, compile__doc__},

static PyObject *
compile_impl(PyObject *module, const char *fmt, PyObject *names);

static PyObject *
compile(PyObject *module, PyObject *args, PyObject *kwargs)
{
    PyObject *return_value = NULL;
    static char *_keywords[] = {"fmt", "names", NULL};
    const char *fmt;
    PyObject *names = Py_None;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s|O:compile", _keywords,
        &fmt, &names))
        goto exit;
    return_value = compile_impl(module, fmt, names);

exit:
    return return_value;
}

PyDoc_STRVAR(calcsize__doc__,
"calcsize($module, /, fmt)\n"
"--\n"
"\n"
"Return size in bits of the bit described by the format string.");

#define CALCSIZE_METHODDEF    \
    {"calcsize", (PyCFunction)calcsize, METH_VARARGS|METH_KEYWORDS, calcsize__doc__},

static Py_ssize_t
calcsize_impl(PyObject *module, const char *fmt);

static PyObject *
calcsize(PyObject *module, PyObject *args, PyObject *kwargs)
{
    PyObject *return_value = NULL;
    static char *_keywords[] = {"fmt", NULL};
    const char *fmt;
    Py_ssize_t _return_value;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s:calcsize", _keywords,
        &fmt))
        goto exit;
    _return_value = calcsize_impl(module, fmt);
    if ((_return_value == -1) && PyErr_Occurred())
        goto exit;
    return_value = PyLong_FromSsize_t(_return_value);

exit:
    return return_value;
}

PyDoc_STRVAR(byteswap__doc__,
"byteswap($module, /, fmt, data, offset=0)\n"
"--\n"
"\n"
"Swap bytes in `data` according to `fmt`, starting at byte `offset` and return the result.\n"
"\n"
"`fmt` must be an iterable, iterating over\n"
"number of bytes to swap. For example, the format string ``\'24\'``\n"
"applied to the bytes ``b\'\\x00\\x11\\x22\\x33\\x44\\x55\'`` will\n"
"produce the result ``b\'\\x11\\x00\\x55\\x44\\x33\\x22\'``.");

#define BYTESWAP_METHODDEF    \
    {"byteswap", (PyCFunction)byteswap, METH_VARARGS|METH_KEYWORDS, byteswap__doc__},

static PyObject *
byteswap_impl(PyObject *module, PyObject *fmt, Py_buffer *data,
              Py_ssize_t offset);

static PyObject *
byteswap(PyObject *module, PyObject *args, PyObject *kwargs)
{
    PyObject *return_value = NULL;
    static char *_keywords[] = {"fmt", "data", "offset", NULL};
    PyObject *fmt;
    Py_buffer data = {NULL, NULL};
    Py_ssize_t offset = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Oy*|n:byteswap", _keywords,
        &fmt, &data, &offset))
        goto exit;
    return_value = byteswap_impl(module, fmt, &data, offset);

exit:
    /* Cleanup for data */
    if (data.obj)
       PyBuffer_Release(&data);

    return return_value;
}
/*[clinic end generated code: output=c102aaba2bc6b295 input=a9049054013a1b77]*/
