# SiNUCA
Simulator or Non-Uniform Cache Architectures

======================================================
This readme author's: 	

- Alves, Marco A. Z. 		- PhD at Federal University of Paraná
- Nguyen, Hoang Anh Du    - PhD student at TU Delft
- Cordeiro, Aline S. 		- Master student at Federal University of Paraná
- Moreira, Francis B. 	- PhD student at Federal University of Rio Grande do Sul	

======================================================
Sections

    1. SiNUCA Pin requirements
    2. Compiling SiNUCA trace generator
    3. Generating traces
    4. Compiling SiNUCA
    5. Running SiNUCA
    6. README Changelog

======================================================
1. SiNUCA Pin requirements

    - Older Pin versions:
    	Intel Pinplay:
    		https://software.intel.com/en-us/articles/program-recordreplay-toolkit
    	Intel Pin:
    		https://software.intel.com/en-us/articles/pin-a-dynamic-binary-instrumentation-tool

        To set-up the pin path, use the following commands:
			$ export PINPLAY_ROOT=/pinplay-folder
			$ export PIN_ROOT=/pin-folder
			$ export PIN_HOME=$PIN_ROOT
			$ export PATH=$PIN_ROOT:$PINPLAY_ROOT:$PATH

        Copy folder sinuca_tracer to the folder
			pinplay-folder/extras/pinplay/examples

        Copy a file pinplay-debugger-shell.H 
        from: 	pinplay-folder/extras/pinplay/examples/
        to: 	sinuca-folder/trace_generator/extras/pinplay/sinuca_tracer/

    - Newer Pin Versions:
        Intel Pinplay + Intel Pin 3.5:
            https://software.intel.com/en-us/articles/program-recordreplay-toolkit

        Extract the contents from 'Intel Pinplay + Intel Pin 3.5' 
        directly into the folder: 
			sinuca-folder/trace_generator/

        Copy the file pinplay-debugger-shell.H
        from: 	sinuca-folder/trace_generator/extras/pinplay/examples/
        to: 	sinuca-folder/trace_generator/extras/pinplay/sinuca_tracer/

        To set-up the pin path, use the following commands:
            $ export PIN_ROOT=/sinuca-folder/trace_generator/
            $ export PINPLAY_HOME=/$PIN_ROOT/extras/pinplay/
            $ export PINPLAY_INCLUDE_HOME=/$PINPLAY_HOME/include/
            $ export PINPLAY_LIB_HOME=/$PINPLAY_HOME/lib/
            $ export EXT_LIB_HOME=/$PINPLAY_HOME/lib-ext/
            $ export PATH=$PIN_ROOT:$PINPLAY_HOME:$PINPLAY_INCLUDE_HOME:$PINPLAY_LIB_HOME:$EXT_LIB_HOME:$PATH

======================================================
2. Compiling SiNUCA trace generator

    - Older Versions:
        SiNUCA tracer is provided in /sinuca-folder/trace_generator/,
        and it can be compiled on Unix-like OSes using make:

            $ make

    	After compiling, sinuca_tracer.so is created and moved to
    	/pinplay-folder/extras/pinplay/bin/intel64/sinuca_tracer.so

    - Newer Versions:
        SiNUCA tracer file is provided in
        /sinuca-folder/trace_generator/extras/pinplay/sinuca_tracer/,
        and it can be compiled on Unix-like OSes using make:

            $ make

        After compiling, sinuca_tracer.so is created and moved to
        /sinuca-folder/trace_generator/extras/pinplay/bin/intel64/sinuca_tracer.so

======================================================
3. Generating traces

    - Older Versions:
        SiNUCA is programmed under GNU C++, and it can be executed on Unix-like OSes using pin:

            $ pin -t
             /pinplay-folder/extras/pinplay/bin/intel64/sinuca_tracer.so -- ./app

        Three SiNUCA traces (stat, dyn, and mem) will be generated.

    - Newer Versions:
        SiNUCA is programmed under GNU C++, and it can be executed on Unix-like OSes using pin:

            $ pin -t
            /sinuca-folder/tracer_generator/extras/pinplay/bin/intel64/sinuca_tracer.so
            -trace_mode <mode> -- ./app

        "-trace_mode" is an optional flag present in the newest trace-generator, it defines which 
		architecture the traces will be based on (x86 or HMC). 
		The option <mode> can be (default is ix86):
            
			- ix86 (intrinsics-x86 traces);
            - iHMC (intrinsics-HMC traces);

        Three SiNUCA traces (stat, dyn, and mem) will be generated.

======================================================
4. Compiling SiNUCA

    SiNUCA is programmed under GNU C++, and it can be compiled on Unix-like OSes using make:

        $ make

======================================================
5. Running SiNUCA

	SiNUCA takes two inputs: configuration file (.cfg) and trace files (.out.gz)

    	$ ./sinuca --config file.cfg --trace basename

	Typing ./sinuca may help you out figure out what
	parameters are required to full execution a simulation.

	Configuration file is the file contains all info of processor, memory, interconnection.
	For example: sandy_16cores.cfg
	Example configuration file is in the folder:
		sinuca-folder/config_examples/

	The basename is the name of the trace without thread number.
	For example: 3 traces are named as
	unroll_vecsum.1MB.1t.tid0.dyn.out.gz
	unroll_vecsum.1MB.1t.tid0.mem.out.gz
	unroll_vecsum.1MB.1t.tid0.stat.out.gz
	
	then the basename is:
		unroll_vecsum.1MB.1t

======================================================
6. README changelog

    30/08/2017 - Created first README
    28/07/2018 - Additional informations about Intel Pinplay and Intel Pin.


======================================================
Extra information

Please refer to the following documents for a general introduction of the tool:

Alves, Marco Antonio Zanata. "Increasing energy efficiency of processor caches via line usage predictors." (2014).

Alves, Marco Antonio Zanata, et al. "Sinuca: A validated micro-architecture simulator." High Performance Computing and Communications (HPCC), 2015 

Alves, Marco AZ, et al. "Large vector extensions inside the HMC." Design, Automation & Test in Europe Conference & Exhibition (DATE), 2016. IEEE, 2016.

