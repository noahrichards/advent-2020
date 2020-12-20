#include <execution>
#include <fstream>
#include <numeric>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/str_split.h"
#include "absl/strings/strip.h"
#include "absl/strings/substitute.h"
#include "absl/types/optional.h"
#include "glog/logging.h"

int DistanceToLast(const absl::flat_hash_map<int64_t, int64_t>& last_seen,
                   int64_t value, int index) {
  auto it = last_seen.find(value);
  if (it == last_seen.end()) {
    return 0;
  }
  return index - it->second;
}

int main(int argc, char** argv) {
  google::InstallFailureSignalHandler();
  google::InitGoogleLogging(argv[0]);
  FLAGS_logtostderr = 1;

  absl::flat_hash_map<int64_t, int64_t> last_seen;

  std::ifstream file(argv[1]);
  CHECK(file);
  std::string line;
  CHECK(std::getline(file, line));

  int index = 0;
  int64_t last_value = -1;
  for (auto input : absl::StrSplit(line, ",")) {
    int64_t number;
    CHECK(absl::SimpleAtoi(input, &number));
    if (last_value >= 0) {
      last_seen[last_value] = index;
    }
    last_value = number;
    ++index;
  }
  // We start by looking at the *last* number, so clear it out of the map.
  last_seen.erase(last_value);

  for (; index <= 29999999; ++index) {
    int64_t next = DistanceToLast(last_seen, last_value, index);
    if (index == 2019) {
      LOG(INFO) << "PART 1: " << next;
    }
    last_seen[last_value] = index;
    last_value = next;
  }

  LOG(INFO) << "PART 2: " << last_value;
  return 0;
}
