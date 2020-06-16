% dime.m - DiME client for Matlab
% Copyright (c) 2020 Nicholas West, Hantao Cui, CURENT, et. al.
%
% Permission to use, copy, modify, and/or distribute this software for any
% purpose with or without fee is hereby granted, provided that the above
% copyright notice and this permission notice appear in all copies.
%
% This software is provided "as is" and the author disclaims all
% warranties with regard to this software including all implied warranties
% of merchantability and fitness. In no event shall the author be liable
% for any special, direct, indirect, or consequential damages or any
% damages whatsoever resulting from loss of use, data or profits, whether
% in an action of contract, negligence or other tortious action, arising
% out of or in connection with the use or performance of this software.

classdef dime < handle
    % dime DiME client
    %
    %   Allows a Matlab process do send/receive variables from its workspace to
    %   other clients connected to a shared DiME server. Note that this class
    %   includes several method aliases to be as API-compatible with the
    %   original DiME client as possible; However, since DiME2 introduces
    %   concepts foreign to the original DiME (e.g. groups), full API
    %   compatibility may not be attainable.

    properties (Access=private)
        serialization % Serialization method currently in use
        send_ll       % Low-level send function
        recv_ll       % Low-level receive function
        close_ll      % Low-level close function
    end

    methods
        function obj = dime(proto, varargin)
            % dime Construct a dime instance
            %
            %   Create a dime client via the specified protocol. The exact
            %   arguments depend on the protocol:
            %   * If the protocol is 'ipc', then the function expects one
            %     additional argument: the pathname of the Unix domain socket
            %     to connect to.
            %   * If the protocol is 'tcp', then the function expects two
            %     additional arguments: the hostname and port of the TCP socket
            %     to connect to, in that order.
            %
            % Inputs:
            %   proto : Transport protocol to use, either 'ipc' or 'tcp'
            %   varargin : Additional arguments, as described above
            %
            % Outputs:
            %   obj : The newly constructed dime instance

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

            sendmsg(obj, msg, uint8.empty);
            [msg, ~] = recvmsg(obj);

            if msg.status ~= 0
                error(msg.errmsg);
            end

            obj.serialization = msg.serialization;
        end

        function delete(obj)
            % delete Destruct a dime instance
            %
            %   Performs cleanup of a DiME client connection. This generally
            %   means closing the socket on which the connection was opened.
            %
            % Inputs:
            %   obj : The dime instance

            obj.close_ll();
        end

        function [] = join(obj, varargin)
            % join Send a "join" command to the server
            %
            %   Instructs the DiME server to add the client to one or more
            %   named groups.
            %
            % Inputs:
            %   obj : The dime instance
            %   varargin : The group name(s)

            for i = 1:length(varargin)
                msg = struct();

                msg.command = 'join';
                msg.name = varargin{i};

                sendmsg(obj, msg, uint8.empty);
            end
        end

        function [] = leave(obj, varargin)
            % leave Send a "leave" command to the server
            %
            %   Instructs the DiME server to remove the client from one or more
            %   named groups.
            %
            % Inputs:
            %   obj : The dime instance
            %   varargin : The group name(s)

            for i = 1:length(varargin)
                msg = struct();

                msg.command = 'leave';
                msg.name = varargin{i};

                sendmsg(obj, msg, uint8.empty);
            end
        end

        function [] = send_var(obj, name, varargin)
            % send_var Alias for send

            send(obj, name, varargin{:});
        end

        function [] = send(obj, name, varargin)
            % send Send a "send" command to the server
            %
            %   Sends one or more variables from the base workspace to all
            %   clients in a specified group.
            %
            % Inputs:
            %   obj : The dime instance
            %   name : the group name
            %   varargin : The variable name(s) in the workspace

            for i = 1:length(varargin)
                msg = struct();

                msg.command = 'sendmsg';
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

                sendmsg(obj, msg, bindata);
            end
        end

        function [] = broadcast(obj, varargin)
            % broadcast Send a "broadcast" command to the server
            %
            %   Sends one or more variables from the base workspace to all
            %   other clients.
            %
            % Inputs:
            %   obj : The dime instance
            %   varargin : The variable name(s) in the workspace

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

                sendmsg(obj, msg, bindata);
            end
        end

        function [] = sync(obj, varargin)
            % send Send a "sync" command to the server
            %
            %   Tell the server to start sending this client the variables sent
            %   to this client by other clients.
            %
            % Inputs:
            %   obj : The dime instance
            %   varargin : Either one argument specifying the maximum number of
            %              variables to receive, or blank to receive all
            %              variables sent.

            n = -1;

            if length(varargin) >= 1
                n = varargin{1};
            end

            msg = struct();

            msg.command = 'sync';
            msg.n = n;

            sendmsg(obj, msg, uint8.empty);

            while true
                [msg, bindata] = recvmsg(obj);

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

        function [names] = get_devices(obj)
            % get_devices alias for devices

            names = devices(obj);
        end

        function [names] = devices(obj)
            % send Send a "devices" command to the server
            %
            %   Tell the server to start sending this client the variables sent
            %   to this client by other clients.
            %
            % Inputs:
            %   obj : The dime instance
            %   varargin : Either one argument specifying the maximum number of
            %              variables to receive, or blank to receive all
            %              variables sent.

            msg = struct();

            msg.command = 'devices';

            sendmsg(obj, msg, uint8.empty);

            [msg, ~] = recvmsg(obj);

            names = msg.devices;
        end

        function [] = sendmsg(obj, json, bindata)
            % sendmsg Send a DiME message over the socket
            %
            % Inputs:
            %   obj : The dime instance
            %   json : JSON portion of the message to send
            %   bindata : Binary portion of the message to send

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

        function [json, bindata] = recvmsg(obj)
            % sendmsg Receive a DiME message over the socket
            %
            % Inputs:
            %   obj : The dime instance
            %
            % Outputs:
            %   json : JSON portion of the message received
            %   bindata : Binary portion of the message received

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
                    [json, bindata] = recvmsg(obj);
                else
                    error('Received unknown meta instruction');
                end
            end
        end
    end
end
