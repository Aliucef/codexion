# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
make        # build the codexion binary
make clean  # remove object files and binary
make fclean # same as clean (also removes binary)
make re     # clean rebuild
```

Compiled with `cc -Wall -Wextra -Werror -pthread`. No external dependencies.

## Running

```
./codexion <nb_coders> <time_to_burnout> <time_to_compile> <time_to_debug> <time_to_refactor> <required_compiles> <dongle_cooldown> <scheduler>
```

- All numeric args are in milliseconds.
- `<scheduler>` is either `fifo` or `edf`.
- Exactly 8 arguments required.

## Architecture

This is a 42 school "Philosophers"-style concurrency simulation — coders (threads) compete for dongles (shared resources/forks) to compile code, with a monitor thread detecting burnout.

**Central state: `t_sim`** (`coders/parser/parse.h`)
- Owns `t_coder[]` (one per thread) and `t_dongle[]` (one per coder, arranged in a ring)
- Shared mutexes: `log_m` (serializes stdout) and `stop_m` (guards the global stop flag)

**Thread model:**
- One `coder_routine` thread per coder (`coders/coders.c`) — each must acquire two adjacent dongles (lower index first to avoid deadlock) before compiling, then releases them and sleeps through debug/refactor phases
- One `monitor_routine` thread — polls every 1ms, triggers stop if any coder exceeds `time_to_burnout` ms without starting a compile, or if all coders have reached `required_compiles`

**Dongle locking** (`coders/dongles/dongles.c`):
- Each `t_dongle` has a mutex + condition variable + `cooldown_until_ms` timestamp
- `dongle_take` blocks via `pthread_cond_timedwait` until the dongle is free and its cooldown has elapsed
- `dongle_release` sets a new cooldown and broadcasts to all waiters
- On stop, `sim_wake_all` broadcasts on every dongle condvar so blocked threads can exit

**Stop propagation** (`coders/stop/`): `sim_set_stop`/`sim_should_stop` guard a single `int stop` field behind `stop_m`. All loops check this flag to exit cleanly.

**Module layout:**
- `coders/parser/` — arg parsing (`parse.c`) and simulator init (`init_simulator.c`)
- `coders/validation/` — argument validation and scheduler string parsing (`fifo`/`edf`)
- `coders/logs/` — timestamped thread-safe `log_state` (locks `log_m`)
- `coders/stop/` — stop flag accessors
- `coders/dongles/` — dongle acquire/release with cooldown
- `coders/coders.c` — `coder_routine` and `monitor_routine`
- `coders/main.c` — entry point, thread creation, join, cleanup

## 42 Norminette

Code must pass `norminette`. Key rules: 25-line function limit, no more than 5 variables per function, no `for` loops, no variable declarations after statements. Use `make re` after norm fixes to verify compilation.
