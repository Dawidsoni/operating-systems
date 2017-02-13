# Operating systems

## Description

Project contains my solutions to operating systems tasks.

Some synchronization problems:
- creating semaphore from mutex
- dining philosophers problem
- dining savages problem
- search-insert-delete problem
- cigarette smokers problem

Some Linux processes tasks.

Some code-optimization tasks:

- matrix transposition with and without blocking

```
./transpose -n 13 -v 0
time: about 1.47s.
./transpose -n 13 -v 1
time: about 0.27s.
```


- binary search vs. heap search

```
./bsearch -n 22 -t 23 -v 0
time: about 5.20s.
./bsearch -n 22 -t 23 -v 1
time: about 2.10s.
```

- random walk optimized to make branch predictor less miserable

```
./randwalk -n 7 -s 15 -t 14 -v 0
time: about 4.8s.
./randwalk -n 7 -s 15 -t 14 -v 1
time: about 3.2s.
```

- measuring cache line size and L1, L2, L3 cache sizes walking through array permutations

```
./cache -n 26 -s 26 -t 20
time about 3.1s.
```
