# C++ Rapid Profile

Minimalistic, header only, Cpp/C++ execution profiler (C++11).

## Getting Started

Just copy the `rapid-profile.hpp` file to your sources or add it to your include paths.

```cpp
#include "rapid-profile.hpp"

int main() 
{
    RAPID_PROFILE_INIT();

    INTERVAL(RUNTIME);

    INTERVAL(EMPTY);
    INTERVAL_END(EMPTY);

    INTERVAL(LOOP);
    for (int i = 0; i < 5; i++)
    {
        INTERVAL(REPEAT_OUTER);
        INTERVAL(REPEAT_INNER);
        INTERVAL_END(REPEAT_INNER);
        INTERVAL_END(REPEAT_OUTER);
    }
    INTERVAL_END(LOOP);

    INTERVAL(INCREMENT);
    int i = 2;
    i++;
    INTERVAL_END(INCREMENT);

    INTERVAL_END(RUNTIME);

    return i;
}
```

Below is a sample of the output (post-processed) after executing tha above test.

```bash
-------------------------------------------------------------------------------------
   id                             name        cnt        min        max        avg
-------------------------------------------------------------------------------------
    0               RAPID_PROFILE_INIT          1      43.50      43.50      43.50
    1            RAPID_PROFILE_RECHUNK          1       8.90       8.90       8.90
    2                          RUNTIME          1       9.60       9.60       9.60
    3                            EMPTY          1       0.40       0.40       0.40
    4                             LOOP          1       6.70       6.70       6.70
    5                     REPEAT_OUTER          5       0.70       1.30       0.90
    6                     REPEAT_INNER          5       0.20       0.30       0.22
    7                        INCREMENT          1       0.20       0.20       0.20
-------------------------------------------------------------------------------------
```

## Synopsis

Rapid profile keeps track of all the timers defined by a named INTERVAL and INTERVAL_END set. Each time that execution passes through a timer block (INTERVAL to INTERVAL_END), a new associated interval is recorded. Upon termination/interrrupt, the recorded timers and associated intervals are exported to file.

## Target Audience

This tool is for you if you are:

- Looking for a low overhead, quick and easy to use C++ profiler.
- Trying to rapidly trackdown a latency issue.
- Wanting to quickly benchmark some routines.

This tool is not for you if you are:

- Looking for extensive profiling functionality.
- Unable to modify the sources and/or rebuild the software to be profiled.
- Not even willing to tolerate minimal overheads

## Preprocessor Switches

|Switch|Description|Default|
|:---|:---|:---:|
|RAPID_PROFILE_DSIABLE|Define this to completely disable rapid profile.|Undefined|
|RAPID_PROFILE_STR_SIZE|Sets the maximum number of characters, including a null terminator, for timer names (tags) and associated file names.|64|
|RAPID_PROFILE_MAX_TIMERS|Maximum number of timers that can be instantiated during a single execution.|1024|
|RAPID_PROFILE_CHUNK_SIZE|The size of each memory a chunk in bytes.|1048576|
|RAPID_PROFILE_THREAD_SAFE|Enables/disables threadsafety.|1|
|RAPID_PROFILE_INTERNAL|Enables/disables monitoring of internal execution times.|1|

## Implementation

For those of you that are more inquisitive and are not content with blindly using this trivial API, this section is for you. Thankfully, there is really not much under the hood.

### Timers

The timers are a preallocated vector of tags, where the vector size is defined by RAPID_PROFILE_MAX_TIMERS. The index of each tag is its unique ID. Each tag is a structure comprisesing of the following fields. That's about as specific as a timer needs to be.

|Type|Name|Description|
|:---|:---|:---:|
|char[RAPID_PROFILE_STR_SIZE]|name|Name of the timer (does not need to be unique)|
|char[RAPID_PROFILE_STR_SIZE]|filename|Filename where the timer is started|
|int|line number|Line number where the timer is started|

You may have noticed that RAPID_PROFILE_STR_SIZE is defined to allow each timer to be preallocated and reside in contiguous memory. This removes memory allocation delays and ensures efficient lookups. Furthermore, the number of timers, RAPID_PROFILE_MAX_TIMERS, is statically defined for efficiency reasons. Since each timer can be run multiple times (see intervals) and the results can be dynamically stored, there is no need to dynamically allocate the timers. In fact, to instantiate a new timer, a static code change is required (addition of INTERVAL and INTERVAL_END) anyway. If more timers are needed, RAPID_PROFILE_MAX_TIMERS can be updated accordingly.

### Intervals

An interval is just a start and stop time as well as a timer ID. Every time, that the INTERVAL macro is reached during execution, an interval with the associated timer ID is allocated (see chunker) and subsequently started. Every time the an INTERVAL_END macro is reached during execution, the current interval is stopped.

### Chunks

If you read the interval section, you will note that intervals are **dynamically allocated** 😱. Do not fear, there is minmal latency here 😅. This is precisely, the purpose of the "chunker". The chunker is essentially a linked list containing preallocated vectors of intervals, where the vector size is defined by RAPID_PROFILE_CHUNK_SIZE. At initialization a single chunk is created, therefore, all subsequent interval allocations are simply references to preallocated memory. Well, that is until there are more intervals than the chunk can contain. At this point a new chunk has to be allocated and added to the list. This is unavoidable, but the chunking latency can be recorded and deducted from the active intervals during post processing. This deduction is not completely accurate, more so in multi-threaded environments. Therefore, if even more accuracy is required, it is recommended that the RAPID_PROFILE_CHUNK_SIZE is increased in attempt to avoid chunking, where possible.

### Logging

Once the application terminates or receives an interrupt signal, two log files are generated (`tags.ro.csv` and `intervatls.rp.csv`). The first contains the list of all the timer tags and the second, all the recorded intervals.

### Thread Safety

To support thread safety, mutexs are used whenever shared resources are involved. Since this adds to execution overheads, RAPID_PROFILE_THREAD_SAFE can be used to disable thread safety in single thread execution.

### Internal Timers

It may be of interest to monitor rapid profile's initialization time as well as rechunking intervals. This can be controlled using RAPID_PROFILE_INTERNAL.

## Post Processing

For improved visualization, a post processing Python script `test/rapid-profile.py` is provided. This is a very elementry summary of results. Future releases will contain improved graphical representations.
