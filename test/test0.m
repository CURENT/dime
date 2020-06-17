d = dime('ipc', '/tmp/dime.sock');
d.join('test0');

ct = 0;

tic;

while toc < 20
    d.sync();

    for i = 1:99
        try
            x = eval(sprintf('a%d', i));
        catch
            x = {};
        end

        if isnumeric(x)
            [m n] = size(x);
            ct = ct + m * n;
        end
    end

    for i = 1:99
        try
            x = eval(sprintf('b%d', i));
        catch
            x = {};
        end

        if isnumeric(x)
            [m n] = size(x);
            ct = ct + m * n;
        end
    end

    clear -regexp a[0-9]+ b[0-9]+;
end

disp(ct);
