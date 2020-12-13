#include <execution>
#include <fstream>
#include <numeric>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_split.h"
#include "absl/strings/strip.h"
#include "absl/strings/substitute.h"
#include "absl/types/optional.h"
#include "glog/logging.h"

bool AllPredsTrue(const std::vector<std::function<bool(int64_t)>>& preds,
                  int64_t number) {
  // LOG(INFO) << "CHECKING " << number;
  for (const auto& pred : preds) {
    if (!pred(number)) return false;
    LOG(INFO) << "Pred succeeded.";
  }
  return true;
}

// From https://rosettacode.org/wiki/Chinese_remainder_theorem#C.2B.2B
// Solve for x in "(a * x) % b == 1"
template <typename T>
T MulInv(T a, T b) {
  T b0 = b;
  T x0 = 0;
  T x1 = 1;

  if (b == 1) {
    return 1;
  }

  while (a > 1) {
    T q = a / b;
    T amb = a % b;
    a = b;
    b = amb;

    T xqx = x1 - q * x0;
    x1 = x0;
    x0 = xqx;
  }

  if (x1 < 0) {
    x1 += b0;
  }

  return x1;
}

// From https://rosettacode.org/wiki/Chinese_remainder_theorem#C.2B.2B
// Solve for x given a list of equations of the form:
// x = a mod n
template <typename T>
T ChineseRemainder(std::vector<T> n, std::vector<T> a) {
  // Multiple all the ns so we can solve mod <product>.
  T prod = std::reduce(n.begin(), n.end(), 1, [](T a, T b) { return a * b; });

  T sm = 0;
  for (int i = 0; i < n.size(); i++) {
    T p = prod / n[i];
    sm += a[i] * MulInv(p, n[i]) * p;
  }

  return sm % prod;
}

int main(int argc, char** argv) {
  google::InstallFailureSignalHandler();
  google::InitGoogleLogging(argv[0]);
  FLAGS_logtostderr = 1;

  std::ifstream file(argv[1]);
  CHECK(file);

  std::string line;
  CHECK(std::getline(file, line));
  int time_to_depart;
  CHECK(absl::SimpleAtoi(line, &time_to_depart));

  CHECK(std::getline(file, line));
  std::vector<int> busses;
  std::vector<std::string> bus_strings = absl::StrSplit(line, ",");
  for (auto bus_string : bus_strings) {
    if (bus_string == "x") continue;
    int bus;
    CHECK(absl::SimpleAtoi(bus_string, &bus));
    busses.push_back(bus);
  }

  // Part 1: find the bus that leaves closest to but not before our departure
  // time.
  int min_distance = INT_MAX;
  int bus_id = -1;
  for (int bus : busses) {
    int distance = bus - (time_to_depart % bus);
    if (distance < min_distance) {
      min_distance = distance;
      bus_id = bus;
    }
  }
  LOG(INFO) << "PART 1: " << bus_id * min_distance;

  // Part 2: lets try some chinese remainder theorem, because that's what the
  // subreddit folks hinted at. This is an obnoxious "gotcha" problem that you
  // can't reasonably solve with brute force (estimates are anywhere from 2-70
  // days, on the subreddit).
  // I don't actually know the math for this, so I took it from rosettacode.org.
  // This isn't a "programming" problem, this is a "do you recognize a math
  // identity" problem.
  std::vector<int64_t> a;
  std::vector<int64_t> n;
  for (int64_t i = 0; i < bus_strings.size(); ++i) {
    int64_t bus_id;
    if (!absl::SimpleAtoi(bus_strings[i], &bus_id)) continue;

    // Remember that we're looking for next largest, so we're looking for the
    // coefficient to be bus_id - i.
    a.push_back(bus_id - i);
    n.push_back(bus_id);
  }

  LOG(INFO) << "PART 2: " << ChineseRemainder(n, a);
  return 0;
}
