function [] = test_matlab_tcp(hostname, port)
    d1 = dime('tcp', hostname, port);
    d2 = dime('tcp', hostname, port);

    d1.join('d1');
    d2.join('d2');

    a = rand(500, 500);

    d1.send('d2', 'a');

    a_old = a;
    clear a;

    d2.sync();

    assert(isequal(a_old, a));
end
