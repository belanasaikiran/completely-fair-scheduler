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
