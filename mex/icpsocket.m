classdef ICPSocket
    properties
        fd
    end
    methods
        function obj = ICPSocket(filename)
            obj.fd = sunconnect(filename);
        end

        function delete(obj)
            sunclose(obj.fd);
        end

        function [] = send(obj, msg)
            % Convert msg to uint8 here
            sunsend(obj.fd, msg);
        end

        function msg = recv(obj)
            msg = sunrecv(obj.fd);
            % Convert msg from uint8 here
        end
    end
end
