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

The log is be used to print message in the output of the code. There are different ways of using it. 
The basic usage is the following

```c++
double pi = 3.1415;
m_log_h3lpr("pi is equal to %e", pi);
```

The output looks like: 

```text
[h3lpr] pi is equal to 3.1415
```
The format of the output follows the implementation of the standard. 

By default, only the rank 0 of `MPI_COMM_WORLD` will print the message. To have all the ranks displaying the message, the code must be compiled with `-DLOG_ALLRANKS`. 

To have a personalised header, we encourage you to define a macro in your code that wraps the default macro `m_log_default`. Here is the code snippet we used to define the logs of h3lpr:

```
#define m_log_h3lpr(format, ...)                   \
    ({                                             \
        m_log_def("h3lpr", format, ##__VA_ARGS__); \
    })
```
To perform a run without any log, you can compile the librairy with the `-DLOG_MUTE` flag.
