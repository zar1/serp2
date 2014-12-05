serp2
=====

A scalable PCG implementation that can correct intermittent hardware faults.

This is the code I used to research the behavior of linear solvers when 
running in a faulty environment ([tech report](http://traell.cs.uchicago.edu/research/publications/techreports/TR-2013-11)).

Purpose
---------------------------------------------

On occasion, computers make mistakes due to physics. Alpha particle strikes,
radiation from packaging, and electronic errors resulting from operating at 
near-threshold voltage cause either arithmetic errors or memory corruption.
On a personal, 4-core system, these events will only take place on the order
of once every few years. However, on a supercomputer with hundreds of 
thousands of cores, these events may take once a day or once an hour. 
Conventionally, large systems deal with this problem by periodically saving
state and restarting from the most recent save when something bad happens. When
the system is big enough, these events may take place with sufficient 
frequency that the conventional strategy no longer allows for forward progress.
In such a climate, it is important to explore alternative ways of dealing with
intermittent hardware faults. One possibility is to deal with faults in an
application-specific way. Different applications have properties that
allow them to detect and correct faults in different ways. This code explores
how the very common Preconditioned Conjugate Gradient algorithm behaves in
a faulty environment and how effective different detection and correction 
methods are.

Building 
=========

Since this is research code developed under pressure, building and usage are 
really rather complicated. 

Building requires a few components:

1. [The GVR library](https://sites.google.com/site/uchicagolssg/lssg/research/gvr)
2. A version of [Trilinos](http://trilinos.org/) modified to utilize GVR.
Unfortunately, our Trilinos fork isn't provided here. You can contact me for
more information.

Given those two packages are installed, you will need to set the environmental
variable FTPCG\_RESOURCE\_PREFIX to point to a directory containing a 
collection of CSV files similar to those in the parameters folder and a
matrices folder containing .mtx files with sparse matrices to solve.

You will need to set TRILINOS\_INSTALL\_DIR to Trilinos' installation directory.

Then, cd to src and run make
