#include <fstream>

#include "absl/strings/numbers.h"
#include "absl/strings/str_split.h"
#include "absl/strings/substitute.h"
#include "glog/logging.h"

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
  LOG(INFO) << absl::Substitute("Total count: $0", total);
  LOG(INFO) << absl::Substitute("Valid count: $0", valid);
}

int main(int argc, char** argv) {
  google::InstallFailureSignalHandler();
  google::InitGoogleLogging(argv[0]);
  FLAGS_logtostderr = 1;

  std::vector<PasswordEntry> entries;
  std::ifstream file(argv[1]);
  CHECK(file);

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
      LOG(ERROR) << "Failed to convert: " << vec[0] << "/" << vec[1];
      return 1;
    }
    entries.push_back(PasswordEntry{min, max, character, std::string(pwd)});
  }

  LOG(INFO) << "PART 1:";
  CheckPasswords(entries,
                 [&](auto entry) { return MeetsPart1Requirements(entry); });
  LOG(INFO) << "PART 2:";
  CheckPasswords(entries,
                 [&](auto entry) { return MeetsPart2Requirements(entry); });
  return 0;
}
