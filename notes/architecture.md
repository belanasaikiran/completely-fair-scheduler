Each job will be described by a profile that drives how it behaves at runtime:

```cpp
struct JobProfile {
    std::string name;
    int         nice;          // -20 to +19 → maps to weight
    int         total_work;    // total CPU ticks needed to complete
    // Burst pattern: alternating [cpu_burst, io_burst, cpu_burst, ...]
    std::vector<int> burst_pattern;
}; 
```

## Example Profiles:

```cpp
// CPU-hungry background job (low priority)
{ "compiler",  19,  120, {40, 5, 40, 5, 40} }

// Interactive shell (high priority, short bursts)
{ "shell",    -10,   30, {5, 20, 5, 20, 5, 20} }

// Normal task
{ "server",    0,   80, {20, 10, 20, 10, 20} }
```

This is the key insight: **priority alone doesn't determine behavior**. The *burst pattern* determines whether a job is I/O-bound or CPU-bound, which in turn affects how its `vruntime` accumulates and how often it gets preempted.

---

## Architecture
```
┌─────────────────────────────────────────────┐
│               Scheduler                      │
│                                              │
│  run_queue: std::map<u64, Job*>  ← RB-Tree  │
│  sleep_queue: list of (Job*, wake_tick)      │
│  current_job: Job*                           │
│  min_vruntime: u64                           │
│                                              │
│  tick()                                      │
│   ├── wake sleeping jobs if timer expired    │
│   ├── update current job's vruntime          │
│   ├── check if current job should block      │
│   ├── check preemption                       │
│   └── pick_next() if needed                  │
└─────────────────────────────────────────────┘
```




