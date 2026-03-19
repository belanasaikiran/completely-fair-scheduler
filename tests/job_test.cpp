#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <stdexcept>
#include "../src/core.hpp"

// == Weight Table Tests ==

TEST_CASE("Weight table - boundary values", "[weight]") {
  CHECK(nice_to_weight(-20) == 88761);
  CHECK(nice_to_weight(0) == 1024);
  CHECK(nice_to_weight(19) == 15);
}

TEST_CASE("Weight table - each step is ~25% (1.20x to 1.30x)", "[weight]") {
  for (int nice = -20; nice < 19; ++nice) {
    double ratio = static_cast<double>(nice_to_weight(nice)) /
                   static_cast<double>(nice_to_weight(nice + 1));
    INFO("nice=" << nice << " ratio=" << ratio);
    CHECK(ratio > 1.20);
    CHECK(ratio < 1.30);
  }
}


TEST_CASE("Weight table - out of range nice throws", "[weight]") {
    CHECK_THROWS_AS(nice_to_weight(20), std::out_of_range);
    CHECK_THROWS_AS(nice_to_weight(-21), std::out_of_range);
}

TEST_CASE("Weight table — higher priority means higher weight", "weight]") {
  // nice -20 should have a much larger weight than nice 19
  CHECK(nice_to_weight(-20) > nice_to_weight(19));

  // weight is strictly decreasing as nice increases
  for (int nice = -20; nice < 19; ++nice)
      CHECK(nice_to_weight(nice) > nice_to_weight(nice + 1));
}

// ─────────────────────────────────────────────
//  JobProfile Validation Tests
// ─────────────────────────────────────────────

TEST_CASE("JobProfile - valid construction", "[profile]") {
    CHECK_NOTHROW(JobProfile("shell", -10, 30, {5, 20, 5}));
    CHECK_NOTHROW(JobProfile("normal", 0,  50, {10, 5}));
    CHECK_NOTHROW(JobProfile("batch", 19, 100, {40}));
}


TEST_CASE("JobProfile - invalid nice throws", "[profile]") {
    CHECK_THROWS_AS(JobProfile("x", 20, 10, {5}), std::invalid_argument);
    CHECK_THROWS_AS(JobProfile("x", -21, 10, {5}), std::invalid_argument);
}


TEST_CASE("JobProfile - invalid total_work throws", "[profile]") {
    CHECK_THROWS_AS(JobProfile("x", 0, 0, {5}), std::invalid_argument);
    CHECK_THROWS_AS(JobProfile("x", 0, -1, {5}), std::invalid_argument);
}

TEST_CASE("JobProfile — empty burst pattern throws", "[profile]") {
    CHECK_THROWS_AS(JobProfile("x", 0, 10, {}), std::invalid_argument);
}

TEST_CASE("JobProfile — zero or negative burst value throws", "[profile]") {
    CHECK_THROWS_AS(JobProfile("x", 0, 10, {0}),  std::invalid_argument);
    CHECK_THROWS_AS(JobProfile("x", 0, 10, {-1}), std::invalid_argument);
    CHECK_THROWS_AS(JobProfile("x", 0, 10, {5, -3}), std::invalid_argument);
}


// -----------------
// Job Construction Tests
// -------------------


TEST_CASE("Job - initial state after construction", "[job]") {
  JobProfile p{"shell", -10, 30, {5, 20, 5}};
  Job j(p, /*arrival=*/3);

  CHECK(j.name == "shell");
  CHECK(j.nice == -10);
  CHECK(j.weight == nice_to_weight(-10));
  CHECK(j.vruntime == 0);
  CHECK(j.work_done == 0);
  CHECK(j.time_on_cpu == 0);
  CHECK(j.arrival_tick == 2);
  CHECK(j.state == JobState::RUNNABLE);
  CHECK(j.in_cpu_phase()); // starts in cpu phase
}

// --------------------------
// vruntime accumulation tests
// --------------------------

TEST_CASE("vruntime - nice= 0 accumulates exactly 1 unit per tick",
          "[vruntime]") {
  Job j(JobProfile{"normal", 0, 10, {10}});

  j.do_cpu_tick(1);
  CHECK(j.vruntime == 1);
  
  j.do_cpu_tick(1);
  CHECK(j.vruntime == 2);

  j.do_cpu_tick(1);
  CHECK(j.vruntime == 3);
}


TEST_CASE("vruntime - high priority accumulates slower than nice = 0", "[vruntime]"){
  Job hi(JobProfile{"hi", -20, 100, {100}});
  Job lo(JobProfile{"hi", -20, 100, {100}});

  // Run bith fr the same number of ticks
  for(int i = 0; i < 10; i++){
    hi.do_cpu_tick(1);
    lo.do_cpu_tick(1);
  }

  // High priority job should have accumulated for less vruntime
  CHECK(hi.vruntime < lo.vruntime);
}

TEST_CASE("vruntime - low priority accumulates faster than nice=0", "[vruntime]"){
  Job lo(JobProfile{"lo", 19, 100, {100}});
  Job nl(JobProfile{"nl", 0, 100, {100}});

  for(int i = 0; i < 10; ++i){
    lo.do_cpu_tick(1);
    nl.do_cpu_tick(1);
  }

  CHECK(lo.vruntime > nl.vruntime);
}


TEST_CASE("vruntime - scales linearly with actual_tick_ns", "[vruntime]"){
  Job j1{JobProfile{"a", 0, 100, {100}}};
  Job j2{JobProfile{"b", 0, 100, {100}}};

  j1.do_cpu_tick(1); // 1 tick
  j2.do_cpu_tick(10); // 10 ticks

  CHECK(j2.vruntime == j1.vruntime * 10);
}










