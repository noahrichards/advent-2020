#include <fstream>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_split.h"
#include "absl/types/optional.h"
#include "glog/logging.h"

typedef absl::flat_hash_map<std::string, std::string> Passport;

bool ValidateYear(std::string value, int min, int max) {
  int year;
  if (!absl::SimpleAtoi(value, &year)) return false;
  return year >= min && year <= max;
}

bool ValidateHeight(std::string value) {
  if (value.size() < 3) return false;
  auto unit = value.substr(value.size() - 2, 2);
  auto height_str = value.substr(0, value.size() - 2);
  int height;
  if (!absl::SimpleAtoi(height_str, &height)) return false;
  if (unit == "cm") {
    return height >= 150 && height <= 193;
  } else if (unit == "in") {
    return height >= 59 && height <= 76;
  }
  return false;
}

bool ValidateHexDigit(char c) {
  return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
}

bool ValidateHex(std::string value) {
  if (value.size() != 7) return false;
  if (value[0] != '#') return false;
  for (int i = 1; i < 7; ++i) {
    if (!ValidateHexDigit(value[i])) return false;
  }
  return true;
}

bool ValidateEyeColor(std::string value) {
  absl::flat_hash_set<std::string> valid_eye_colors = {
      "amb", "blu", "brn", "gry", "grn", "hzl", "oth"};
  return valid_eye_colors.count(value) > 0;
}

bool ValidatePassportId(std::string value) {
  if (value.size() != 9) return false;
  for (char c : value) {
    if (c < '0' || c > '9') return false;
  }
  return true;
}

std::tuple<bool, bool> ValidatePassport(absl::optional<Passport> passport) {
  if (!passport) return {false, false};

  absl::flat_hash_set<std::string> required_fields = {
      "byr", "iyr", "eyr", "hgt", "hcl", "ecl", "pid"};
  bool fully_valid = true;
  for (auto [key, value] : *passport) {
    required_fields.erase(key);
    if (key == "byr") {
      fully_valid = fully_valid && ValidateYear(value, 1920, 2002);
    } else if (key == "iyr") {
      fully_valid = fully_valid && ValidateYear(value, 2010, 2020);
    } else if (key == "eyr") {
      fully_valid = fully_valid && ValidateYear(value, 2020, 2030);
    } else if (key == "hgt") {
      fully_valid = fully_valid && ValidateHeight(value);
    } else if (key == "hcl") {
      fully_valid = fully_valid && ValidateHex(value);
    } else if (key == "ecl") {
      fully_valid = fully_valid && ValidateEyeColor(value);
    } else if (key == "pid") {
      fully_valid = fully_valid && ValidatePassportId(value);
    }
  }
  return {required_fields.empty(), fully_valid};
}

int main(int argc, char** argv) {
  google::InstallFailureSignalHandler();
  google::InitGoogleLogging(argv[0]);
  FLAGS_logtostderr = 1;

  std::ifstream file(argv[1]);
  CHECK(file);

  absl::optional<Passport> passport;
  std::string line;
  int present_count = 0;
  int valid_count = 0;
  while (std::getline(file, line)) {
    if (line.empty()) {
      auto [present, valid] = ValidatePassport(passport);
      if (present) {
        ++present_count;
        if (valid) ++valid_count;
      }
      passport.reset();
    } else {
      if (!passport) passport.emplace();
      std::vector<std::string> entries = absl::StrSplit(line, " ");
      for (auto entry : entries) {
        std::vector<std::string> parts = absl::StrSplit(entry, ":");
        passport->emplace(parts[0], parts[1]);
      }
    }
  }
  auto [present, valid] = ValidatePassport(passport);
  if (present) {
    ++present_count;
    if (valid) ++valid_count;
  }

  LOG(INFO) << "PART 1: " << present_count;
  LOG(INFO) << "PART 2: " << valid_count;
  return 0;
}
