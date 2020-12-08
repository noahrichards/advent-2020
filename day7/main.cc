#include <fstream>

#include "absl/strings/strip.h"
#include "glog/logging.h"

struct BagType {
  std::string adjective;
  std::string color;
};

struct ContainRule {
  int count;
  BagType bag;
};

struct Rule {
  BagType subject;
  std::vector<ContainRule> contains;
};

typedef std::vector<Rule> Rules;

// Parses the words into a BagType
BagType ParseBagType(std::vector<std::string> words) {
  return {words[0], words{1}};
}

// Parses the words into a ContainRule
ContainRule ParseContainRule(std::vector<std::string> words) {
  int count;
  CHECK(absl::SimpleAtoi(words[0], &count));
  return {count, words[1], words[2]};
}

Rule ParseRule(std::string line) {
  std::vector<std::string> parts = absl::StrSplit(line, " contain ");
  CHECK(parts.size() == 2);
  Rule rule;
  rule.subject = ParseBagType(absl::StrSplit(parts[0], " "));
  parts = absl::StrSplit(parts[1], ", ");
  for (auto part : parts) {
    rule.contains.push_back(ParseContainRule(part));
  }
}

int main(int argc, char** argv) {
  google::InstallFailureSignalHandler();
  google::InitGoogleLogging(argv[0]);
  FLAGS_logtostderr = 1;

  std::ifstream file(argv[1]);
  CHECK(file);

  Rules rules;
  std::string line;
  while (std::getline(file, line)) {
    line = absl::StripSuffix(line, ".");
    rules.push_back(ParseRule(line));
  }

  LOG(INFO) << "PART 1: ";
  LOG(INFO) << "PART 2: ";
  return 0;
}
