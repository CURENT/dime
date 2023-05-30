===================
DiME documentation
===================

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
    :card: + intro-card text-center
    :column: col-lg-6 col-md-6 col-sm-6 col-xs-12 d-flex

    ---

    Getting started
    ^^^^^^^^^^^^^^^

    New to DiME? DiME has a few dependencies necessary to get it running, so we've prepared a quick start guide to walk you through installation. 
    Currently there is a guide for `Linux <./quick_start/linux.html>`_ and `Windows <./quick_start/windows.hmtl>`_. 

    +++

    .. link-button:: getting-started
            :type: ref
            :text: To the getting started guides
            :classes: btn-block btn-secondary stretched-link

    ---

    Advanced usage
    ^^^^^^^^

    The usage provide instructions on how to use AGVis to its various features,
    including Configuration, MultiLayer, and Independent Data Reader. 

    +++

    .. link-button:: usage
            :type: ref
            :text: To the advanced usage
            :classes: btn-block btn-secondary stretched-link

    ---

    Model development guide
    ^^^^^^^^^^^^^^^^^^^^^^^

    Looking to implement new functionalities in AGVis?
    The development guide provides in-depth information on the
    implementation of the geographical visualization.

    +++

    .. link-button:: development
            :type: ref
            :text: To the development guide
            :classes: btn-block btn-secondary stretched-link
    ---

    API reference
    ^^^^^^^^^^^^^

    The API reference contains a detailed description of the AGVis package. The
    reference describes how the methods work and which parameters can be used.

    +++

    .. link-button:: api_reference
            :type: ref
            :text: To the API reference
            :classes: btn-block btn-secondary stretched-link

    ---
    :column: col-12 p-3

    Using AGVis for Research?
    ^^^^^^^^^^^^^^^^^^^^^^^^^
    Please cite our paper [Parsly2022]_ if AGVis is used in your research for
    publication.











Welcome to DiME
###############

What is DiME?
-------------

**DiME** (\ **Di**\ stributed **M**\ essaging **E**\ nvironment) is a library enabling multiple Matlab processes to share data across an operating system or over a network. This version is a rewrite of the original DiME, which suffered from severe slowdown from for large workloads. The current rewrite has many improvements upon the original, including single-threadedness, (I/O multiplexing is done via select(2)) fewer dependencies, and significantly higher throughput. DiME is compatible with MATLAB, Python, and JavaScript.

The source repository for the current version of DiME can be found `here <https://github.com/CURENT/dime>`_.

Quick Start
-----------

DiME has a few dependencies necessary to get it running, so we've prepared a quick start guide to walk you through installation. Currently there is a guide for `Linux <./quick_start/linux.html>`_ and `Windows <./quick_start/windows.hmtl>`_. 

API Reference
-------------

Although the API calls you will be making for each DiME client are fundamentally the same, there are slight differences between them due to the differences in the languages. To combat any possible confusion, references have been made for all three -- `MATLAB <./api/matlab.html>`_, `Python <./api/python.html>`_, and `JavaScript <./api/javascript>`_.

Examples
--------

After setting up DiME and looking through the API, feel free to check out some of the to get a better understanding of DiME's capabilities. There are examples for each of the clients, showing interactions with the DiME server and the passing of variables between instances of the clients. 



.. toctree::
   :caption: DiME Manual
   :maxdepth: 2
   :hidden:
   
   Linux Installation <quick_start/linux.md>
   Windows Installation <quick_start/windows.md>
   JavaScript API <api/javascript.md>
   MATLAB API <api/matlab.md>
   Python API <api/python.md>
   JavaScript Examples <examples/jex.md>
   MATLAB Examples <examples/mex.md>
   Python Examples <examples/pex.md>
