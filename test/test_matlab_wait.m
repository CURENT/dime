function [] = test_matlab_wait(pathname, which)
    d = dime('ipc', pathname);

    if which == 0
        d.join('d1');

        while ~any(strcmp(d.devices(), 'd2'))
            pause(0.05);
        end

        a = 'ping';
        d.send('d2', 'a');

        n = d.wait();
        assert(n == 1 || n == 2);
        d.sync(1);

        assert(strcmp(a, 'pong'));

        t = timer('executionMode', 'singleShot', 'StartDelay', 10, 'timerFcn', @timeout);

        d.wait();

        delete(t);
    else
        d.join('d2');

        while ~any(strcmp(d.devices(), 'd1'))
            pause(0.05);
        end

        n = d.wait();
        assert(n == 1);
        d.sync();

        assert(strcmp(a, 'ping'));

        a = 'pong';
        b = uint32(0xDEADBEEF);
        d.send('d1', 'a', 'b');
    end
end

function [] = timeout(t, events)
    error('"wait" command timed out')
end
