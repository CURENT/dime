function [] = test_matlab_sync(pathname)
    d1 = dime('ipc', pathname);
    d2 = dime('ipc', pathname);
    d3 = dime('ipc', pathname);
    d4 = dime('ipc', pathname);

    d1.join('a', 'b', 'c', 'd', 'e');
    d2.join('a', 'b', 'c');
    d3.join('e', 'f');
    d4.join('g');

    devices = sort(d1.devices());

    assert(isequal(devices, {'a'; 'b'; 'c'; 'd'; 'e'; 'f'; 'g'}));

    d1.leave('a', 'b', 'c', 'd');
    d2.leave('a', 'b');
    d3.leave('e', 'f');

    d1.join('b');
    d3.join('h');
    d4.join('f');

    devices = sort(d1.devices());

    assert(isequal(devices, {'b'; 'c'; 'e'; 'f'; 'g'; 'h'}));

    d1.leave('b', 'e');
    d2.leave('c');
    d3.leave('h');
    d4.leave('f', 'g');

    devices = d1.devices();

    assert(isequal(devices, {}));
end
