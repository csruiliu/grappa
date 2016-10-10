Grappa
===============================================================================
![Build Status][buildStatus]

Grappa is a runtime system for scaling irregular applications on commodity clusters. It's a PGAS library and runtime system that allows you to write global-view C++11 code that runs on distributed-memory computers.

Grappa is a research project and is still young! Please expect things to break. Please do not expect amazing performance yet. Please ask for help if you run into problems. We're excited for you to use the software and to help make Grappa a great tool for the irregular applications community! For more information about the project, visit the project website: [grappa.io](http://grappa.io).

Dependences
-------------------------------------------------------------------------------
You must have a 64-bit Linux system with the following installed to build Grappa:

* Build system
  * Ruby >= 1.9.3
  * CMake >= 2.8.12
* Compiler
  * GCC >= 4.7.2 (we depend on C++11 features only present in 4.7.2 and newer)
  * Or: Clang >= 3.4
* External:
  * MPI (must support MPI-3)
    * OpenMPI >= 1.7.4
    * MVAPICH2 >= 1.9
    * MPICH >= 3.1
    * Intel MPI >= 5.0.2.044

The configure script deals with some other dependences automatically. You may want to override the default behavior for your specific system. See [BUILD.md](BUILD.md) for more details.

In addition, our test and run scripts all assume your machine uses the Slurm job manager. You may still run jobs with using any other MPI launcher, but you'll have to set necessary environment variables yourself. See [doc/running.md](doc/running.md) for more details.

Quick Start
-------------------------------------------------------------------------------
Ensure you have the dependences described above. Then checkout the code:

```bash
git clone git@github.com:uwsampa/grappa.git
```
If you don't have github keys set up and get an authentication error, then try the http URL
```bash
git clone http://github.com/uwsampa/grappa.git
```

Now build grappa and hello world.

```bash
cd grappa
./configure
cd build/Make+Release
make demo-hello_world
```

Now you should have a binary which you can launch as an MPI job. If you have Slurm installed on your system, you may be able to run jobs like this:

```bash
srun --nodes=2 --ntasks-per-node=2 -- applications/demos/hello_world/hello_world.exe
```

If that doesn't work, use whatever commands are required to launch MPI jobs on your system.

For more detailed instructions on building Grappa, see [BUILD.md](BUILD.md).

To run all our tests (a lengthy process) on a system using the Slurm job manager, do `make check-all-pass`. More information on testing is in [doc/testing.md](doc/testing.md).


Build and Launch From Zero (Fork 5th.10.2016)
-------------------------------------------------------------------------------

1. Install Git by **sudo apt-get install git** 
2. Git clone from Github to 64bit Linux Operating System (taking ubuntu 14.04LTS as an example)
3. Install Ruby by **sudo apt-get install ruby** (version: 1.9.3 p484)
4. Install cmake by **sudo apt-get install cmake** (version: 2.8.12.2)
5. Install g++ by **sudo apt-get install g++** (version: 4.8.4) and make sure gcc version > 4.7.3
6. Install mpich by **sudo apt-get install mpich** (version: 3.0.4) 
7. Install doxygen by **sudo apt-get install doxygen** (version: 1.8.6)
8. Download file from http://grappa.cs.washington.edu/files/grappa-third-party-downloads.tar and put it into grappa/third-party/downloads.
9. Go to grappa, compile by command, ./configure --cc={path/to/gcc} --third-party-tarfile={path/to/grappa-third-party-downloads.tar}
10. Go to grappa/build/Make+Release/, use make or make demo-hello_world command.
11. Install munge by **sudo apt-get install -y libmunge-dev munge build-essential**
12. Generate munge key and start munge daemon by **sudo /usr/sbin/create-munge-key**
13. (1) Check /etc/init.d/munge to verify the variable USER="munge" is defined. (2) Check /etc/default/munge to verify the USER variable is not redefined to something else. (3) Check /etc/passwd to make sure the munge user exists. (4) Check to make sure you're running /etc/init.d/munge start as root. (5) edit /etc/default/munge and add **OPTIONS="--syslog --force"**
14. Start munge by **sudo service munge start**
15. Install munge by **sudo apt-get install slurm-llnl**
16. Create slurm configure file in /etc/slurm-llnl by **sudo vim slurm.conf**
17. Copy an example configure file from SchedMD@Github.
18. Start slurm control daemon by **sudo slurmctld -c**
19. Start slurm daemons for each prefined nodes by **sudo slurmd -c -N nodename**
20. Check if everything is right by **sinfo**
21. Run the demo by **srun --nodes=2 --ntasks-per-node=2 -- applications/demos/hello_world/hello_world.exe**






Learning More
-------------------------------------------------------------------------------
You can learn more about Grappa's design and use in four ways:

1. Follow the tutorial in the doc/tutorial directory. Read about running jobs, debugging, tracing, and other low-level functionality in the `doc/` directory in the repo.
2. Take a look at the autogenerated API docs, hosted at [grappa.io/doxygen](http://grappa.io/doxygen/). Or you can build them yourself as explained in [BUILD.md](BUILD.md).
3. Read our papers, available from the Grappa website.

Virtual machine images
-------------------------------------------------------------------------------
We have a couple other ways to try Grappa without installing it yourself!

* [grappa-docker](https://github.com/uwsampa/grappa-docker/): Grappa in a docker container
* [grappa-starcluster](https://github.com/uwsampa/grappa-starcluster): Grappa in the Amazon cloud


Getting Help
-------------------------------------------------------------------------------
The best way to ask questions is to submit an issue on GitHub: by keeping questions there we can make sure the answers are easy for everyone to find. View previously-discussed issues here: https://github.com/uwsampa/grappa/issues?labels=question. If your question isn't already answered, please submit an issue there!

Grappa developers communicate through the grappa-dev mailing list hosted at cs.washington.edu.

Contributing
-------------------------------------------------------------------------------
We welcome contributions, both in the core software and (especially!) in applications. Get in touch with us if you're thinking of contributing something!


[buildStatus]: https://travis-ci.org/uwsampa/grappa.png?branch=master (https://travis-ci.org/uwsampa/grappa)
[![Analytics](https://ga-beacon.appspot.com/UA-33911150-1/uwsampa/grappa)](https://github.com/igrigorik/ga-beacon)


