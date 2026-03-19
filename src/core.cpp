#include "core.hpp"
#include <cassert>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

void Job::print_summary() const {
  int turnaround = completion_tick - arrival_tick;
  std::cout << " [" << name << "]"
            << "  nice=" << nice << "  vruntime=" << vruntime
            << "  work=" << work_done << "/" << total_work
            << "  wait=" << total_wait << " ticks"
            << "  sleep=" << total_sleep << " ticks"
            << "  turnaround=" << turnaround << " ticks"
            << "  state=" << state_str(state) << "\n";
}

