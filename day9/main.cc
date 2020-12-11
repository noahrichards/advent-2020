#include <fstream>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_split.h"
#include "absl/strings/strip.h"
#include "absl/strings/substitute.h"
#include "absl/types/optional.h"
#include "glog/logging.h"

enum Opcode { kNop, kAcc, kJmp };

typedef std::vector<long> Numbers;

bool FindSum(Numbers numbers, long number, int start, int length) {
  absl::flat_hash_set<long> set;
  for (int i = start; i < start + length; ++i) {
    long new_number = numbers[i];
    if (set.count(number - new_number) > 0) return true;
    set.insert(new_number);
  }
  return false;
}

long FindBadSequence(Numbers numbers, long invalid_number) {
  for (int i = 0; i < numbers.size(); ++i) {
    long total = 0;
    long smallest = INT_MAX;
    long largest = INT_MIN;
    for (int j = i; j < numbers.size(); ++j) {
      long number = numbers[j];
      smallest = std::min(smallest, number);
      largest = std::max(largest, number);
      total += numbers[j];
      if (total == invalid_number) {
        return smallest + largest;
      }
      if (total > invalid_number) {
        break;
      }
    }
  }
  CHECK(false);
  return -1;
}

long FirstInvalid(Numbers numbers, int preamble) {
  for (int i = preamble; i < numbers.size(); ++i) {
    long number = numbers[i];
    if (!FindSum(numbers, number, i - preamble, preamble)) {
      return number;
    }
  }
  CHECK(false);
}

int main(int argc, char** argv) {
  google::InstallFailureSignalHandler();
  google::InitGoogleLogging(argv[0]);
  FLAGS_logtostderr = 1;

  std::ifstream file(argv[1]);
  CHECK(file);

  Numbers numbers;
  std::string line;
  while (std::getline(file, line)) {
    long number;
    CHECK(absl::SimpleAtoi(line, &number));
    numbers.push_back(number);
  }

  long first_invalid = FirstInvalid(numbers, 25);
  LOG(INFO) << "PART 1: " << first_invalid;
  LOG(INFO) << "PART 2: " << FindBadSequence(numbers, first_invalid);
  return 0;
}
