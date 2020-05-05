classdef dime
    properties
        conn
    end
    methods
        function obj = dime(name, proto, varargin)
            switch (proto)
            case 'icp'
                obj.conn = icpsocket(varargin{:});

            case 'tcp'
                obj.conn = tcpsocket(varargin{:});
            end

            msg = {};

            msg.command = 'register';
            msg.name = name;

            send(obj.conn, msg);
            msg = recv(obj.conn);

            if msg.status ~= 0
                error(msg.errmsg);
            end
        end

        function [] = send_var(obj, name, varargin)
            for i = 1:length(varargin)
                msg = {};

                msg.command = 'send';
                msg.name = name;
                msg.varname = varargin{i};
                msg.varval = evalin('base', varargin{i});

                send(obj.conn, msg);
            end
        end

        function [] = broadcast(obj, varargin)
            for i = 1:length(varargin)
                msg = {};

                msg.command = 'broadcast';
                msg.varname = varargin{i};
                msg.varval = evalin('base', varargin{i});

                send(obj.conn, msg);
            end
        end

        function [] = sync(obj)
            msg = {};

            msg.command = 'sync';

            send(obj.conn, msg);

            while true
                msg = recv(obj.conn);

                if ~isfield(msg, 'varname')
                    break;
                end

                assignin('base', msg.varname, msg.varval);
            end
        end
    end
end
