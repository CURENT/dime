# Welcome to DiME

## What is DiME?

**DiME** (**Di**stributed **M**ATLAB **E**nvironment) is a library enabling multiple Matlab processes to share data across an operating system or over a network. This version is a rewrite of the original DiME, which suffered from severe slowdown from for large workloads. The current rewrite has many improvements upon the original, including single-threadedness, (I/O multiplexing is done via select(2)) fewer dependencies, and significantly higher throughput. DiME is compatible with MATLAB, Python, and JavaScript.

The source repository for the current version of DiME can be found [here](https://github.com/CURENT/dime).

## Quick Start
DiME has a few dependencies necessary to get it running, so we've prepared a quick start guide to walk you through installation. Currently there is a guide for [Linux](/quick_start/linux) and [Windows](quick_start/windows). 


## API Reference
Although the API calls you will be making for each DiME client are fundamentally the same, there are slight differences between them due to the differences in the languages. To combat any possible confusion, references have been made for all three -- [MATLAB](/api/matlab), [Python](/api/python), and [JavaScript](/api/javascript).

## Examples
After setting up DiME and looking through the API, feel free to check out some of the to get a better understanding of DiME's capabilities. There are examples for each of the clients, showing interactions with the DiME server and the passing of variables between instances of the clients. (At least there will be soon.)  

