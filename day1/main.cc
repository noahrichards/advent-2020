#include <fstream>

#include "absl/container/flat_hash_set.h"
#include "absl/strings/substitute.h"
#include "glog/logging.h"

constexpr int kTotal = 2020;

void DoPart1(const absl::flat_hash_set<int>& numbers) {
  for (int i : numbers) {
    int other = kTotal - i;
    if (numbers.count(other) > 0) {
      LOG(INFO) << absl::Substitute("PART1: Found: $0 and $1, mult: $2", i,
                                    other, i * other);
      return;
    }
  }

  LOG(ERROR) << "PART1: NOT FOUND";
}

void DoPart2(const absl::flat_hash_set<int>& numbers) {
  for (int i : numbers) {
    for (int j : numbers) {
      if (i == j) continue;
      int other = kTotal - i - j;
      if (numbers.count(other) > 0) {
        LOG(INFO) << absl::Substitute("PART2: Found: $0/$1/$2, mult: $3", i, j,
                                      other, i * j * other);
        return;
      }
    }
  }
}

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();
  FLAGS_logtostderr = 1;

  absl::flat_hash_set<int> numbers;
  std::ifstream file(argv[1]);
  CHECK(file);

  int in;
  while (file >> in) {
    numbers.insert(in);
  }

  DoPart1(numbers);
  DoPart2(numbers);
  return 0;
}
