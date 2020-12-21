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

typedef std::vector<int64_t> Ticket;

struct Rule {
  std::string name;
  std::vector<std::pair<int64_t, int64_t>> ranges;
};

Ticket ParseTicket(std::string line) {
  Ticket ticket;
  int number;
  for (auto part : absl::StrSplit(line, ",")) {
    CHECK(absl::SimpleAtoi(part, &number)) << part;
    ticket.push_back(number);
  }
  return ticket;
}

Rule ParseRule(std::string line) {
  Rule rule;
  std::vector<std::string> parts = absl::StrSplit(line, ": ");
  CHECK(parts.size() == 2);
  rule.name = parts[0];
  for (auto part : absl::StrSplit(parts[1], " or ")) {
    parts = absl::StrSplit(part, "-");
    CHECK(parts.size() == 2) << part;
    int64_t min, max;
    CHECK(absl::SimpleAtoi(parts[0], &min));
    CHECK(absl::SimpleAtoi(parts[1], &max));
    rule.ranges.push_back({min, max});
  }
  return rule;
}

bool RuleMatches(Rule rule, int64_t value) {
  for (auto [min, max] : rule.ranges) {
    if (value >= min && value <= max) {
      return true;
    }
  }
  return false;
}

bool CheckValidity(const std::vector<Rule>& rules, Ticket ticket,
                   int64_t& error_rate) {
  bool valid = true;
  for (auto val : ticket) {
    bool matches_one = false;
    for (auto rule : rules) {
      if (RuleMatches(rule, val)) {
        matches_one = true;
        break;
      }
    }
    if (!matches_one) {
      error_rate += val;
      valid = false;
    }
  }
  return valid;
}

int main(int argc, char** argv) {
  google::InstallFailureSignalHandler();
  google::InitGoogleLogging(argv[0]);
  FLAGS_logtostderr = 1;

  std::ifstream file(argv[1]);
  CHECK(file);

  std::vector<Rule> rules;

  std::string line;
  // Section 1: rules
  while (std::getline(file, line)) {
    if (line.empty()) break;
    rules.push_back(ParseRule(line));
  }

  // Section 2: your ticket
  CHECK(std::getline(file, line));
  CHECK_EQ(line, "your ticket:");
  CHECK(std::getline(file, line));
  auto your_ticket = ParseTicket(line);

  // Section 3: other tickets
  std::vector<Ticket> other_tickets;
  int64_t error_rate = 0;
  while (std::getline(file, line)) {
    if (line.empty() || line == "nearby tickets:") continue;
    auto ticket = ParseTicket(line);
    // Part 1 (and needed for Part 2): throw away invalid and count them.
    if (!CheckValidity(rules, ticket, error_rate)) continue;
    other_tickets.push_back(ParseTicket(line));
  }

  LOG(INFO) << "PART 1: " << error_rate;

  // Part 2: for every rule, figure out which ticket value (index) matches that
  // rule and print the product of "departure" values on your ticket.
  int value_count = other_tickets[0].size();

  // Find all values that could fit each rule.
  absl::flat_hash_map<int, absl::flat_hash_set<int>> rule_to_matching_values;
  for (int rule_index = 0; rule_index < rules.size(); ++rule_index) {
    auto rule = rules[rule_index];
    auto& matching_values = rule_to_matching_values[rule_index];
    for (int i = 0; i < value_count; ++i) {
      bool all_match = true;
      for (const auto& ticket : other_tickets) {
        if (!RuleMatches(rule, ticket[i])) {
          all_match = false;
          break;
        }
      }
      if (all_match) {
        matching_values.insert(i);
      }
    }
    CHECK(!rule_to_matching_values[rule_index].empty())
        << "Can't find match for rule: " << rule.name;
  }

  // Treat this like simplest sudoku (also how windiff works, sorta): find the
  // rules that match a single value, remove that value from the list of
  // allowed matches of all other rules, and repeat until there are no rules
  // left. Harder sudoku will end up with points where there isn't a clear 1:1
  // mapping and you have to try different outcomes, but I'm assuming this
  // puzzle was designed a bit simpler.
  //
  // For the curious: Windiff's version of this is: find a line that matches
  // uniquely and add them to the match set. Walk lines forwards and backwards
  // from that line and add them to the match set as long as they match. When
  // complete, anything that remains is an added/removed line.
  absl::flat_hash_map<int, int> rule_to_matching_value;
  absl::flat_hash_set<int> matched_rules;
  while (matched_rules.size() != value_count) {
    // Find things with a single match.
    for (auto& [rule_index, matching_values] : rule_to_matching_values) {
      if (matching_values.empty()) continue;
      if (matching_values.size() == 1) {
        // Definite match.
        auto value = *matching_values.begin();
        matching_values.clear();
        auto [_, set_inserted] = matched_rules.insert(value);
        CHECK(set_inserted) << "Value already matched: " << value;
        auto [__, map_inserted] =
            rule_to_matching_value.insert({rule_index, value});
        CHECK(map_inserted) << "Rule already matched: " << rule_index;
        for (auto& [_, other_matching_values] : rule_to_matching_values) {
          other_matching_values.erase(value);
        }
      }
    }
  }

  // If we got here, the previous while loop finished, so the puzzle is set up
  // as expected :) Part 2 asks us to multiply the values on *our* ticket that
  // start with the word "departure".
  int64_t product = 1;
  for (auto [rule_index, value_index] : rule_to_matching_value) {
    auto rule = rules[rule_index];
    if (absl::StartsWith(rule.name, "departure")) {
      product *= your_ticket[value_index];
    }
  }

  LOG(INFO) << "PART 2: " << product;
  return 0;
}
