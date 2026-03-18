# completely-fair-scheduler

A completely fair scheduler (CFS) aims to give every job/process an equal share of CPU time proportional to its weight (priority). The key idea here is:

**If you have N jobs of equal priority, each should get exactly 1/N of the CPU. No job should starve, and no job should hog the CPU unfairly.**

> Linux's own scheduler (from kernel 2.6.23) is based on this model.

[Concepts](./notes/concepts.md)

This app includes:

- A `Job` struct with `vruntime`, `weight`, `nice` and burst 10:41- A Red-Black Tree 
- A scheduler class with `add_job()`, `pick_next()`, `run_tick()`
- A simulation loop that runs jobs and prints a timeline.


