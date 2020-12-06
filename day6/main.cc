#include <fstream>

#include "absl/container/flat_hash_set.h"
#include "glog/logging.h"

typedef absl::flat_hash_set<char> Answers;

int AllContain(const std::vector<Answers>& group, char c) {
  for (const auto& answers : group) {
    if (answers.count(c) == 0) return false;
  }
  return true;
}

int CountAllYes(const std::vector<Answers>& group) {
  int count = 0;
  for (char c = 'a'; c <= 'z'; ++c) {
    count += AllContain(group, c) ? 1 : 0;
  }
  return count;
}

int main(int argc, char** argv) {
  google::InstallFailureSignalHandler();
  google::InitGoogleLogging(argv[0]);
  FLAGS_logtostderr = 1;

  std::ifstream file(argv[1]);
  CHECK(file);

  std::string line;
  std::vector<Answers> group;
  Answers any_yes_answers;

  long any_yes_sum = 0;
  long all_yes_sum = 0;

  while (std::getline(file, line)) {
    if (line.empty()) {
      all_yes_sum += CountAllYes(group);
      any_yes_sum += any_yes_answers.size();
      any_yes_answers.clear();
      group.clear();
    } else {
      Answers answers;
      for (char c : line) {
        answers.insert(c);
        any_yes_answers.insert(c);
      }
      group.push_back(answers);
    }
  }
  any_yes_sum += any_yes_answers.size();
  all_yes_sum += CountAllYes(group);

  LOG(INFO) << "PART 1: " << any_yes_sum;
  LOG(INFO) << "PART 2: " << all_yes_sum;
  return 0;
}
