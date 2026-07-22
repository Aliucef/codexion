# Codexion — Deep Dive Explanation

---

## 1. The Origin: Dining Philosophers

This project is a direct re-skin of one of the most famous problems in computer science: **The Dining Philosophers Problem**, introduced by Edsger Dijkstra in 1965.

The original setup: five philosophers sit at a round table. Between each pair of neighbors lies a single fork. To eat, a philosopher needs two forks — one on their left, one on their right. They spend their life alternating between thinking and eating.

The problem has nothing to do with food. It is a model for:
- Multiple independent processes (philosophers) competing for shared, limited resources (forks)
- The need to coordinate access without any direct communication between processes

In Codexion, the re-skin is:
- Philosophers → **Coders**
- Forks → **USB Dongles**
- Eating → **Compiling** (requires 2 dongles simultaneously)
- Thinking → **Debugging + Refactoring** (no resources needed)
- Starving to death → **Burning Out** (didn't compile within the deadline)

The cosmetic renaming doesn't change the fundamental challenge — it is still the Dining Philosophers problem at its core.

---

## 2. Why Is This Problem Hard?

Before explaining the solution, it helps to understand exactly what can go wrong.

### 2.1 Deadlock

Imagine 5 coders all simultaneously grab their **left** dongle. Now everyone is waiting for their **right** dongle — which is being held by the person to their right, who is also waiting. Nobody can proceed. Nobody will ever release. The simulation freezes forever.

This is **deadlock**: a circular wait where every participant is waiting for a resource held by the next participant.

Deadlock requires four conditions to all be true simultaneously (Coffman's conditions):
1. **Mutual exclusion** — a dongle can only be held by one coder at a time
2. **Hold and wait** — a coder holds one dongle while waiting for the second
3. **No preemption** — you cannot forcibly take a dongle from someone else
4. **Circular wait** — A waits on B, B waits on C, C waits on A

Conditions 1, 2, and 3 are inherent to the problem. The only one we can eliminate is **circular wait**.

### 2.2 Starvation

Even without deadlock, a coder might consistently lose the race for dongles to its neighbors. If the scheduling is unfair, one coder could theoretically never compile — and eventually burn out — even though the system as a whole keeps moving. This is **starvation**.

### 2.3 Race Conditions

Multiple threads share data: compile counts, timestamps, the stop flag, the dongle state. If two threads read and write the same memory without coordination, you get undefined behavior — corrupted values, missed updates, ghost signals.

---

## 3. The Architecture — How We Structured the Solution

The program has a clear layered structure:

```
main.c          — entry point, thread creation, teardown
coders.c        — per-coder thread logic (the compile/debug/refactor cycle)
monitor.c       — monitor thread logic (burnout detection, termination)
dongles/        — the shared resource layer (dongle locking with scheduling)
  dongles.c     — dongle_take and dongle_release
  pqueue.c      — min-heap: sift up/down, comparison
  pqueue2.c     — min-heap: push, pop, peek, remove
parser/         — argument parsing and simulator initialization
  parse.c       — init_arguments, get_time_ms
  init_simulator.c — malloc and init all structs
validation/     — is_valid, parse_scheduler
stop/           — sim_should_stop, sim_set_stop
logs/           — log_state (mutex-protected printf)
```

---

## 4. The Central Data Structures

Understanding the structs is understanding the program.

### `t_sim` — The simulation world

```c
typedef struct s_sim {
    t_parse         config;         // all arguments parsed from argv
    long            start_ms;       // when the simulation started
    int             stop;           // global stop flag (0 = running, 1 = stop)
    pthread_mutex_t stop_m;         // mutex protecting stop
    pthread_mutex_t log_m;          // mutex protecting printf output
    t_coder         *coders;        // array of N coders
    t_dongle        *dongles;       // array of N dongles
} t_sim;
```

This is the single shared object that every thread has access to. All coordination flows through it.

### `t_coder` — One coder (one thread)

```c
typedef struct coders {
    int             id;                 // coder number (1 to N)
    pthread_t       th;                 // the POSIX thread handle
    int             compile_count;      // how many times this coder has compiled
    long            last_compile_start_ms; // timestamp of last compile start
    pthread_mutex_t m;                  // protects compile_count and last_compile_start_ms
    pthread_cond_t  wait_cond;          // this coder's personal condition variable
    t_sim           *sim;               // pointer back to the simulation
} t_coder;
```

The key addition (not in the classic philosophers problem) is `wait_cond`. Each coder has its **own** condition variable. This is how we achieve targeted signaling — instead of waking everyone up when a dongle is released, we only wake up the specific coder who won the scheduling decision.

`last_compile_start_ms` is critical: it is what the monitor watches. If `now - last_compile_start_ms > time_to_burnout`, that coder has burned out.

### `t_dongle` — One shared resource

```c
typedef struct s_dongle {
    pthread_mutex_t mutex;          // protects all fields below
    int             held;           // 1 if currently held by a coder, 0 if free
    long            cooldown_until_ms; // dongle unusable until this timestamp
    t_waiter        *queue;         // min-heap of waiting coders
    int             queue_size;     // current number of waiters in the heap
} t_dongle;
```

The `queue` is a dynamically allocated min-heap. When multiple coders want the same dongle, they all register themselves in this heap. The heap's ordering (FIFO or EDF) determines who gets it first.

### `t_waiter` — One entry in the scheduling heap

```c
typedef struct s_waiter {
    int     coder_id;   // which coder is waiting
    long    priority;   // lower value = wins sooner
} t_waiter;
```

For **FIFO**, priority = the timestamp when the request arrived (earlier = lower value = wins).
For **EDF**, priority = `last_compile_start_ms + time_to_burnout` (earlier deadline = lower value = wins).

---

## 5. Solving Deadlock — Fixed Ordering

The fix for circular wait is simple and elegant: **always acquire resources in the same global order**.

Every coder sits between two dongles. We call them `left = id - 1` and `right = id % N`. These are the raw indices.

Before grabbing anything, we sort them:
```c
if (left < right) { first = left;  second = right; }
else              { first = right; second = left;  }
```

Now every coder always takes the **lower-index dongle first**. 

Why does this prevent circular wait? Consider 3 coders:
- Coder 1 wants dongles 0 and 1 → takes 0 first
- Coder 2 wants dongles 1 and 2 → takes 1 first
- Coder 3 wants dongles 2 and 0 → takes **0** first (sorted)

In the classic deadlock scenario, coder 3 would take dongle 2, leaving everyone stuck. With fixed ordering, coder 3 tries for dongle 0 first. Coder 1 already holds dongle 0, so coder 3 waits. Coder 1 eventually gets dongle 1 and compiles, releases both, and then coder 3 can proceed. The circle is broken.

**Edge case: single coder.** When N=1, `left = id-1 = 0` and `right = id % N = 0`. Both point to the same dongle. Without a fix, the coder would take dongle 0, then try to take dongle 0 again — deadlock against itself. The fix: detect N=1 and skip the second `dongle_take`.

---

## 6. The Priority Queue — Fair Scheduling

This is the main new mechanism compared to classic solutions.

A condition variable's `pthread_cond_broadcast` wakes **all** waiting threads. They all rush to grab the mutex, and whichever thread the OS scheduler picks wins. This is random — neither FIFO nor EDF.

To implement ordered scheduling, every dongle maintains a **min-heap** of waiting coders.

### What is a min-heap?

A min-heap is a binary tree stored as an array where the **smallest element is always at index 0** (the root). Every parent is smaller than its children. Operations:

- **Push** (insert): add to the end, then "sift up" — repeatedly swap with parent until the parent is smaller. O(log n).
- **Pop** (remove root): swap root with last element, shrink array, then "sift down" — repeatedly swap with the smaller child until both children are larger. O(log n).
- **Peek**: read index 0. O(1).
- **Remove arbitrary element**: find it, replace with last element, then sift both up and down (it might need to go either direction). O(n) find + O(log n) sift.

### The comparison function

```c
static int pq_wins(t_waiter a, t_waiter b) {
    if (a.priority != b.priority)
        return (a.priority < b.priority);
    return (a.coder_id < b.coder_id);
}
```

`a` wins over `b` if `a` has a lower priority value. If equal (rare but possible in EDF), the lower coder ID wins — making the tie-breaking deterministic and reproducible.

### How the heap is used

When a coder wants a dongle, it calls `dongle_take`:
1. Compute its priority (arrival time for FIFO, deadline for EDF)
2. Lock the dongle's mutex
3. `pq_push` itself into the dongle's heap
4. Enter a wait loop

The wait loop condition: "am I at the top of the heap AND is the dongle free AND has the cooldown passed?"

If yes → take the dongle (pop yourself from heap, set held=1, unlock, return).
If I'm at the top but cooldown hasn't passed yet → `pthread_cond_timedwait` until the cooldown expires.
Otherwise → `pthread_cond_wait` — someone else is ahead of me or the dongle is held.

When a coder releases a dongle, `dongle_release`:
1. Set held=0
2. Set new cooldown deadline
3. `pq_peek` to find who is next in queue
4. `pthread_cond_signal` that specific coder's `wait_cond`

Only one coder is woken. No thundering herd. The winner is determined by the heap ordering.

---

## 7. The Two Scheduling Modes

### FIFO — First In, First Out

Priority = `get_time_ms()` at the moment the request is made.

The coder who asked for the dongle earliest has the lowest timestamp, wins the heap, gets the dongle first. Pure arrival order. Fair, simple, predictable.

### EDF — Earliest Deadline First

Priority = `last_compile_start_ms + time_to_burnout`

This is the timestamp at which the coder will burn out if it doesn't compile again. The coder **closest to burning out** has the earliest deadline, wins the heap, compiles first.

EDF is a real-time scheduling algorithm used in operating systems for hard real-time tasks. It is **optimal** in the sense that if any schedule can meet all deadlines, EDF can too. In this context, it ensures that the most at-risk coder always gets priority — reducing burnouts compared to FIFO when timing is tight.

**Why EDF is better under pressure:**
Imagine coder A has 50ms left before burnout and coder B has 400ms left. FIFO might give the dongle to B if B asked first. EDF always gives it to A. A is then safe. B still has 400ms — plenty of time.

---

## 8. Condition Variables — The Waiting Mechanism

A condition variable (`pthread_cond_t`) is a synchronization primitive that lets a thread sleep efficiently until a condition becomes true. It is always paired with a mutex.

The basic pattern:
```c
pthread_mutex_lock(&mutex);
while (!condition_is_true)
    pthread_cond_wait(&condvar, &mutex);
// condition is now true, proceed
pthread_mutex_unlock(&mutex);
```

`pthread_cond_wait` atomically: releases the mutex AND puts the thread to sleep. When signaled, it atomically: re-acquires the mutex AND returns. The `while` (not `if`) is essential because of **spurious wakeups** — a thread can wake up even without being signaled. Always re-check the condition.

### Why per-coder condition variables?

The standard approach (one condvar per dongle, broadcast on release) has a problem: **the thundering herd**. If 4 coders are waiting for dongle 3 and it gets released, all 4 wake up. Three of them re-check their condition, find they lost the race, and go back to sleep. This is wasted work that scales badly.

With per-coder condvars:
- Only the winner (heap root) gets signaled
- Everyone else keeps sleeping
- No spurious competitions, no wasted wakeups

The trick: a coder waits on its own `wait_cond` but paired with the **dongle's mutex**:
```c
pthread_cond_wait(&coder->wait_cond, &dongle->mutex);
```
This is valid — a condvar doesn't belong to a specific mutex. The coder holds `dongle->mutex` when it enters the wait, and `pthread_cond_wait` releases it atomically. When signaled, the mutex is re-acquired before returning.

### `pthread_cond_timedwait` — Waking at cooldown expiry

When a coder is the next in queue (heap head) but the dongle is still in cooldown:
```c
ts.tv_sec = d->cooldown_until_ms / 1000;
ts.tv_nsec = (d->cooldown_until_ms % 1000) * 1000000L;
pthread_cond_timedwait(&coder->wait_cond, &d->mutex, &ts);
```

The thread sleeps until either:
- It gets signaled (someone else released the dongle or simulation stopped)
- The absolute timeout `ts` expires (the cooldown has passed)

After waking, it re-checks all conditions and takes the dongle if everything is clear.

---

## 9. The Cooldown Mechanism

When a coder releases a dongle, the dongle enters a cooldown period — it cannot be taken for `dongle_cooldown` milliseconds. This models a real-world scenario where hardware needs time to reset after use.

```c
dongle->held = 0;
dongle->cooldown_until_ms = get_time_ms() + sim->config.dongle_cooldown;
```

The scheduler at the head of the queue will wait for this deadline using `pthread_cond_timedwait`. Nobody barges past the cooldown — the heap check includes `now >= cooldown_until_ms`.

The cooldown also implicitly adds delay between consecutive uses by the same coder — which helps other coders get a turn even without explicit fairness enforcement.

---

## 10. The Monitor Thread — Burnout Detection

The monitor is a separate thread that runs in parallel with all coder threads. It is the "referee" of the simulation.

Every 1 millisecond it:
1. Reads `last_compile_start_ms` for each coder (under `coder->m` to avoid races)
2. Computes `now - last_compile_start_ms`
3. If this exceeds `time_to_burnout` → prints "burned out", triggers stop
4. Checks if all coders have compiled `required_compiles` times → triggers stop

The **10ms burnout precision** requirement is met because:
- The monitor polls every 1ms
- The burnout log is printed immediately when detected (before doing anything else)
- The gap between actual burnout and detection is at most the polling interval

When stop is triggered:
1. `sim_set_stop` sets the stop flag (mutex-protected)
2. `sim_wake_all` signals every coder's `wait_cond` — even if they're sleeping in a `pthread_cond_wait`, they wake up, re-check `sim_should_stop`, and exit

Coders that are currently in `usleep` (debugging or refactoring) are NOT interrupted — `usleep` doesn't respond to signals. They finish their sleep, return to the top of the while loop, check `sim_should_stop`, and exit cleanly. This causes the small delay between the burnout message and the program actually terminating.

---

## 11. The Stop Flag — Clean Shutdown

```c
// In t_sim:
int             stop;
pthread_mutex_t stop_m;
```

Every thread checks `sim_should_stop` frequently — at the top of their main loop and inside the dongle wait loop. The stop flag is protected by `stop_m` so that reads and writes across threads are always consistent.

Why a mutex and not `volatile int`? In C, `volatile` only prevents compiler optimizations — it does not prevent CPU reordering or cache incoherence on multi-core systems. A mutex provides a **memory barrier**, guaranteeing that after `pthread_mutex_unlock`, the updated value is visible to all threads that subsequently acquire the same mutex.

---

## 12. Log Serialization

Every state log goes through:
```c
void log_state(t_sim *sim, int coder_id, const char *message) {
    long timestamp = get_time_ms() - sim->start_ms;
    pthread_mutex_lock(&sim->log_m);
    printf("%ld %d %s\n", timestamp, coder_id, message);
    pthread_mutex_unlock(&sim->log_m);
}
```

Without `log_m`, two threads could interleave their `printf` calls, producing garbled output like:
```
200 1 is 201 2 is compilingcompiling
```

The mutex ensures one complete line is printed before the next thread gets access.

---

## 13. Memory Layout and Lifecycle

**Initialization** (`init_simulator.c`):
1. `memset` the entire `t_sim` to zero
2. Init `stop_m` and `log_m`
3. `malloc` the `t_coder` array, init each: mutex + wait_cond + starting values
4. `malloc` the `t_dongle` array, init each: mutex + `malloc` the queue array
5. If any step fails, `sim_init_cleanup` unwinds everything already initialized

**Teardown** (`main.c`):
1. `pthread_join` all coder threads
2. `pthread_join` monitor thread
3. `sim_destroy`: destroy all wait_conds and coder mutexes, free coders array, free each dongle's queue, destroy dongle mutexes, free dongles array, destroy log_m and stop_m

The teardown order matters: threads must be joined before destroying the resources they use. Joining ensures the thread has fully returned from its routine before any cleanup happens.

---

## 14. The Complete Flow — Start to Finish

```
argv parsed → simulation initialized → N coder threads created → monitor thread created

Each coder thread (parallel):
    compute dongle indices (first, second) with fixed ordering
    while not stopped AND compile_count < required:
        dongle_take(first)   → push into heap, wait until head + free + cooldown
        dongle_take(second)  → same
        record last_compile_start_ms = now
        log "is compiling", sleep time_to_compile
        dongle_release(first), dongle_release(second)
        compile_count++
        log "is debugging", sleep time_to_debug
        log "is refactoring", sleep time_to_refactor
    thread exits

Monitor thread (parallel):
    every 1ms:
        check each coder: if now - last_compile_start_ms > time_to_burnout:
            log "burned out"
            set stop flag
            wake all coders
            exit
        if all coders have reached required_compiles:
            set stop flag
            wake all coders
            exit

Main thread:
    join all coder threads
    join monitor thread
    sim_destroy (free everything)
    exit
```

---

## 15. Summary — What We Actually Built

| Concept | Implementation |
|---------|---------------|
| Thread per coder | `pthread_create` with `coder_routine` |
| Deadlock prevention | Fixed lower-index-first dongle ordering |
| Fair scheduling | Min-heap priority queue inside each dongle |
| FIFO policy | Priority = arrival timestamp |
| EDF policy | Priority = last_compile_start + time_to_burnout |
| EDF tie-breaking | Lower coder ID wins when deadlines are equal |
| Targeted wakeup | Per-coder `wait_cond` signaled individually |
| Cooldown enforcement | `pthread_cond_timedwait` with absolute deadline |
| Burnout detection | Monitor polls every 1ms, logs within 10ms |
| Clean shutdown | Stop flag + `sim_wake_all` signals all sleeping coders |
| Log serialization | Shared `log_m` mutex around all `printf` calls |
| Memory safety | Paired malloc/free, init/destroy for all resources |
| Single-coder fix | Skip second dongle_take when N=1 |
