*This project has been created as part of the 42 curriculum by alyousse.*

## Description

Codexion is a concurrency simulation inspired by the classic Dining Philosophers problem. Multiple coders compete for shared USB dongles to compile quantum code. Each coder must hold two adjacent dongles simultaneously to compile, then releases them to debug and refactor before trying again. The goal is to keep all coders compiling without anyone burning out from resource starvation.

## Instructions

**Build:**
```
make
```

**Run:**
```
./codexion <nb_coders> <time_to_burnout> <time_to_compile> <time_to_debug> <time_to_refactor> <required_compiles> <dongle_cooldown> <scheduler>
```

All time arguments are in milliseconds. `scheduler` must be `fifo` or `edf`.

**Examples:**
```bash
# 5 coders, generous timing, FIFO scheduling
./codexion 5 800 200 200 200 3 50 fifo

# Same but EDF — coders closest to burnout get priority
./codexion 5 800 200 200 200 3 50 edf

# Single coder
./codexion 1 800 200 200 200 3 50 fifo

# Tight timing — one coder will burn out
./codexion 2 300 200 200 200 10 50 fifo
```

**Clean:**
```
make clean   # remove object files
make fclean  # remove object files and binary
make re      # full rebuild
```

## Resources

- [POSIX Threads Programming — Blaise Barney, LLNL](https://hpc-tutorials.llnl.gov/posix/)
- [The Little Book of Semaphores — Allen B. Downey](https://greenteapress.com/wp/semaphores/)
- [Dining Philosophers Problem — Wikipedia](https://en.wikipedia.org/wiki/Dining_philosophers_problem)
- [Earliest Deadline First Scheduling — Wikipedia](https://en.wikipedia.org/wiki/Earliest_deadline_first_scheduling)

**AI usage:** Claude Code was used to analyse and suggest a better approach, review norm compliance. All generated code was reviewed, understood, and integrated manually.

## Blocking Cases Handled

**Deadlock prevention:** Each coder always acquires dongles in index order (lower index first). Since all coders follow the same ordering, circular wait — one of Coffman's four necessary conditions — is impossible. The pair `(left=id-1, right=id%n)` is sorted before acquiring so no two coders wait on each other in a cycle.

**Starvation prevention:** A custom min-heap priority queue sits inside each dongle. When multiple coders compete, the scheduler (FIFO or EDF) determines who gets the dongle next. Under EDF, the coder whose burnout deadline is soonest wins, ensuring the most urgent coder always makes progress. Equal deadlines are broken by coder ID to make EDF fully deterministic.

**Cooldown handling:** After a dongle is released, it enters a cooldown period (`dongle_cooldown` ms) during which it cannot be taken. The waiting coder at the head of the queue uses `pthread_cond_timedwait` with a deadline set to the cooldown expiry, waking up exactly when the dongle becomes available again.

**Burnout detection:** A dedicated monitor thread polls every 1 ms, comparing `now - last_compile_start_ms` against `time_to_burnout` for each coder. The burnout message is printed within 10 ms of actual burnout time. On detecting burnout (or simulation completion), the monitor signals all waiting coders to unblock and exit cleanly.

**Single-coder edge case:** With one coder, both left and right dongle indices resolve to 0. Since a coder must hold two **distinct** dongles to compile but only one exists, it can never compile. The coder keeps picking up and dropping the single dongle without ever making progress, until the monitor detects it has not compiled within `time_to_burnout` and terminates the simulation.

## Thread Synchronization Mechanisms

**Per-coder mutex (`coder->m`):** Protects `compile_count` and `last_compile_start_ms`, which are written by the coder thread and read by both the coder thread and the monitor thread.

**Per-dongle mutex (`dongle->mutex`):** Protects `held`, `cooldown_until_ms`, and the priority queue (`queue`, `queue_size`). Any access to dongle state is done under this lock.

**Per-coder condition variable (`coder->wait_cond`):** Each coder has its own condvar. When blocked waiting for a dongle, the coder waits on its own condvar paired with the target dongle's mutex. On `dongle_release`, only the condvar of the next winner (the head of the dongle's priority queue) is signaled via `pthread_cond_signal` — not a broadcast. On simulation stop, `sim_wake_all` signals every coder's condvar so all blocked threads unblock and check the stop flag.

**Log mutex (`sim->log_m`):** Wraps every `printf` call to prevent interleaved output. The monitor prints the burnout message under this lock before setting the stop flag.

**Stop mutex (`sim->stop_m`):** Guards the shared `stop` integer. `sim_set_stop` and `sim_should_stop` always lock this mutex, ensuring the stop signal is visible across all threads without data races.

**Race condition example:** Without `coder->m`, the monitor could read a stale `last_compile_start_ms` while the coder thread is mid-update, incorrectly triggering a burnout. The mutex ensures the monitor always sees a complete, consistent value.
