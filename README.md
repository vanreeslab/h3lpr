# H3LPR

Helper library for profiling, logging, and parsing.


## Dependencies

- c++17 compiler (`clang` or others)
- [Google Test](https://github.com/google/googletest): optional, only needed to compile the CI/CD testing

## Build

To build the `h3lpr` library (and install it in `PREFIX`):

```bash
ARCH_FILE=make_arch/make.yours PREFIX=/your/prefix make install

# to build the tests (optional)
ARCH_FILE=make_arch/make.yours make test
```

## Usage

### Profiler

The profiler is used to insert timings from within the code.
The impact on runtime should be very limited during the execution as the MPI-based computations only happen during the `Disp` function.
When starting or ending a timer, only the function `MPI_Wtime()` is called.

To get some fancy colors when compiling, add the option `-DCOLOR_PROF`.
You can also disable all the profiling using the option `-DNO_PROF`.


```c++
Profiler prof;

m_profStart(&prof,"step");
// do something
m_profStart(&prof,"substep");
// do something else, inside the something
m_profStop(&prof,"substep");
m_profStop(&prof,"step");

// here MPI calls will happen
m_profDisp(&prof);
```

In some cases it might be handy to initialize a profiler region without spending time in it.
This is for example the case when using multiple ranks and conditions:

```c++
Profiler prof;

// prevent other ranks than 0 to not have the profiler entry
m_profInitLeave(&prof,"step");
if (rank ==0){
    m_profStart(&prof,"step");
    m_profStop(&prof,"step");
}

m_profDisp(&prof);
```

### Parser

The parser can be used to read from the command line argument and/or from a configuration file.

We currently support:

- flags: `--flag` interpreted as a boolean
- arguments: `--argument=value` interpreted as a value

The way to used the parser can be summarized as below.
- first read the command line

```c++
Parser parser(argc,argv);

// add some flags + documentaton
bool is_flag = parser.GetFlag("--flag","the documentation of the flag");
double value = parser.GetValue<double>("--value","the documentation of the value",0.1);
auto values  = parser.GetValues<double,2>("--array","the documentation of the values",{0.1,0.2});

// display the help if needed
parser.Finalize();
```

In the command line, the use of the parser would then be

```bash
./program --flag --value=0.1 --array=1.7,2.9
```

In case you wish to display the possible flags, you can use the `--help` flag.
This is performed during the `Finalize` execution.

Finally, you can also use a configuration file to avoid typing a long sequence of arguments.
The file is given to the parser as `--config=filename`. The syntax would then be something like:

```makefile
# this is a comment on the flag
--flag
# this is a comment on the value
--value=0.1
# this is a comment on the array
--array=1.7,2.9
```


### Logging

The log is be used to print message in the output of the code with 3 flavors There are different ways of using it. 
The basic usage is the following

```c++
double pi = 3.1415;
m_log_def("pi is equal to %e", pi);
```

The output looks like: 

```text
[h3lpr] pi is equal to 3.1415
```
The format of the output follows the implementation of the standard. 

By default, only the rank 0 of `MPI_COMM_WORLD` will print the message. To have all the ranks displaying the message, the code must be compiled with `-DLOG_ALLRANKS`. 

To have a personalised header, we encourage you to define a macro in your code that wraps the default macro `m_log_default`. Here is the code snippet we used to define the logs of h3lpr:

```c++
#define m_log_h3lpr(format, ...)                   \
    ({                                             \
        m_log_def("h3lpr", format, ##__VA_ARGS__); \
    })
```
To perform a run without any log, you can compile the librairy with the `-DLOG_MUTE` flag.


### asserts

The assert macros is defined similarly to the log-one: `m_assert_def`.
The macro is disabled if the the file that includes the header is compiled with `-DNDEBUG`.
If not, then the backtrace is printed right after the assertion message, unless disabled using `-DNO_BTRACE`.

:warning: to ensure a readable naming information, we need the linker to have the following flags `-rdynamic -ldl`.

To use a personalized assertion, follow the same strategy as for the `m_log_def`:

```c++
#define m_assert_h3lpr(format, ...)                   \
    ({                                                \
        m_assert_def("h3lpr", format, ##__VA_ARGS__); \
    })
```
