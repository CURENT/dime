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



----------

----------
DimeClient
----------

    :ref:`new dime.DimeClient(hostname, port)<api_javascript_dimeclient>`

-------
Complex
-------
    
    :ref:`new dime.Complex(real, imaginary)<api_javascript_complex>`

--------
NDArrays
--------

    :ref:`new dime.NDArray(order, shape, array, complex)<api_javascript_ndarrays>`