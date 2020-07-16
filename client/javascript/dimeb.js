// Boolean sentinels
const TYPE_NULL  = 0x00;
const TYPE_TRUE  = 0x01;
const TYPE_FALSE = 0x02;

// Scalar types
const TYPE_I8  = 0x03;
const TYPE_I16 = 0x04;
const TYPE_I32 = 0x05;
const TYPE_I64 = 0x06;
const TYPE_U8  = 0x07;
const TYPE_U16 = 0x08;
const TYPE_U32 = 0x09;
const TYPE_U64 = 0x0A;

const TYPE_SINGLE         = 0x0B;
const TYPE_DOUBLE         = 0x0C;
const TYPE_COMPLEX_SINGLE = 0x0D;
const TYPE_COMPLEX_DOUBLE = 0x0E;

// Matrix types
const TYPE_MAT_I8  = 0x10 | TYPE_I8;
const TYPE_MAT_I16 = 0x10 | TYPE_I16;
const TYPE_MAT_I32 = 0x10 | TYPE_I32;
const TYPE_MAT_I64 = 0x10 | TYPE_I64;
const TYPE_MAT_U8  = 0x10 | TYPE_U8;
const TYPE_MAT_U16 = 0x10 | TYPE_U16;
const TYPE_MAT_U32 = 0x10 | TYPE_U32;
const TYPE_MAT_U64 = 0x10 | TYPE_U64;

const TYPE_MAT_SINGLE         = 0x10 | TYPE_SINGLE;
const TYPE_MAT_DOUBLE         = 0x10 | TYPE_DOUBLE;
const TYPE_MAT_COMPLEX_SINGLE = 0x10 | TYPE_COMPLEX_SINGLE;
const TYPE_MAT_COMPLEX_DOUBLE = 0x10 | TYPE_COMPLEX_DOUBLE;

// Other types
const TYPE_STRING     = 0x20;
const TYPE_ARRAY      = 0x21;
const TYPE_ASSOCARRAY = 0x22;

function loads(bytes) {
    let obj, nread;

    const dview = new DataView(bytes, 1);

    switch (new DataView(bytes).getUint8(0)) {
    case TYPE_NULL:
        obj = null;
        nread = 1;

        break;

    case TYPE_TRUE:
        obj = true;
        nread = 1;

        break;

    case TYPE_FALSE:
        obj = false;
        nread = 1;

        break;

    case TYPE_I8:
        obj = dview.getInt8(0);
        nread = 2;

        break;

    case TYPE_I16:
        obj = dview.getInt16(0);
        nread = 3;

        break;

    case TYPE_I32:
        obj = dview.getInt32(0);
        nread = 5;

        break;

    case TYPE_I64:
        obj = dview.getBigInt64(0);
        nread = 9;

        break;

    case TYPE_U8:
        obj = dview.getUint8(0);
        nread = 2;

        break;

    case TYPE_U16:
        obj = dview.getUint16(0);
        nread = 3;

        break;

    case TYPE_U32:
        obj = dview.getUint32(0);
        nread = 5;

        break;

    case TYPE_U64:
        obj = dview.getBigUint64(0);
        nread = 9;

        break;

    case TYPE_SINGLE:
        obj = dview.getFloat32(0);
        nread = 5;

        break;

    case TYPE_DOUBLE:
        obj = dview.getFloat32(0);
        nread = 5;

        break;

    case TYPE_COMPLEX_SINGLE:
        {
            const realpart = dview.getFloat32(0);
            const imagpart = dview.getFloat32(1);

            // ?
        }
        nread = 9;

        break;

    case TYPE_COMPLEX_DOUBLE:
        {
            const realpart = dview.getFloat64(0);
            const imagpart = dview.getFloat64(1);

            // ?
        }
        nread = 17;

        break;

    // ?

    case TYPE_STRING:
        {
            const len = dview.getUint32(0);

            obj = new TextDecoder().decode(bytes.slice(5, len + 5));
            nread = len + 5;
        }

        break;

    case TYPE_ARRAY:
        {
            const len = dview.getUint32(0);

            obj = [];
            nread = 5;

            for (let i = 0; i < len; i++) {
                const [elem, elem_siz] = loads(bytes.slice(nread));

                obj.append(elem);
                nread += elem_siz;
            }
        }

    case TYPE_ASSOCARRAY:
        {
            const len = dview.getUint32(0);

            obj = {};
            nread = 5;

            for (let i = 0; i < len; i++) {
                const [key, key_siz] = loads(bytes.slice(nread));
                nread += key_siz;

                const [val, val_siz] = loads(bytes.slice(nread));
                nread += val_siz;

                obj[key] = val;
            }
        }

        break;
    }

    return [obj, nread];
}

export function dimebloads(bytes) {
    const [obj, nread] = loads(bytes);

    return obj;
}

export function dimebdumps(x) {

}
