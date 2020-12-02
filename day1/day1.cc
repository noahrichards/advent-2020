#include <fstream>
#include <iostream>

#include "absl/container/flat_hash_set.h"
#include "absl/strings/str_format.h"

constexpr int kTotal = 2020;

void DoPart1(const absl::flat_hash_set<int>& numbers) {
  for (int i : numbers) {
    int other = kTotal - i;
    if (numbers.count(other) > 0) {
      std::cout << absl::StrFormat("PART1: Found: %d and %d, mult: %d", i,
                                   other, i * other)
                << std::endl;
      return;
    }
  }
  std::cerr << "PART1: NOT FOUND" << std::endl;
}

void DoPart2(const absl::flat_hash_set<int>& numbers) {
  for (int i : numbers) {
    for (int j : numbers) {
      if (i == j) continue;
      int other = kTotal - i - j;
      if (numbers.count(other) > 0) {
        std::cout << absl::StrFormat("PART2: Found: %d/%d/%d, mult: %d", i, j,
                                     other, i * j * other)
                  << std::endl;
        return;
      }
    }
  }
}

int main(int argc, char** argv) {
  absl::flat_hash_set<int> numbers;
  std::ifstream file(argv[1]);

  int in;
  while (file >> in) {
    numbers.insert(in);
  }

  DoPart1(numbers);
  DoPart2(numbers);
  return 0;
}
