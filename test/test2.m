d = dime('test2', 'ipc', '/tmp/dime.sock');

i = 0;
lasti = 0;

while true
    while i == lasti
        i = randi(30);
    end
    lasti = i;

    varname = sprintf('b%d', i);

    assignin('base', varname, rand(randi(500), randi(500)));
    send_var(d, 'test0', varname);
end
