d = dime('ipc', '/tmp/dime.sock');

i = 0;
lasti = 0;

varnames = {};

while true
    for j = 1:19
        while i == lasti
            i = randi(99);
        end
        lasti = i;

        varnames{j} = sprintf('a%d', i);

        assignin('base', varnames{j}, rand(randi(500), randi(500)));
    end

    send_var(d, 'test0', varnames{:});
end
