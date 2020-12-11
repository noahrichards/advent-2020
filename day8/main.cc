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

struct Op {
  Opcode opcode;
  int operand;
};

Opcode ParseOpcode(std::string value) {
  if (value == "nop") return kNop;
  if (value == "acc") return kAcc;
  if (value == "jmp") return kJmp;
  CHECK(false);
}

Op ParseOp(std::string line) {
  std::vector<std::string> parts = absl::StrSplit(line, " ");
  CHECK(parts.size() == 2);
  int operand;
  CHECK(absl::SimpleAtoi(parts[1], &operand));
  return {ParseOpcode(parts[0]), operand};
}

typedef std::vector<Op> Code;

// Returns true if the program is valid (no value is executed twice and the
// final pc is the next instruction after |code|) and the associated accumulator
// value.
std::tuple<bool, int> ExecuteProgram(Code code) {
  int accumulator = 0;
  int pc = 0;
  absl::flat_hash_set<int> pc_visited;
  for (;;) {
    if (pc == code.size()) return {true, accumulator};
    if (pc < 0 || pc > code.size() || pc_visited.count(pc) > 0) {
      return {false, accumulator};
    }
    pc_visited.insert(pc);
    switch (code[pc].opcode) {
      case kNop:
        ++pc;
        break;
      case kAcc:
        accumulator += code[pc].operand;
        ++pc;
        break;
      case kJmp:
        pc += code[pc].operand;
        break;
    }
  }
}

int main(int argc, char** argv) {
  google::InstallFailureSignalHandler();
  google::InitGoogleLogging(argv[0]);
  FLAGS_logtostderr = 1;

  std::ifstream file(argv[1]);
  CHECK(file);

  Code code;
  std::string line;
  while (std::getline(file, line)) {
    code.push_back(ParseOp(line));
  }

  // For Part 1: execute until an instruction is re-hit and return the
  // accumulator at that point.
  {
    auto [correct, accumulator] = ExecuteProgram(code);
    CHECK(!correct);
    LOG(INFO) << "PART 1: " << accumulator;
  }

  // For Part 2: try all flips of a single nop -> jmp or jmp -> nop until one is
  // successful;
  for (int i = 0; i < code.size(); ++i) {
    if (code[i].opcode == kAcc) continue;
    Code modified = code;
    modified[i].opcode = code[i].opcode == kNop ? kJmp : kNop;
    auto [correct, accumulator] = ExecuteProgram(modified);
    if (correct) {
      LOG(INFO) << "PART 2: " << accumulator;
    }
  }
  return 0;
}
