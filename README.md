# NuLog

single header logging lib

![showcase](./assets/showcase.png)

## how to use

its very hard... you need to copy `nulog.h` into your project and include it:

```c
#include "nulog.h"
```

and using it is also just as hard...

```c
#include "nulog.h"

int main() {
    nulog_init(); // default settings

    DEBUG("hello, world?");
    INFO("hello world");
    WARN("hello world...");
    ERROR("HELLO WORLD!");
    FATAL("hello world :(");

    return 0;
}
```

## log levels
- `DEBUG`, `INFO`, `WARN`, `ERROR`, `FATAL`

## config

you can configure nulog's behaviour

```c
#include "nulog.h"

int main() {
    nulog_init(); // default settings

    // custom config
    NuLogConfig config = {
        .min_level = NULOG_LEVEL_INFO, // min. log level to display
        .show_timestamp = 1,           // should timestamps be shown in output
        .show_source = 1,              // should source file and line be shown in output
        .streams = {{stdout, 1}},      // output to stdout with colors
        .stream_count = 1
    };

    nulog_configure(config);

    // file stream
    FILE* log_file = fopen("/path/to/log.txt", "w");
    if (log_file) {
        nulog_add_stream(log_file, 0);
    }

    INFO("wow this is so cool");
    ERROR("oops");
    
    nulog_remove_stream(log_file);
    fclose(log_file);

    return 0;
}
```
\* colored output is automatically disabled for non-terminal streams (like files)

**inspired by [rxi/log.c](https://github.com/rxi/log.c/tree/master) i guess**
