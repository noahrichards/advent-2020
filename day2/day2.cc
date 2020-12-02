#include <fstream>
#include <iostream>

#include "absl/strings/numbers.h"
#include "absl/strings/str_format.h"
#include "absl/strings/str_split.h"

struct PasswordEntry {
  int min;
  int max;
  char c;
  std::string pwd;
};

bool MeetsPart1Requirements(PasswordEntry entry) {
  int count = 0;
  for (char c : entry.pwd) {
    if (c == entry.c) ++count;
  }
  return count >= entry.min && count <= entry.max;
}

bool MeetsPart2Requirements(PasswordEntry entry) {
  int count = 0;
  return entry.pwd[entry.min] != entry.pwd[entry.max] &&
         (entry.pwd[entry.min] == entry.c || entry.pwd[entry.max] == entry.c);
}

void CheckPasswords(std::vector<PasswordEntry> entries,
                    std::function<bool(PasswordEntry)> check_entry_func) {
  int total = 0;
  int valid = 0;
  for (auto entry : entries) {
    ++total;
    if (check_entry_func(entry)) ++valid;
  }
  std::cout << absl::StrFormat("Total count: %d", total) << std::endl
            << absl::StrFormat("Valid count: %d", valid) << std::endl;
}

int main(int argc, char** argv) {
  std::vector<PasswordEntry> entries;
  std::ifstream file(argv[1]);

  std::string line;
  while (std::getline(file, line)) {
    // Format is: 1-3 a: abcde
    // Split out reqs from password.
    std::vector<absl::string_view> vec = absl::StrSplit(line, ":");
    auto reqs = vec[0];
    auto pwd = vec[1];
    // Reqs are: min-max character
    vec = absl::StrSplit(reqs, " ");
    auto character = vec[1][0];
    vec = absl::StrSplit(vec[0], "-");
    int min, max;
    if (!absl::SimpleAtoi(vec[0], &min) || !absl::SimpleAtoi(vec[1], &max)) {
      std::cerr << "Failed to convert: " << vec[0] << "/" << vec[1]
                << std::endl;
      return 1;
    }
    entries.push_back(PasswordEntry{min, max, character, std::string(pwd)});
  }

  std::cout << "PART 1:" << std::endl;
  CheckPasswords(entries,
                 [&](auto entry) { return MeetsPart1Requirements(entry); });
  std::cout << std::endl << "PART 2:" << std::endl;
  CheckPasswords(entries,
                 [&](auto entry) { return MeetsPart2Requirements(entry); });
  return 0;
}
