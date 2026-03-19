#include "core.hpp"
#include <cstdio>
#include <iostream>
#include <string>

int main() {
    std::cout << std::string(65, '*') << "\n";
    std::cout << std::string(25, '*') << " Job Profiles " << std::string(26, '*') << "\n";
    std::cout << std::string(65, '*') << "\n";

  std::vector<JobProfile> profiles = {
      // name      nice   total_work  burst_pattern
      {"shell", -10, 30, {5, 20, 5, 20, 5}},
      {"server", 0, 80, {20, 10, 20, 10, 20}},
      {"compiler", 19, 120, {40, 5, 40, 5, 40}},
      {"monitor", -5, 50, {3, 15, 3, 15, 3, 15}},
  };

  // computer total weight for ideal share calculation
  int total_weight = 0;
  for (auto &p : profiles)
    total_weight += nice_to_weight(p.nice);

  std::cout << std::string(65, '-') << "\n";
  printf("%-12s %6s %8s %10s %12s\n", "Job", "Nice", "Weight", "Ideal%",
         "Burst Pattern");
  std::cout << std::string(65, '-') << "\n";

  for (auto &p : profiles) {
    Job j(p);
    double share = j.ideal_cpu_share(total_weight) * 100.0;
    printf("%-12s %6d %8d %9.2f%%  [", p.name.c_str(), p.nice, j.weight, share);
    for (int i = 0; i < (int)p.burst_pattern.size(); i++) {
      printf("%d%s", p.burst_pattern[i],
             i + 1 < (int)p.burst_pattern.size() ? "," : "");
    }
    printf("]\n");
  }
  std::cout << std::string(65, '-') << "\n";
  printf("%-12s %6s %8d %9.2f%%\n", "TOTAL", "", total_weight, 100.0);
}
