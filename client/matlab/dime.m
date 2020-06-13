classdef dime < handle
    properties
        name
        serialization
        send_ll
        recv_ll
        close_ll
    end

    methods
        function obj = dime(proto, varargin)
            switch (proto)
            case 'ipc'
                conn = sunconnect(varargin{:});

                obj.send_ll = @(msg) sunsend(conn, msg);
                obj.recv_ll = @(n) sunrecv(conn, n);
                obj.close_ll = @() sunclose(conn);

            case 'tcp'
                conn = tcpclient(varargin{:});

                obj.send_ll = @(msg) write(conn, msg);
                obj.recv_ll = @(n) read(conn, n);
                obj.close_ll = @() []; % TODO: does clear() close a TCP socket?
            end

            msg = struct();

            msg.command = 'register';
            msg.serialization = 'matlab';

            send(obj, msg, uint8.empty);
            [msg, ~] = recv(obj);

            if msg.status ~= 0
                error(msg.errmsg);
            end

            obj.serialization = msg.serialization;
        end

        function delete(obj)
            obj.close_ll();
        end

        function [] = join(obj, varargin)
            for i = 1:length(varargin)
                msg = struct();

                msg.command = 'join';
                msg.name = varargin{i};

                send(obj, msg, uint8.empty);
            end
        end

        function [] = leave(obj, varargin)
            for i = 1:length(varargin)
                msg = struct();

                msg.command = 'leave';
                msg.name = varargin{i};

                send(obj, msg, uint8.empty);
            end
        end

        function [] = send_var(obj, name, varargin)
            for i = 1:length(varargin)
                msg = struct();

                msg.command = 'send';
                msg.name = name;
                msg.varname = varargin{i};
                msg.serialization = obj.serialization;

                x = evalin('base', varargin{i});

                switch obj.serialization
                case 'matlab'
                    bindata = getByteStreamFromArray(x);

                case 'dimeb'
                    bindata = dimebdumps(x);
                end

                send(obj, msg, bindata);
            end
        end

        function [] = broadcast(obj, varargin)
            for i = 1:length(varargin)
                msg = struct();

                msg.command = 'broadcast';
                msg.varname = varargin{i};
                msg.serialization = obj.serialization;

                x = evalin('base', varargin{i});

                switch obj.serialization
                case 'matlab'
                    bindata = getByteStreamFromArray(x);

                case 'dimeb'
                    bindata = dimebdumps(x);
                end

                send(obj, msg, bindata);
            end
        end

        function [] = sync(obj, varargin)
            n = -1;

            if length(varargin) >= 1
                n = varargin{1};
            end

            msg = struct();

            msg.command = 'sync';
            msg.n = n;

            send(obj, msg, uint8.empty);

            while true
                [msg, bindata] = recv(obj);

                if ~isfield(msg, 'varname')
                    break;
                end

                switch msg.serialization
                case 'matlab'
                    x = getArrayFromByteStream(bindata);

                case 'dimeb'
                    x = dimebloads(bindata);

                otherwise
                    continue
                end

                assignin('base', msg.varname, x);
            end
        end

        function [devices] = get_devices(obj)
            msg = struct();

            msg.command = 'devices';

            send(obj, msg, uint8.empty);

            [msg, ~] = recv(obj);

            devices = msg.devices;
        end

        function [] = send(obj, json, bindata)
            [~, ~, endianness] = computer;

            json = uint8(jsonencode(json));

            json_len = uint32(length(json));
            bindata_len = uint32(length(bindata));

            % Convert to network (big) endianness
            if endianness == 'L'
                json_len = swapbytes(json_len);
                bindata_len = swapbytes(bindata_len);
            end

            header = [uint8('DiME') typecast(json_len, 'uint8') typecast(bindata_len, 'uint8')];

            obj.send_ll([header json bindata]);
        end

        function [json, bindata] = recv(obj)
            [~, ~, endianness] = computer;

            header = obj.recv_ll(12);

            if header(1:4) ~= uint8('DiME')
                error('Invalid DiME message');
            end

            json_len = typecast(header(5:8), 'uint32');
            bindata_len = typecast(header(9:12), 'uint32');

            % Convert from network (big) endianness
            if endianness == 'L'
                json_len = swapbytes(json_len);
                bindata_len = swapbytes(bindata_len);
            end

            % Faster to get both in one syscall
            msg = obj.recv_ll(json_len + bindata_len);

            json = jsondecode(char(msg(1:json_len)));
            bindata = msg((json_len + 1):end);

            if isfield(json, 'meta') && json.meta
                if isfield(json, 'serialization')
                    obj.serialization = json.serialization;
                    [json, bindata] = recv(obj);
                else
                    error('Received unknown meta instruction');
                end
            end
        end
    end
end
