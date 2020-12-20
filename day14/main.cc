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

typedef absl::flat_hash_map<int64_t, int64_t> Memory;

struct Instruction {
  std::string mask;
  std::vector<std::pair<int64_t, int64_t>> writes;
};

int64_t DoPart1(const std::vector<Instruction>& instructions) {
  Memory mem;

  for (auto [mask, writes] : instructions) {
    int64_t and_mask, or_mask;
    // For the AND portion: convert Xs to 1s (ignore) and read in base 2.
    CHECK(absl::numbers_internal::safe_strtoi_base(
        absl::StrReplaceAll(mask, {{"X", "1"}}), &and_mask, 2));
    // For the OR portion: convert Xs to 0s (ignore) and read in base 2.
    CHECK(absl::numbers_internal::safe_strtoi_base(
        absl::StrReplaceAll(mask, {{"X", "0"}}), &or_mask, 2));

    for (auto [address, val] : writes) {
      val &= and_mask;
      val |= or_mask;
      mem[address] = val;
    }
  }
  // Sum all values;
  int64_t total = 0;
  for (auto [_, val] : mem) {
    total += val;
  }
  return total;
}

int FindFirstSetDigit(int64_t bitset) {
  if (bitset == 0) return -1;

  int digit = 0;
  while (bitset) {
    if (bitset & 1) {
      return digit;
    }
    bitset = bitset >> 1;
    ++digit;
  }
  CHECK(false);
}

void DoCombinationsFrom(Memory& mem, int64_t address, int64_t mask,
                        int64_t value) {
  int digit = FindFirstSetDigit(mask);

  // Tail case: just apply the value.
  if (digit == -1) {
    mem[address] = value;
    return;
  }

  CHECK(digit >= 0);

  // Recursive: split at the rightmost set bit.
  int64_t address_on = address | (1ll << digit);
  int64_t address_off = address & ~(1ll << digit);
  mask &= (mask - 1);

  DoCombinationsFrom(mem, address_on, mask, value);
  DoCombinationsFrom(mem, address_off, mask, value);
}

int64_t DoPart2(const std::vector<Instruction>& instructions) {
  Memory mem;

  for (auto [mask, writes] : instructions) {
    // For the OR portion: convert Xs to 0s (ignore) and read in base 2.
    int64_t or_mask;
    CHECK(absl::numbers_internal::safe_strtoi_base(
        absl::StrReplaceAll(mask, {{"X", "0"}}), &or_mask, 2));

    // Find all the xs by overwriting 1s to 0s and then Xs to 1s.
    int64_t x_mask;
    CHECK(absl::numbers_internal::safe_strtoi_base(
        absl::StrReplaceAll(mask, {{"1", "0"}, {"X", "1"}}), &x_mask, 2));
    CHECK(x_mask > 0);

    for (auto [address, val] : writes) {
      address |= or_mask;
      DoCombinationsFrom(mem, address, x_mask, val);
    }
  }
  // Sum all values;
  int64_t total = 0;
  for (auto [_, val] : mem) {
    total += val;
  }
  return total;
}

int main(int argc, char** argv) {
  google::InstallFailureSignalHandler();
  google::InitGoogleLogging(argv[0]);
  FLAGS_logtostderr = 1;

  std::ifstream file(argv[1]);
  CHECK(file);

  std::string line;
  std::vector<Instruction> instructions;
  absl::optional<Instruction> instruction;
  // Apply the masks as an and-mask to clear out places with zeros and an
  // or-mask to add places with ones.
  while (std::getline(file, line)) {
    std::vector<absl::string_view> parts = absl::StrSplit(line, " = ");
    if (parts[0] == "mask") {
      if (instruction) {
        instructions.push_back(*instruction);
      }
      instruction.emplace();
      instruction->mask = parts[1];
    } else {
      CHECK(instruction);
      // First part is mem[address], second part is the value.
      auto mem_string =
          absl::StripSuffix(absl::StripPrefix(parts[0], "mem["), "]");
      int64_t address;
      CHECK(absl::SimpleAtoi(mem_string, &address));
      // Second part is just the value. Since we're guaranteed the AND and OR
      // portions don't overlap, order doesn't matter.
      int64_t val;
      CHECK(absl::SimpleAtoi(parts[1], &val));
      instruction->writes.emplace_back(address, val);
    }
  }
  if (instruction) {
    instructions.push_back(*instruction);
  }
  LOG(INFO) << "PART 1: " << DoPart1(instructions);
  LOG(INFO) << "PART 2: " << DoPart2(instructions);
  return 0;
}
