.. _api_javascript:

============================
JavaScript API Reference
============================

DiME uses two custom JavaScript objects to handle interactions between the JavaScript client and the MATLAB and Python clients: Complex and NDArray.
The JavaScript client also has a few specialty functions for handling data.

----------

-------------
Instantiation
-------------

+--------------------------------------------+--------------+
| :ref:`dime.DimeClient<api_javascript_>`    | Dime client. |
+--------------------------------------------+--------------+

----------

-----
Other
-----

+--------------------------------------------+---------------------------------------------------------------------------+
| ``dime.Complex``                           | A custom object that represents complex numbers.                          |
+--------------------------------------------+---------------------------------------------------------------------------+
| ``dime.NDArray``                           | A custom N-dimensional array object that acts similarly to the numpy      |
|                                            | object with the same name.                                                | 
+--------------------------------------------+---------------------------------------------------------------------------+       

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