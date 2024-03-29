==================
DiME documentation
==================

**Useful Links**: `Source Repository`_ | `Report Issues`_
| `Q&A`_ | `LTB Website`_ | `LTB Repository`_

.. _`Source Repository`: https://github.com/CURENT/dime
.. _`Report Issues`: https://github.com/CURENT/dime/issues
.. _`Q&A`: https://github.com/CURENT/dime/discussions
.. _`LTB Website`: https://ltb.curent.org
.. _`LTB Repository`: https://github.com/CURENT
.. _`ANDES`: https://github.com/CURENT/andes
.. _`AGVis`: https://github.com/CURENT/agvis

.. image:: /images/sponsors/CURENT_Logo_NameOnTrans.png
   :alt: CURENT Logo
   :width: 300px
   :height: 74.2px

**DiME** (\ **Di**\ stributed **M**\ essaging **E**\ nvironment) is a library enabling multiple Matlab processes to 
share data across an operating system or over a network. 
This version is a rewrite of the original DiME, which suffered from severe slowdown from for large workloads. 
The current rewrite has many improvements upon the original, including single-threadedness, 
(I/O multiplexing is done via select(2)) fewer dependencies, and significantly higher throughput. 
DiME is compatible with MATLAB, Python, and JavaScript.

DiME is the distributed messaging module for the CURENT Largescale Testbed (LTB). More information about
CURENT LTB can be found at the `LTB Repository`_.

.. panels::
    :container: container-lg pb-2
    :card: + intro-card text-center
    :column: col-lg-6 col-md-6 col-sm-6 col-xs-12 d-flex

    ---

    Getting Started
    ^^^^^^^^^^^^^^^

    New to DiME? DiME has a few dependencies necessary to get it running, 
    so we've prepared a quick start guide to walk you through installation. 
    Currently there is a guide for Linux and Windows. 

    +++

    .. link-button:: quick_start
        :type: ref
        :text: To the getting started guides
        :classes: btn-block btn-secondary stretched-link

    ---

    API Reference
    ^^^^^^^^^^^^^

    The API reference contains a detailed description of the DiME package.
    The reference describes how the methods work and which parameters can be used
    for each of the MATLAB, JavaScript, and Python clients.

    +++

    .. link-button:: api_reference
        :type: ref
        :text: To the API reference
        :classes: btn-block btn-secondary stretched-link

    ---
    :column: col-12 p-3

    Examples
    ^^^^^^^^

    After setting up DiME and looking through the API, feel free to check out some 
    of the examples to get a better understanding of DiME's capabilities. 
    There are examples for each of the clients, showing interactions with the DiME 
    server and the passing of variables between instances of the clients. 

    +++

    .. link-button:: examples
        :type: ref
        :text: To the examples
        :classes: btn-block btn-secondary stretched-link

    ---
    :column: col-12 p-3

    Using DiME for Research?
    ^^^^^^^^^^^^^^^^^^^^^^^^
    Please cite our paper [Parsly2022]_ if DiME is used in your research for
    publication.


.. [Parsly2022] N. Parsly, J. Wang, N. West, Q. Zhang, H. Cui and F. Li, "DiME and AGVIS:
       A Distributed Messaging Environment and Geographical Visualizer for
       Large-scale Power System Simulation," in arXiv, Nov. 2022, doi: arXiv.2211.11990.

.. toctree::
   :caption: DiME Manual
   :maxdepth: 2
   :hidden:
   
   quick_start/index
   examples/index
   api
   api/javascript/index
   api/matlab/index
   api/python/index