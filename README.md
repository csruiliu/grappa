Grappa
===============================================================================
![Build Status][buildStatus]

Grappa is a runtime system for scaling irregular applications on commodity clusters. It's a PGAS library and runtime system that allows you to write global-view C++11 code that runs on distributed-memory computers.

Grappa is a research project and is still young! Please expect things to break. Please do not expect amazing performance yet. Please ask for help if you run into problems. We're excited for you to use the software and to help make Grappa a great tool for the irregular applications community! For more information about the project, visit the project website: [grappa.io](http://grappa.io).

Dependences
-------------------------------------------------------------------------------
You must have a Linux system with the following installed to build Grappa:

* Build system
  * Ruby >= 1.9.3
  * CMake >= 2.8.6
* Compiler
  * GCC >= 4.7.2 (we depend on C++11 features only present in 4.7.2 and newer)
  * Or: Clang >= 3.4
* External:
  * MPI (tested with OpenMPI >= 1.5.4 and Mvapich2 >= 1.7, but should work with any)

The configure script deals with some other dependences automatically. You may want to override the default behavior for your specific system. See [BUILD.md](BUILD.md) for more details.

In addition, our test and run scripts all assume your machine uses the Slurm job manager. You may still run jobs with using any other MPI launcher, but you'll have to set necessary environment variables yourself. See [doc/running.md](doc/running.md) for more details.

Quick Start
-------------------------------------------------------------------------------
Ensure you have the dependences described above. Then:

```bash
git clone git@github.com:uwsampa/grappa.git
```
If you don't have github keys set up and get an authentication error, then try the http URL
```bash
git clone http://github.com/uwsampa/grappa.git
```

```bash
cd grappa
./configure
cd build/Make+Release
make -j demo-hello_world
```

Now you should have a binary which you can launch as an MPI job. If you have Slurm installed on your system, you can use our convenient job-launch script:

```bash
bin/grappa_srun --nnode=2 --ppn=2 -- applications/demos/hello_world/hello_world.exe
```

For more detailed instructions on building Grappa, see [BUILD.md](BUILD.md).

To run all our tests (a lengthy process) on a system using the Slurm job manager, do `make check-all-pass`. More information on testing is in [doc/testing.md](doc/testing.md).

Learning More
-------------------------------------------------------------------------------
You can learn more about Grappa's design and use in four ways:

1. Follow the tutorial in the doc/tutorial directory. Read about running jobs, debugging, tracing, and other low-level functionality in the `doc/` directory in the repo.
2. Take a look at the autogenerated API docs, hosted at [grappa.io/doxygen](http://grappa.io/doxygen/). Or you can build them yourself as explained in [BUILD.md](BUILD.md).
3. Read our papers, available from the Grappa website.

Getting Help
-------------------------------------------------------------------------------
The best way to ask questions is to submit an issue on GitHub: by keeping questions there we can make sure the answers are easy for everyone to find. View previously-discussed issues here: https://github.com/uwsampa/grappa/issues?labels=question. If your question isn't already answered, please submit an issue with the "Question" tag.

Grappa developers communicate through the grappa-dev mailing list hosted at cs.washington.edu.

Contributing
-------------------------------------------------------------------------------
We welcome contributions, both in the core software and (especially!) in applications. Get in touch with us if you're thinking of contributing something!


[buildStatus]: https://travis-ci.org/uwsampa/grappa.png?branch=master (https://travis-ci.org/uwsampa/grappa)
[![Analytics](https://ga-beacon.appspot.com/UA-33911150-1/uwsampa/grappa)](https://github.com/igrigorik/ga-beacon)


