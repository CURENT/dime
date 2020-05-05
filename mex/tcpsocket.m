classdef TCPSocket
    properties
        fd
        rbuf
        wbuf
    end
    methods
        function obj = TCPSocket(hostname, port)
            obj.fd = tcpclient(hostname, port, 'Timeout', 60);
            obj.rbuf = uint8.empty;
        end

        function delete(obj)
            clear obj.fd;
        end

        function [] = send(obj, msg)
            % Convert msg to uint8 here
            write(obj.fd, msg);
        end

        function msg = recv(obj)
            while true
                try
                    % Convert msg from uint8 here
                catch
                    % The way Matlab handles sockets is less than ideal; it's
                    % non-blocking and reads all available bytes if no size is
                    % given, but blocks until _exactly_ the number of bytes is
                    % read if it is given
                    c = read(obj.fd, 1);
                    obj.rbuf = [obj.rbuf c read(obj.fd)];
                    continue
                end

                break
            end
        end
    end
end
