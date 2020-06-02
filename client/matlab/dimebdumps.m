function [bytes] = dimebdumps(obj)
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

    if isempty(obj) && ~iscell(obj)
        bytes = [TYPE_NULL];
    elseif islogical(obj) && isscalar(obj)
        if obj
            bytes = [TYPE_TRUE];
        else
            bytes = [TYPE_FALSE];
        end
    elseif isreal(obj) && ~ischar(obj)
        if isscalar(obj)
            switch string(class(obj))
            case 'int8'
                type = TYPE_I8;

            case 'int16'
                type = TYPE_I16;

            case 'int32'
                type = TYPE_I32;

            case 'int64'
                type = TYPE_I64;

            case 'uint8'
                type = TYPE_U8;

            case 'uint16'
                type = TYPE_U16;

            case 'uint32'
                type = TYPE_U32;

            case 'uint64'
                type = TYPE_U64;

            case 'single'
                type = TYPE_SINGLE;

            case 'double'
                type = TYPE_DOUBLE;
            end

            if ENDIANNESS == 'L'
                obj = swapbytes(obj);
            end

            bytes = [type typecast(obj, 'uint8')];
        else
            switch string(class(obj))
            case 'int8'
                type = TYPE_MAT_I8;

            case 'int16'
                type = TYPE_MAT_I16;

            case 'int32'
                type = TYPE_MAT_I32;

            case 'int64'
                type = TYPE_MAT_I64;

            case 'uint8'
                type = TYPE_MAT_U8;

            case 'uint16'
                type = TYPE_MAT_U16;

            case 'uint32'
                type = TYPE_MAT_U32;

            case 'uint64'
                type = TYPE_MAT_U64;

            case 'single'
                type = TYPE_MAT_SINGLE;

            case 'double'
                type = TYPE_MAT_DOUBLE;
            end

            shape = uint32(size(obj));

            if ENDIANNESS == 'L'
                shape = swapbytes(shape);
                obj = swapbytes(obj);
            end

            obj = obj(:).';
            bytes = [type uint8(length(shape)) typecast(shape, 'uint8') typecast(obj, 'uint8')];
        end
    elseif isnumeric(obj) && ~ischar(obj)
        if isscalar(obj)
            if isa(obj, 'single')
                type = TYPE_COMPLEX_SINGLE;
            elseif isa(obj, 'double')
                type = TYPE_COMPLEX_DOUBLE;
            else
                obj = double(obj);
                type = TYPE_COMPLEX_DOUBLE;
            end

            realpart = real(obj);
            imagpart = imag(obj);

            if ENDIANNESS == 'L'
                realpart = swapbytes(realpart);
                imagpart = swapbytes(imagpart);
            end

            bytes = [type typecast(realpart, 'uint8') typecast(imagpart, 'uint8')];
        else
            if isa(obj, 'single')
                type = TYPE_MAT_COMPLEX_SINGLE;
            elseif isa(obj, 'double')
                type = TYPE_MAT_COMPLEX_DOUBLE;
            else
                obj = double(obj);
                type = TYPE_MAT_COMPLEX_DOUBLE;
            end

            realpart = real(obj);
            imagpart = imag(obj);
            realimag = [realpart(:), imagpart(:)].';
            realimag = realimag(:).';

            shape = uint32(size(obj));

            if ENDIANNESS == 'L'
                shape = swapbytes(shape);
                realimag = swapbytes(realimag);
            end

            bytes = [type uint8(length(shape)) typecast(shape, 'uint8') typecast(realimag, 'uint8')];
        end
    elseif ischar(obj) || isstring(obj)
        if isstring(obj)
            obj = char(obj);
        end

        siz = uint32(length(obj));
        if ENDIANNESS == 'L'
            siz = swapbytes(siz);
        end

        bytes = [TYPE_STRING typecast(siz, 'uint8') uint8(obj)];
    elseif iscell(obj)
        siz = uint32(length(obj));
        if ENDIANNESS == 'L'
            siz = swapbytes(siz);
        end

        obj = cellfun(@dimebdumps, obj, 'UniformOutput', false);

        bytes = [TYPE_ARRAY typecast(siz, 'uint8') obj{:}];
    elseif isstruct(obj) || isa(obj, 'containers.Map')
        if isstruct(obj)
            k = fieldnames(obj);
            v = struct2cell(obj);
        else
            k = keys(obj);
            v = values(obj);
        end

        siz = uint32(length(k));
        if ENDIANNESS == 'L'
            siz = swapbytes(siz);
        end

        bytes = [TYPE_ASSOCARRAY typecast(siz, 'uint8')];

        for i = 1:length(k)
            bytes = [bytes dimebdumps(k{i}) dimebdumps(v{i})];
        end
    end
end
