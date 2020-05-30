function [obj, nread] = loads_mat(bytes, dtype, itemsize)
    [~, ~, ENDIANNESS] = computer;

    rank = bytes(2);
    shape = typecast(bytes(3:(4 * rank + 2)), 'uint32');

    if ENDIANNESS == 'L'
        shape = swapbytes(shape);
    end

    nread = 4 * rank + 2 + prod(shape) * itemsize;
    obj = typecast(bytes((4 * rank + 3):nread), dtype);
    if ENDIANNESS == 'L'
        obj = swapbytes(obj);
    end

    obj = reshape(obj, shape);
end

function [obj, nread] = loads(bytes)
    % Boolean sentinels
    TYPE_NULL  = uint8(0x00);
    TYPE_TRUE  = uint8(0x01);
    TYPE_FALSE = uint8(0x02);

    % Scalar types
    TYPE_I8  = uint8(0x03);
    TYPE_I16 = uint8(0x04);
    TYPE_I32 = uint8(0x05);
    TYPE_I64 = uint8(0x06);
    TYPE_U8  = uint8(0x07);
    TYPE_U16 = uint8(0x08);
    TYPE_U32 = uint8(0x09);
    TYPE_U64 = uint8(0x0A);

    TYPE_SINGLE         = uint8(0x0B);
    TYPE_DOUBLE         = uint8(0x0C);
    TYPE_COMPLEX_SINGLE = uint8(0x0D);
    TYPE_COMPLEX_DOUBLE = uint8(0x0E);

    % Matrix types
    TYPE_MAT_I8  = bitor(0x10, TYPE_I8);
    TYPE_MAT_I16 = bitor(0x10, TYPE_I16);
    TYPE_MAT_I32 = bitor(0x10, TYPE_I32);
    TYPE_MAT_I64 = bitor(0x10, TYPE_I64);
    TYPE_MAT_U8  = bitor(0x10, TYPE_U8);
    TYPE_MAT_U16 = bitor(0x10, TYPE_U16);
    TYPE_MAT_U32 = bitor(0x10, TYPE_U32);
    TYPE_MAT_U64 = bitor(0x10, TYPE_U64);

    TYPE_MAT_SINGLE         = bitor(0x10, TYPE_SINGLE);
    TYPE_MAT_DOUBLE         = bitor(0x10, TYPE_DOUBLE);
    TYPE_MAT_COMPLEX_SINGLE = bitor(0x10, TYPE_COMPLEX_SINGLE);
    TYPE_MAT_COMPLEX_DOUBLE = bitor(0x10, TYPE_COMPLEX_DOUBLE);

    % Other types
    TYPE_STRING     = uint8(0x20);
    TYPE_ARRAY      = uint8(0x21);
    TYPE_ASSOCARRAY = uint8(0x22);

    [~, ~, ENDIANNESS] = computer;

    switch bytes(1)

    case TYPE_NULL
        obj = [];
        nread = 1;

    case TYPE_TRUE
        obj = true;
        nread = 1;

    case TYPE_FALSE
        obj = false;
        nread = 1;

    case TYPE_I8
        obj = typecast(bytes(2), 'int8');
        nread = 2;

    case TYPE_I16
        obj = typecast(bytes(2:3), 'int16');
        if ENDIANNESS == 'L'
            obj = swapbytes(obj);
        end

        nread = 3;

    case TYPE_I32
        obj = typecast(bytes(2:5), 'int32');
        if ENDIANNESS == 'L'
            obj = swapbytes(obj);
        end

        nread = 5;

    case TYPE_I64
        obj = typecast(bytes(2:9), 'int64');
        if ENDIANNESS == 'L'
            obj = swapbytes(obj);
        end

        nread = 9;

    case TYPE_U8
        obj = bytes(2)
        nread = 2;

    case TYPE_U16
        obj = typecast(bytes(2:3), 'uint16');
        if ENDIANNESS == 'L'
            obj = swapbytes(obj);
        end

        nread = 3;

    case TYPE_U32
        obj = typecast(bytes(2:5), 'uint32');
        if ENDIANNESS == 'L'
            obj = swapbytes(obj);
        end

        nread = 5;

    case TYPE_U64
        obj = typecast(bytes(2:9), 'uint64');
        if ENDIANNESS == 'L'
            obj = swapbytes(obj);
        end

        nread = 9;

    case TYPE_SINGLE
        obj = typecast(bytes(2:5), 'single');
        if ENDIANNESS == 'L'
            obj = swapbytes(obj);
        end

        nread = 5;

    case TYPE_DOUBLE
        obj = typecast(bytes(2:9), 'double');
        if ENDIANNESS == 'L'
            obj = swapbytes(obj);
        end

        nread = 9;

    case TYPE_COMPLEX_SINGLE
        [real, imag] = typecast(bytes(2:9), 'single');

        if ENDIANNESS == 'L'
            real = swapbytes(real);
            imag = swapbytes(imag);
        end

        obj = complex(real, imag);
        nread = 9;

    case TYPE_COMPLEX_DOUBLE
        [real, imag] = typecast(bytes(2:17), 'double');

        if ENDIANNESS == 'L'
            real = swapbytes(real);
            imag = swapbytes(imag);
        end

        obj = complex(real, imag);
        nread = 17;

    case TYPE_MAT_I8
        [obj, nread] = loads_mat(bytes, 'int8', 1);

    case TYPE_MAT_I16
        [obj, nread] = loads_mat(bytes, 'int16', 2);

    case TYPE_MAT_I32
        [obj, nread] = loads_mat(bytes, 'int32', 4);

    case TYPE_MAT_I64
        [obj, nread] = loads_mat(bytes, 'int64', 8);

    case TYPE_MAT_U8
        [obj, nread] = loads_mat(bytes, 'uint8', 1);

    case TYPE_MAT_U16
        [obj, nread] = loads_mat(bytes, 'uint16', 2);

    case TYPE_MAT_U32
        [obj, nread] = loads_mat(bytes, 'uint32', 4);

    case TYPE_MAT_U64
        [obj, nread] = loads_mat(bytes, 'uint64', 8);

    case TYPE_MAT_SINGLE
        [obj, nread] = loads_mat(bytes, 'single', 4);

    case TYPE_MAT_DOUBLE
        [obj, nread] = loads_mat(bytes, 'double', 8);

    case TYPE_MAT_COMPLEX_DOUBLE
        rank = bytes(2);
        shape = typecast(bytes(3:(4 * rank + 2)), 'uint32');

        if ENDIANNESS == 'L'
            shape = swapbytes(shape);
        end

        nread = 4 * rank + 2 + prod(shape) * 8;
        realimag = typecast(bytes((4 * rank + 3):nread), 'single');
        real = realimag(1:2:length(realimag));
        imag = realimag(2:2:length(realimag));

        if ENDIANNESS == 'L'
            real = swapbytes(real);
            imag = swapbytes(imag);
        end

        obj = reshape(complex(real, imag), shape);

    case TYPE_MAT_COMPLEX_DOUBLE
        rank = bytes(2);
        shape = typecast(bytes(3:(4 * rank + 2)), 'uint32');

        if ENDIANNESS == 'L'
            shape = swapbytes(shape);
        end

        nread = 4 * rank + 2 + prod(shape) * 16;
        realimag = typecast(bytes((4 * rank + 3):nread), 'double');
        real = realimag(1:2:length(realimag));
        imag = realimag(2:2:length(realimag));

        if ENDIANNESS == 'L'
            real = swapbytes(real);
            imag = swapbytes(imag);
        end

        obj = reshape(complex(real, imag), shape);

    case TYPE_STRING
        siz = typecast(bytes(2:5), 'uint32');
        if ENDIANNESS == 'L'
            siz = swapbytes(siz);
        end

        nread = 5 + siz;
        obj = char(bytes(6:nread));

    case TYPE_ARRAY
        siz = typecast(bytes(2:5), 'uint32');
        if ENDIANNESS == 'L'
            siz = swapbytes(siz);
        end

        obj = {};
        nread = 5;

        for i = 1:siz
            [element, element_siz] = loads(bytes((nread + 1):length(bytes)));

            obj{i} = element;
            nread = nread + element_siz;
        end

    case TYPE_ASSOCARRAY
        siz = typecast(bytes(2:5), 'uint32');
        if ENDIANNESS == 'L'
            siz = swapbytes(siz);
        end

        obj = struct();
        nread = 5;

        for i = 1:siz
            [key, key_siz] = loads(bytes((nread + 1):length(bytes)));
            nread = nread + key_siz;

            [val, val_siz] = loads(bytes((nread + 1):length(bytes)));
            nread = nread + val_siz;

            if isa(obj, 'struct')
                try
                    obj.(key) = val;
                catch
                    obj = containers.Map(fieldnames(obj), struct2cell(obj));
                    obj(key) = val;
                end
            else
                obj(key) = val;
            end
        end
    end
end

function [obj] = dimebloads(bytes)
    [obj, ~] = loads(bytes);
end
