.. _api_javascript:

============================
JavaScript API Reference
============================

DiME uses two custom JavaScript objects to handle interactions between the JavaScript client and the MATLAB and Python clients: Complex and NDArray.
The JavaScript client also has a few specialty functions for handling data.

------

------
Client
------

.. table::
    :align: left
    :widths: auto

    +---------------------------------------------------+----------------------------------+
    | :ref:`dime.DimeClient<api_javascript_dimeclient>` | The DiME client.                 |
    +---------------------------------------------------+----------------------------------+

------
Others
------

.. table::
    :align: left
    :widths: auto

    +----------------------------------------------+---------------------------------------------------------------------------+
    | :ref:`dime.Complex<api_javascript_complex>`  | A custom object that represents complex numbers.                          |
    +----------------------------------------------+---------------------------------------------------------------------------+
    | :ref:`dime.NDArray<api_javascript_ndarrays>` | A custom N-dimensional array object that acts similarly to the numpy      |
    |                                              | object with the same name.                                                | 
    +----------------------------------------------+---------------------------------------------------------------------------+
    | :ref:`jsonloads<api_javascript_jsonloads>`   | Loads a new object from JSON.                                             |
    +----------------------------------------------+---------------------------------------------------------------------------+
    | :ref:`jsondumps<api_javascript_jsondumps>`   | Returns the JSON string for an object.                                    |
    +----------------------------------------------+---------------------------------------------------------------------------+
    | :ref:`dimebloads<api_javascript_dimebloads>` | Loads an object from bytes.                                               |
    +----------------------------------------------+---------------------------------------------------------------------------+
    | :ref:`dimebdumps<api_javascript_dimebdumps>` | Returns the bytes of an object.                                           |
    +----------------------------------------------+---------------------------------------------------------------------------+