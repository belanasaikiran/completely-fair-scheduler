## Core Concepts

### 1. Virtual Runtime (vruntime)

This is the heart of CFS. Instead of tracking actual wall-clock time, each job tracks a virtual runtime — how much CPU time it has logically consumed, weighted by its priority.

```cpp
vruntime += actual_runtime_spent / weight
```

- A high-priority job has a larger weight, so its vruntime grows slower → it gets picked more often.
- A low-priority job has a smaller weight, so its vruntime grows faster → it gets picked less often.
- The scheduler always picks the job with the smallest vruntime next.

### 2. The Red-Black Tree

Jobs are stored in a self-balancing BST (Red-Black Tree), ordered by vruntime. This gives:

- O(log N) insertion and removal
- O(1) access to the minimum vruntime job (leftmost node)

This is exactly how Linux CFS works internally.


### 3. Weight & Nice Values

Priority is expressed as a nice value (typically -20 to +19). This maps to a weight using a precomputed table:

```
nice 0  → weight 1024   (baseline)
nice -1 → weight 1277   (~25% more)
nice +1 → weight 820    (~25% less)
```

**The Formula**

Each nice level is ~1.25× heavier than the one above it (or ~0.8× lighter). The multiplier is exactly:

weight[nice] = 1024 * (1/1.25)^nice
             = 1024 * (0.8)^nice


Starting from the baseline: nice 0 → 1024, you multiply or divide by ~1.25 for each step.

Each step of nice value changes weight by ~25%. The vruntime formula then becomes:

```cpp
vruntime += (actual_time × 1024) / weight
```
### 4. Time Slice (Scheduler Period)

CFS doesn't assign fixed time slices. Instead it defines a scheduler period (e.g. 6ms), and divides it proportionally:

```cpp
time_slice_for_job = (weight / total_weight) × scheduler_period
```
This ensures proportional CPU sharing within each period.


### 5. New Job Bootstrapping

When a new job enters the queue, it shouldn't start at `vruntime = 0` (that would let it monopolize the CPU). Instead, it inherits the minimum vruntime currently in the tree, so it starts fairly alongside existing jobs.

The Scheduling Loop

```
loop:
  1. Pick job with minimum vruntime from the red-black tree
  2. Run it for its time slice
  3. Update its vruntime
  4. Re-insert it into the red-black tree
  5. Handle any newly arrived jobs (insert them at min_vruntime)
```
