import collections.abc
import struct
import sys

import numpy as np

__all__ = ["loads", "dumps"]

# Boolean sentinels
TYPE_NULL  = 0x00
TYPE_TRUE  = 0x01
TYPE_FALSE = 0x02

# Scalar types
TYPE_I8  = 0x03
TYPE_I16 = 0x04
TYPE_I32 = 0x05
TYPE_I64 = 0x06
TYPE_U8  = 0x07
TYPE_U16 = 0x08
TYPE_U32 = 0x09
TYPE_U64 = 0x0A

TYPE_SINGLE         = 0x0B
TYPE_DOUBLE         = 0x0C
TYPE_COMPLEX_SINGLE = 0x0D
TYPE_COMPLEX_DOUBLE = 0x0E

# Matrix types
TYPE_MAT_I8  = 0x10 | TYPE_I8
TYPE_MAT_I16 = 0x10 | TYPE_I16
TYPE_MAT_I32 = 0x10 | TYPE_I32
TYPE_MAT_I64 = 0x10 | TYPE_I64
TYPE_MAT_U8  = 0x10 | TYPE_U8
TYPE_MAT_U16 = 0x10 | TYPE_U16
TYPE_MAT_U32 = 0x10 | TYPE_U32
TYPE_MAT_U64 = 0x10 | TYPE_U64

TYPE_MAT_SINGLE         = 0x10 | TYPE_SINGLE
TYPE_MAT_DOUBLE         = 0x10 | TYPE_DOUBLE
TYPE_MAT_COMPLEX_SINGLE = 0x10 | TYPE_COMPLEX_SINGLE
TYPE_MAT_COMPLEX_DOUBLE = 0x10 | TYPE_COMPLEX_DOUBLE

# Other types
TYPE_STRING     = 0x20
TYPE_ARRAY      = 0x21
TYPE_ASSOCARRAY = 0x22

def loads_none(s):
    return None, 1

def loads_true(s):
    return True, 1

def loads_false(s):
    return False, 1

def loads_i8(s):
    return struct.unpack("!b", s[1]), 2

def loads_i16(s):
    return struct.unpack("!h", s[1:3]), 3

def loads_i32(s):
    return struct.unpack("!i", s[1:5]), 5

def loads_i64(s):
    return struct.unpack("!q", s[1:9]), 9

def loads_u8(s):
    return struct.unpack("!B", s[1]), 2

def loads_u16(s):
    return struct.unpack("!H", s[1:3]), 3

def loads_u32(s):
    return struct.unpack("!I", s[1:5]), 5

def loads_u64(s):
    return struct.unpack("!Q", s[1:9]), 9

def loads_single(s):
    return struct.unpack("!f", s[1:5]), 5

def loads_double(s):
    return struct.unpack("!d", s[1:9]), 9

def loads_complex_single(s):
    real, imag = struct.unpack("!ff", s[1:9])
    return complex(real, imag), 9

def loads_complex_double(s):
    real, imag = struct.unpack("!dd", s[1:17])
    return complex(real, imag), 17

def loads_mat(s, dtype):
    rank = struct.unpack("!B", s[1])
    shape = struct.unpack("!" + "I" * rank, s[2:(4 * rank + 2)])

    count = dtype.itemsize * np.prod(rank)
    offset = 4 * rank + 2

    arr = np.frombuffer(s, dtype, count, offset).reshape(shape, "F")
    if sys.byteorder != "big":
        arr.byteswap(True)

    return arr, count + offset

def loads_mat_i8(s):
    return loads_mat(s, np.dtype(np.int8))

def loads_mat_i16(s):
    return loads_mat(s, np.dtype(np.int16))

def loads_mat_i32(s):
    return loads_mat(s, np.dtype(np.int32))

def loads_mat_i64(s):
    return loads_mat(s, np.dtype(np.int64))

def loads_mat_u8(s):
    return loads_mat(s, np.dtype(np.uint8))

def loads_mat_u16(s):
    return loads_mat(s, np.dtype(np.uint16))

def loads_mat_u32(s):
    return loads_mat(s, np.dtype(np.uint32))

def loads_mat_u64(s):
    return loads_mat(s, np.dtype(np.uint64))

def loads_mat_single(s):
    return loads_mat(s, np.dtype(np.float32))

def loads_mat_double(s):
    return loads_mat(s, np.dtype(np.float64))

def loads_mat_complex_single(s):
    return loads_mat(s, np.dtype(np.complex64))

def loads_mat_complex_double(s):
    return loads_mat(s, np.dtype(np.complex128))

def loads_string(s):
    siz = struct.unpack("!I", s[1:5])
    return s[5:(siz + 5)].decode("utf-8"), siz + 5

