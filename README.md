# H3LPR

Helper library for profiling, logging, and parsing.


### Profiler

The profiler is used to insert timings from within the code.
The impact on runtime should be very limited during the execution as the MPI-based computations only happen during the `Disp` function.
When starting or ending a timer, only the function `MPI_Wtime()` is called.

To get some fancy colors when compiling, add the option `-DCOLOR_PROF`.


```c++
Profiler prof;

m_profStart(&prof,"step");
// do something
m_profStart(&prof,"substep");
// do something else, inside the something
m_profEnd(&prof,"substep");
m_profEnd(&prof,"step");

// here MPI calls will happen
prof.Disp()
```

### Parser

The parser can be used to read from the command line argument and/or from a configuration file.

We currently support:

- flags: `--flag` interpreted as a boolean
- arguments: `--argument=value` interpreted as a value

The way to used the parser is:

- first read the command line
```c++
Parser parser(argc,argv)
```

- indicate which value is used 

### Logging


