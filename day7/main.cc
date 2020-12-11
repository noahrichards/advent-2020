#include <fstream>
#include <queue>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_split.h"
#include "absl/strings/strip.h"
#include "absl/strings/substitute.h"
#include "absl/types/optional.h"
#include "glog/logging.h"

struct BagType {
  std::string adjective;
  std::string color;

  template <typename H>
  friend H AbslHashValue(H h, const BagType& bag_type) {
    return H::combine(std::move(h), bag_type.adjective, bag_type.color);
  }
  bool operator==(const BagType& other) const {
    return adjective == other.adjective && color == other.color;
  }
  std::string ToString() const {
    return absl::Substitute("$0 $1", adjective, color);
  }
};

struct ContainRule {
  int count;
  BagType bag;

  std::string ToString() const {
    return absl::Substitute("$0 $1", count, bag.ToString());
  }
};

struct Rule {
  BagType subject;
  std::vector<ContainRule> contains;
};

typedef absl::flat_hash_map<BagType, Rule> Rules;

// Parses the words into a BagType
BagType ParseBagType(std::vector<std::string> words) {
  return {words[0], words[1]};
}

// Parses the words into a ContainRule
ContainRule ParseContainRule(std::vector<std::string> words) {
  int count;
  CHECK(absl::SimpleAtoi(words[0], &count)) << "'" << words[0] << "'";
  return {count, {words[1], words[2]}};
}

Rule ParseRule(std::string line) {
  std::vector<std::string> parts = absl::StrSplit(line, " contain ");
  CHECK(parts.size() == 2);
  Rule rule;
  rule.subject = ParseBagType(absl::StrSplit(parts[0], " "));
  parts = absl::StrSplit(parts[1], ", ");
  for (auto part : parts) {
    std::vector<std::string> words = absl::StrSplit(part, " ");
    if (words[0] == "no") {
      continue;
    }
    rule.contains.push_back(ParseContainRule(words));
  }
  return rule;
}

int Count(Rules rules, BagType bag_type) {
  auto rule = rules.find(bag_type);
  int total = 1;  // Count this bag.
  if (rule == rules.end()) {
    return total;
  }
  for (auto contains : rule->second.contains) {
    total += contains.count * Count(rules, contains.bag);
  }
  return total;
}

int main(int argc, char** argv) {
  google::InstallFailureSignalHandler();
  google::InitGoogleLogging(argv[0]);
  FLAGS_logtostderr = 1;

  std::ifstream file(argv[1]);
  CHECK(file);

  Rules rules;
  std::string line;
  // For part1: remember all child->parent bag mappings.
  // Counts don't matter and one bag may be contained in many different parents.
  absl::flat_hash_map<BagType, absl::flat_hash_set<BagType>>
      may_be_contained_in;
  while (std::getline(file, line)) {
    line = std::string(absl::StripSuffix(line, "."));
    auto rule = ParseRule(line);
    rules.try_emplace(rule.subject, rule);
    // Part 1: record all child->parent mappings.
    for (auto contains : rule.contains) {
      may_be_contained_in.try_emplace(contains.bag);
      may_be_contained_in[contains.bag].insert(rule.subject);
    }
  }

  // Part 1: Get the full set of bags that may indirectly hold a shiny gold
  // bag.
  absl::flat_hash_set<BagType> shiny_golden_containers;
  std::queue<BagType> bags_to_check;
  bags_to_check.push({"shiny", "gold"});
  while (!bags_to_check.empty()) {
    auto bag = bags_to_check.front();
    bags_to_check.pop();
    auto may_contain = may_be_contained_in.find(bag);
    if (may_contain == may_be_contained_in.end()) continue;
    // Find everything this bag can go in, remember that, and then add them to
    // the list to check next.
    for (auto parent : may_contain->second) {
      auto [_, did_insert] = shiny_golden_containers.insert(parent);
      if (did_insert) {
        bags_to_check.push(parent);
      }
    }
  }

  LOG(INFO) << "PART 1: " << shiny_golden_containers.size();

  // For part 2: starting with the shiny gold bag rule, count the total number
  // of bags inside. Subtract one so we're not counting the shiny gold bag
  // itself.
  LOG(INFO) << "PART 2: " << Count(rules, {"shiny", "gold"}) - 1;
  return 0;
}