def loads_array(s):
    siz = struct.unpack("!I", s[1:5])

    ret = []
    i = 5

    for _ in range(siz):
        element, element_siz = _loads(s[i:])

        ret.append(element)
        i += element_siz

    return ret, i

def loads_assocarray(s):
    siz = struct.unpack("!I", s[1:5])

    ret = {}
    i = 5

    for _ in range(siz):
        key, key_siz = _loads(s[i:])
        i += key_siz

        val, val_siz = _loads(s[i:])
        i += val_siz

        ret[key] = val

    return ret, i

def _loads(s):
    tab = {
        TYPE_NULL:  loads_null,
        TYPE_TRUE:  loads_true,
        TYPE_FALSE: loads_false,

        TYPE_I8:  loads_i8,
        TYPE_I16: loads_i16,
        TYPE_I32: loads_i32,
        TYPE_I64: loads_i64,
        TYPE_U8:  loads_u8,
        TYPE_U16: loads_u16,
        TYPE_U32: loads_u32,
        TYPE_U64: loads_u64,

        TYPE_SINGLE:         loads_single,
        TYPE_DOUBLE:         loads_double,
        TYPE_COMPLEX_SINGLE: loads_complex_single,
        TYPE_COMPLEX_DOUBLE: loads_complex_double,

        TYPE_MAT_I8:  loads_mat_i8,
        TYPE_MAT_I16: loads_mat_i16,
        TYPE_MAT_I32: loads_mat_i32,
        TYPE_MAT_I64: loads_mat_i64,
        TYPE_MAT_U8:  loads_mat_u8,
        TYPE_MAT_U16: loads_mat_u16,
        TYPE_MAT_U32: loads_mat_u32,
        TYPE_MAT_U64: loads_mat_u64,

        TYPE_MAT_SINGLE:         loads_mat_single,
        TYPE_MAT_DOUBLE:         loads_mat_double,
        TYPE_MAT_COMPLEX_SINGLE: loads_mat_complex_single,
        TYPE_MAT_COMPLEX_DOUBLE: loads_mat_complex_double,

        TYPE_STRING:     loads_string,
        TYPE_ARRAY:      loads_array,
        TYPE_ASSOCARRAY: loads_assocarray
    }

    return tab[s[0]](s)

def loads(x):
    return _loads(x)[0]

def dumps(x):
    if x is None:
        return bytes([TYPE_NULL])

    elif x is True:
        return bytes([TYPE_TRUE])

    elif x is False:
        return bytes([TYPE_FALSE])

    elif isinstance(x, int):
        return struct.pack("!Bq", TYPE_I64, x)

    elif isinstance(x, float):
        return struct.pack("!Bd", TYPE_DOUBLE, x)

    elif isinstance(x, complex):
        return struct.pack("!Bdd", TYPE_COMPLEX_DOUBLE, x.real, x.imag)

    elif isinstance(x, np.ndarray):
        tab = {
            ("i", 1): TYPE_MAT_I8,
            ("i", 2): TYPE_MAT_I16,
            ("i", 4): TYPE_MAT_I32,
            ("i", 8): TYPE_MAT_I64,
            ("u", 1): TYPE_MAT_U8,
            ("u", 2): TYPE_MAT_U16,
            ("u", 4): TYPE_MAT_U32,
            ("u", 8): TYPE_MAT_U64,
            ("f", 4): TYPE_MAT_SINGLE,
            ("f", 8): TYPE_MAT_DOUBLE,
            ("c", 8): TYPE_MAT_COMPLEX_SINGLE,
            ("c", 16): TYPE_MAT_COMPLEX_DOUBLE
        }

        if not (x.dtype.byteorder == ">" or (x.dtype.byteorder == "=" and sys.byteorder == "big")):
            x = x.byteswap()

        shapedata = struct.pack("!BB" + "I" * len(x.shape), tab[(x.dtype.kind, x.dtype.itemsize)], len(x.shape), *x.shape)

        return shapedata + x.tobytes()

    elif isinstance(x, str):
        x = x.encode("utf-8")
        return struct.pack("!BI", TYPE_STRING, len(x)) + x

    elif isinstance(x, collections.abc.Sequence):
        elements = bytearray()

        for elem in x:
            elements += dumps(elem)

        return struct.pack("!BI", TYPE_ARRAY, len(x)) + bytes(elements)

    elif isinstance(x, collections.abc.Mapping):
        keyvalpairs = bytearray()

        for key, val in x.items():
            keyvalpairs += dumps(key)
            keyvalpairs += dumps(val)

        return struct.pack("!BI", TYPE_ASSOCARRAY, len(x)) + bytes(elements)

    else:
        raise TypeError
