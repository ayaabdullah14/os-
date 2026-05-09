# ENCS3390 / ENCS4110 – Operating System Concepts
## Project #1: Word Frequency Analysis — Naive vs Multiprocessing vs Multithreading

## Overview

This project compares three approaches for computing word frequencies from a large text dataset:

| Approach | Best Time (s) | Best Speedup | Best Throughput |
|---|---|---|---|
| Naive (sequential) | 917.1 | 1.00 | 0.00109 |
| Multiprocessing | 313.683 *(6 procs)* | 2.92 | 0.00319 |
| Multithreading | 321.578 *(4 threads)* | 2.85 | 0.00311 |

**Winner: Multiprocessing with 6 processes.**

---

## Project Structure

```
project/
├── naive.c              # Sequential word frequency counter
├── multiprocess.c       # Fork-based parallel word frequency counter
├── multithread.c        # Pthreads-based parallel word frequency counter
├── data.txt             # Input dataset (required)
└── frequencies_N.txt    # Temporary per-process output files (auto-generated)
```

---

## How It Works

### Data Structure
All three approaches share a common `WordFreq` struct:
```c
#define MAX_WORD_LENGTH 50
typedef struct {
    char word[MAX_WORD_LENGTH];
    int  frequency;
} WordFreq;
```

Words are read from `data.txt` into a dynamically allocated array that doubles in size when full.

### 1. Naive Approach
- Reads all words sequentially
- For each word, performs a linear search through the frequency array
- Time complexity: **O(n × m)** where n = total words, m = unique words
- Sorts results with `qsort` — O(m log m)
- Baseline execution time: **917.1 seconds**

### 2. Multiprocessing Approach
- Uses `fork()` to create N child processes
- Each child handles an equal slice of the word array
- Child writes its local frequencies to a temp file (`frequencies_0.txt`, `frequencies_1.txt`, …)
- Parent calls `wait()` to synchronize, then `merge_frequencies()` to combine all files
- Final results sorted with `qsort()`
- Best configuration: **6 processes → 313.683 s, speedup 2.92**

### 3. Multithreading Approach
- Uses **POSIX Threads** (`pthread_create`, `pthread_join`)
- Each thread processes a chunk of the word array locally
- A **semaphore** (`sem_wait` / `sem_post`) protects the shared global frequency array during merging
- Memory resized dynamically with `realloc()`
- Timing measured with `gettimeofday()`
- Best configuration: **4 threads → 321.578 s, speedup 2.85**

---

## How to Compile and Run

```bash
# Naive
gcc -Wall -O2 -o naive naive.c
./naive

# Multiprocessing (set NUM_PROCESSES in source or pass as argument)
gcc -Wall -O2 -o multiprocess multiprocess.c
./multiprocess

# Multithreading
gcc -Wall -O2 -o multithread multithread.c -lpthread
./multithread
```

Make sure `data.txt` is in the same directory before running.

---

## Performance Results

### Execution Time & Speedup

| Configuration | Time (s) | Speedup | Throughput (1/T) |
|---|---|---|---|
| Naive | 917.1 | 1.00 | 0.00109 |
| Multiprocess — 2 | 572.57 | 1.60 | 0.00175 |
| Multiprocess — 4 | 352.085 | 2.60 | 0.00284 |
| **Multiprocess — 6** | **313.683** | **2.92** | **0.00319** |
| Multiprocess — 8 | 328.642 | 2.79 | 0.00304 |
| Multithread — 2 | 384.050 | 2.39 | 0.00260 |
| **Multithread — 4** | **321.578** | **2.85** | **0.00311** |
| Multithread — 6 | 348.461 | 2.63 | 0.00287 |
| Multithread — 8 | 438.628 | 2.09 | 0.00228 |

### Amdahl's Law Analysis

- **Serial portion (S):** 0.4% (0.004)
- **Parallel portion:** 99.6%
- **Maximum theoretical speedup (6 cores):**

```
Speedup_max = 1 / (S + (1 - S) / N)
            = 1 / (0.004 + 0.996 / 6)
            ≈ 5.88
```

Actual speedup reached ~2.92, well below the theoretical max of 5.88, due to process/thread creation overhead, inter-process communication, and synchronization costs.

### Optimal Configurations

- **Multiprocessing:** 6 processes — matches the 6 physical CPU cores. Beyond 6, context switching overhead outweighs parallelism gains.
- **Multithreading:** 4 threads — despite having 6 CPU threads, overhead from semaphore contention during the merge phase degrades performance at 6+ threads.

---

## Environment

| Component | Details |
|---|---|
| CPU | Intel Core i7-1255U (12th Gen), 6 cores / 6 threads, ~2.7 GHz base |
| RAM | 4.8 GB total (1.0 GB used, 2.2 GB free) |
| Swap | 2.6 GB (unused) |
| OS | Ubuntu 22.04.2 LTS (running in Oracle VirtualBox) |
| Language | C |
| IDE | Code::Blocks |

---

## Key Takeaways

- **Multiprocessing** gives the best speedup because each process runs independently on its own core with no shared-memory contention.
- **Multithreading** is faster to spin up but suffers from synchronization overhead at the merge step; optimal at 4 threads.
- **Naive** is the simplest but unacceptable for large datasets — nearly 3× slower than the best parallel approach.
- Both parallel approaches hit diminishing returns once process/thread count exceeds physical core count, consistent with Amdahl's Law.

---

## References

- Amdahl, G. M. (1967). Validity of the single processor approach to achieving large-scale computing capabilities.
- POSIX Threads Programming — `pthread_create`, `pthread_join`, `sem_init`
- Linux `fork()`, `wait()` system calls
