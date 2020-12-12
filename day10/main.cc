#include <fstream>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_split.h"
#include "absl/strings/strip.h"
#include "absl/strings/substitute.h"
#include "absl/types/optional.h"
#include "glog/logging.h"

typedef std::vector<int> Adapters;
typedef absl::flat_hash_set<int> AdapterSet;

absl::flat_hash_map<int, int64_t> memo;

int64_t CountPathsLinear(const Adapters& adapters) {
  int max = adapters[adapters.size() - 1];
  std::vector<int64_t> path_counts(max + 3, 0);

  // This is basically generating the memo in reverse. You know the very last
  // adapter has only one path, and then you walk backwards through adapters and
  // generate the number of paths by adding up the valid paths it can reach.
  //
  // If you observe how the memoized recursive function works in practice, it's
  // sorta doing the same thing: calculating paths at the end, tail-recursing
  // (almost) those values towards the start of the array.
  path_counts[max] = 1;
  for (int i = adapters.size() - 2; i >= 0; --i) {
    int adapter = adapters[i];
    path_counts[adapter] = path_counts[adapter + 1] + path_counts[adapter + 2] +
                           path_counts[adapter + 3];
  }
  // So the total is what's reachable from 0, which is 1 + 2 + 3.
  return path_counts[1] + path_counts[2] + path_counts[3];
}

int64_t CountPaths(AdapterSet adapters, int start, int end) {
  if (start == end) {
    return 1;
  }
  auto known = memo.find(start);
  if (known != memo.end()) return known->second;

  int64_t total = 0;
  // Look for valid jumps.
  for (int i = 1; i <= 3; ++i) {
    if (adapters.contains(start + i)) {
      total += CountPaths(adapters, start + i, end);
    }
  }
  memo[start] = total;
  return total;
}

int main(int argc, char** argv) {
  google::InstallFailureSignalHandler();
  google::InitGoogleLogging(argv[0]);
  FLAGS_logtostderr = 1;

  std::ifstream file(argv[1]);
  CHECK(file);

  Adapters adapters;
  std::string line;
  while (std::getline(file, line)) {
    int number;
    CHECK(absl::SimpleAtoi(line, &number));
    adapters.push_back(number);
  }
  std::sort(adapters.begin(), adapters.end());

  // For ease of doing diffs[delta] instead of storing ones and threes.
  std::array<int, 4> diffs = {0, 0, 0, 0};
  int last_joltage = 0;
  for (int adapter : adapters) {
    CHECK(adapter - last_joltage < 4);
    ++diffs[adapter - last_joltage];
    last_joltage = adapter;
  }
  ++diffs[3];

  // Make sure there were no 0 or 2 gaps.
  CHECK_EQ(diffs[0], 0);
  CHECK_EQ(diffs[2], 0);

  LOG(INFO) << "PART 1: " << (diffs[1] * diffs[3]);
  // Find the permutations.
  AdapterSet adapter_set(adapters.begin(), adapters.end());
  LOG(INFO) << "PART 2: "
            << CountPaths(adapter_set, 0, adapters[adapters.size() - 1]);
  LOG(INFO) << "PART 2 linear: " << CountPathsLinear(adapters);
  return 0;
}
