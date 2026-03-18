#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

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


struct Job {

}
