function [] = test_matlab_sync(pathname)
    d1 = dime('ipc', pathname);
    d2 = dime('ipc', pathname);

    d1.join('d1');
    d2.join('d2');

    a_old = rand(500, 500);
    b_old = rand(500, 500);
    c_old = rand(500, 500);
    d_old = rand(500, 500);
    e_old = rand(500, 500);

    a = a_old;
    b = b_old;
    c = c_old;
    d = [];
    e = [];

    d1.send('d2', 'a', 'b', 'c');

    a = [];
    b = [];
    c = [];

    d2.sync(2);

    assert(isequal(a_old, a));
    assert(isequal(b_old, b));
    assert(~isequal(c_old, c));
    assert(~isequal(d_old, d));
    assert(~isequal(e_old, e));

    d = d_old;
    e = e_old;

    d1.send('d2', 'd', 'e');

    d = [];
    e = [];

    d2.sync();

    assert(isequal(a_old, a));
    assert(isequal(b_old, b));
    assert(isequal(c_old, c));
    assert(isequal(d_old, d));
    assert(isequal(e_old, e));
end
