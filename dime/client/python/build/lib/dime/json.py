import base64
import json

import numpy as np

__all__ = ["loads", "dumps"]

class DimeJSONEncoder(json.JSONEncoder):
    def default(self, obj):
        if isinstance(obj, complex):
            return {
                "__dime_type": "complex",
                "real": obj.real,
                "imag": obj.imag
            }

        if isinstance(obj, np.ndarray):
            dtype = str(obj.dtype)

            if dtype not in {"int8", "int16", "int32", "int64",
                             "uint8", "uint16", "uint32", "uint64",
                             "float32", "float64", "complex64", "complex128"}:
                raise TypeError()

            data = base64.b64encode(np.asfortranarray(obj).tobytes()).decode('ascii')
            shape = list(obj.shape)

            return {
                "__dime_type": "matrix",
                "dtype": dtype,
                "shape": shape,
                "data": data
            }

        return super().default(obj)

def dime_JSON_dechook(dct):
    if "__dime_type" in dct:
        if dct["__dime_type"] == "complex":
            return complex(dct["real"], dct["imag"])

        if dct["__dime_type"] == "matrix":
            return np.frombuffer(base64.b64decode(dct["data"].encode('ascii')), dtype = np.dtype(dct["dtype"])).reshape(dct["shape"], order = "F")

    return dct

def loads(x):
    return json.loads(x.decode("utf-8"), object_hook = dime_JSON_dechook)

def dumps(x):
    return json.dumps(x, cls = DimeJSONEncoder).encode("utf-8")
