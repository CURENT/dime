.. _api_matlab:

====================
MATLAB API Reference
====================

DiME Instantiation
^^^^^^^^^^^^^^^^^^

dime(protocol, varargin) ::
    Creates a new DiME instance using the specified protocol.
        Parameters
            protocol
                The chosen protocol. Must be either 'ipc' or 'tpc'.
            varargin
                Additional parameters. Varies based on teh protocol chosen.
::

