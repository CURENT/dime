function [] = test_broadcast(pathname)
    d1 = dime('ipc', pathname);
    d2 = dime('ipc', pathname);
    d3 = dime('ipc', pathname);


    d1.join('d1');
    d2.join('d2');

    a = rand(500, 500);
    b = rand(500, 500);
    c = rand(500, 500);
    d1.broadcast('a', 'b', 'c');

    a_old = a;
    clear a;
    b_old = b;
    clear b;
    c_old = c;
    clear c;

    d2.sync();

    assert(isequal(a_old, a));
    assert(isequal(b_old, b));
    assert(isequal(c_old, c));

    clear a;
    clear b;
    clear c;

    d3.sync();

    assert(isequal(a_old, a));
    assert(isequal(b_old, b));
    assert(isequal(c_old, c));

    a = [];
    b = [];
    c = [];

    d1.sync();

    assert(~isequal(a_old, a));
    assert(~isequal(b_old, b));
    assert(~isequal(c_old, c));
end
