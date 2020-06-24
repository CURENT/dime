function [] = test_matlab_python_interop(pathname)
    d = dime('ipc', pathname);

    d.join('matlab');

    while ~any(strcmp(d.devices(), 'python'))
        pause(0.05);
    end

    nothing = [];
    boolean = true;
    int = int64(0xDEADBEEF);
    float = pi;
    complexfloat = complex(sqrt(0.75), 0.5);
    matrix = [1, 2, 3; 4, 5, 6; 7, 8, 9.01];
    string = 'Hello world!';
    sequence = {int64(-1), ':)', false, []};
    mapping = struct('foo', int64(2), 'bar', 'Green eggs and spam');

    d.send('python', 'nothing', 'boolean', 'int', 'float', 'complexfloat', 'matrix', 'string', 'sequence', 'mapping');

    nothing_old = nothing;
    clear nothing;
    boolean_old = boolean;
    clear boolean;
    int_old = int;
    clear int;
    float_old = float;
    clear float;
    complexfloat_old = complexfloat;
    clear complexfloat;
    matrix_old = matrix;
    clear matrix;
    string_old = string;
    clear string;
    sequence_old = sequence;
    clear sequence;
    mapping_old = mapping;
    clear mapping;

    pause(2);

    d.sync();

    assert(isequal(nothing_old, nothing));
    assert(isequal(boolean_old, boolean));
    assert(isequal(float_old, float));
    assert(isequal(int_old, int));
    assert(isequal(complexfloat_old, complexfloat));
    assert(isequal(matrix_old, matrix));
    assert(isequal(string_old, string));
    assert(isequal(sequence_old, sequence));
    assert(isequal(mapping_old, mapping));
end
