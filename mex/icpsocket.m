classdef icpsocket
    properties
        fd
    end
    methods
        function obj = icpsocket(filename)
            obj.fd = sunconnect(filename);
        end

        function delete(obj)
            sunclose(obj.fd);
        end

        function [] = send(obj, msg)
            sunsend(obj.fd, uint8(json_dump(msg)));
        end

        function msg = recv(obj)
            msg = json_load(char(sunrecv(obj.fd)));
        end
    end
end
