#pragma once


#include <string>
#include <vector>
#include <stdexcept>
#include <cstdint>


// === Nice-to-weight Table ===
// Exact values from the Linux Kernel (sched/core.c)
// Nice range: -20 (highest prority) to +19 (lowest)
// Each step is ~25% more or less weight than the previous.
// Baseline: nice 0 -> weight 1024

// weight[nice] = 1024 × (1/1.25)^nice
// = 1024 × (0.8)^nice
static const int NICE_TO_WEIGHT[40] = {
    /* -20 */ 88761, 71755, 56483, 46273, 36291,
    /* -15 */ 29154, 23254, 18705, 14949, 11916,
    /* -10 */ 9548,  7620,  6100,  4904,  3906,
    /*  -5 */ 3121,  2501,  1991,  1586,  1277,
    /*   0 */ 1024,  820,   655,   526,   423,
    /*   5 */ 335,   272,   215,   172,   137,
    /*  10 */ 110,   87,    70,    56,    45,
    /*  15 */ 36,    29,    23,    18,    15};

// convert a nice value [-20, -19] to its kernel weight
inline int nice_to_weight(int nice) {
  if (nice < -20 || nice > 19)
    throw std::out_of_range("Nice value must be in [-20, 19]");
  return NICE_TO_WEIGHT[nice + 20];
}

//  === Job Profile (Scheduler Input) ===
//  This is what the "user" of the scheduler provides.
//  burst_pattern alternates: [cpu, io, cpu, io, ...]
//  The job executes cpu ticks, then blocks for io ticks,
//  then executes again, etc., until total_work is consumed.
struct JobProfile {
  std::string name;
  int nice;                       // priority: -20 (high) to +19 (low)
  int total_work;                 // total CPU ticks until completion
  std::vector<int> burst_pattern; // alternating [cpu_burst, io_burst, ...]

  JobProfile(std::string n, int nc, int tw, std::vector<int> bp)
      : name(n), nice(nc), total_work(tw), burst_pattern(bp) {
    if (nice < -20 || nice > 19)
      throw std::invalid_argument(name + ": nice must be in [-20, 19]");
    if (total_work <= 0)
      throw std::invalid_argument(name + ": total_work must be > 0");
    if (burst_pattern.empty())
      throw std::invalid_argument(name + ": burst_pattern cannot be empty");
    for (int b : burst_pattern) {
      if (b <= 0)
        throw std::invalid_argument(name + ": all burst values must be > 0");
    }
  }
};


// === Job State Machine ===
//  A Job is the runtime object created from a JobProfile.
//  States:
//    RUNNABLE  — in the run queue, waiting for CPU
//    RUNNING   — currently on the CPU
//    SLEEPING  — blocked on I/O, in the sleep queue
//    DONE      — completed all work

enum class JobState { RUNNABLE, RUNNING, SLEEPING, DONE };

inline const char *state_str(JobState s) {
  switch (s) {
  case JobState::RUNNABLE:
    return "RUNNABLE";
  case JobState::RUNNING:
    return "RUNNING";
  case JobState::SLEEPING:
    return "SLEEPING";
  case JobState::DONE:
    return "DONE";
  }
  return "UNKNOWN";
}



// ─────────────────────────────────────────────
//  Job  — runtime object
// ─────────────────────────────────────────────

struct Job {
    // Identity
    std::string name;
    int         nice;
    int         weight;

    // CFS bookkeeping
    uint64_t    vruntime    = 0;
    int         time_on_cpu = 0;

    // Work tracking
    int         total_work;
    int         work_done   = 0;

    // Burst pattern state machine
    std::vector<int> burst_pattern;
    int  burst_index    = 0;
    int  ticks_in_burst = 0;

    // State
    JobState    state     = JobState::RUNNABLE;
    int         wake_tick = 0;

    // Statistics
    int         arrival_tick    = 0;
    int         completion_tick = 0;
    int         total_wait      = 0;
    int         total_sleep     = 0;

    explicit Job(const JobProfile& p, int arrival = 0)
        : name(p.name)
        , nice(p.nice)
        , weight(nice_to_weight(p.nice))
        , total_work(p.total_work)
        , burst_pattern(p.burst_pattern)
        , arrival_tick(arrival)
    {}

    // ── Burst helpers ──────────────────────────
    // True when we've exhausted the current burst phase
    bool burst_phase_complete() const {
        return ticks_in_burst >= burst_pattern[burst_index];
    }
    // Are we currently in a CPU phase (even indices) or I/O phase (odd) ?
    bool in_cpu_phase() const { return (burst_index % 2) == 0; }

    // Advance to the next burst phase; wraps around if pattern repeats
    void advance_burst_phase() {
        burst_index = (burst_index + 1) % static_cast<int>(burst_pattern.size());
        ticks_in_burst = 0;
    }

    // ── Core tick ─────────────────────────────
    // Returns true if the job just finished all its work
    // Called each tick the job spends on CPU
    // Returns true if the job just completed all its work
    bool do_cpu_tick(uint64_t actual_tick_ns = 1) {
      // vruntime grows inversely proportional to weight:
      // delta_vruntime = (actual_time x NICE_0_WEIGHT) / weight
      // using NICE_0_WEIGHT = 1024 as the normalization constant
      constexpr int NICE_0_WEIGHT = 1024;
      vruntime += (actual_tick_ns * NICE_0_WEIGHT) / weight;
      time_on_cpu++;
      ticks_in_burst++;
      work_done++;
      return work_done >= total_work;
    }

    // ── Reporting ─────────────────────────────
    // CPU share this job should receive (for reporting)
    double ideal_cpu_share(int total_weight) const {
        return static_cast<double>(weight) / total_weight;
    }

    void print_summary() const;
};
