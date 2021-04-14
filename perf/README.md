### Perf benchmark

### Overview

- Using <i>perf_event_open</i> to get hardware and software counters.
- Only useful when you want to measure something inside the code really quick.  
  Otherwise, use <i>perf</i> itself.
- Linux only.
- All predefined hardware and software counters are generated in the header  
  file, you can uncomment counters as you wish.
- If some counters or combination of counters don't work or don't work as  
  expected, don't be surprised and check out performance counter register   
  allocation algorithm of Linux. CPUs have limited registers for performance  
  counters and some counters can use specific registers only.
- Golang version : https://github.com/tezc/goperf

### Usage

```c

#include "sc_perf.h"  

int main(int argc, char *argv[])
{
    sc_perf_start();

    long_running_operation();

    sc_perf_end();
  
    return 0;
}

```

##### Output will be like

```
| Event                     | Value              | Measurement time  
---------------------------------------------------------------
| time (seconds)            | 0.66               | (100,00%)  
| cpu-clock                 | 654075766.00       | (100.00%)  
| task-clock                | 654077198.00       | (100.00%)  
| page-faults               | 3.00               | (100.00%)  
| context-switches          | 46.00              | (100.00%)  
| cpu-migrations            | 0.00               | (100.00%)  
| page-fault-minor          | 3.00               | (100.00%)  
| cpu-cycles                | 2656529748.00      | (100.00%)  
| instructions              | 7589235720.00      | (100.00%)  
| cache-misses              | 28715.00           | (100.00%)  
| L1D-read-miss             | 34124.00           | (100.00%)  
| L1I-read-miss             | 121958.00          | (100.00%) 
```

##### Pause example

```c

#include "sc_perf.h"

int main(int argc, char *argv[])
{
    sc_perf_start();

    long_running_operation();

    //Will stop counters.
    sc_perf_pause();
    operation_you_dont_want_to_measure();

    //Start counters again.
    sc_perf_start();
    another_long_running_operation();

    sc_perf_end();
    
    sc_perf_start();
    some_other_long_running_operation();
    sc_perf_end();
  
    return 0;
}

```