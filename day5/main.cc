#include <fstream>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_split.h"
#include "absl/types/optional.h"
#include "glog/logging.h"

struct Seat {
  int row;
  int col;
};

int GetSeatId(Seat seat) { return seat.row * 8 + seat.col; }

Seat CalculateSeat(std::string directions) {
  // Row is [0,127].
  int row = 0;
  int low = 0;
  int high = 128;
  for (int i = 0; i < 7; ++i) {
    int mid = (low + high) / 2;
    if (directions[i] == 'F') {
      high = mid;
    } else {
      low = mid;
    }
  }
  row = low;

  // Col is [0,7].
  int col = 0;
  low = 0;
  high = 8;
  for (int i = 7; i < 10; ++i) {
    int mid = (low + high) / 2;
    if (directions[i] == 'L') {
      high = mid;
    } else {
      low = mid;
    }
  }
  col = low;

  return {row, col};
}

int main(int argc, char** argv) {
  google::InstallFailureSignalHandler();
  google::InitGoogleLogging(argv[0]);
  FLAGS_logtostderr = 1;

  std::ifstream file(argv[1]);
  CHECK(file);

  std::vector<bool> taken_seat_ids(866, false);

  std::string line;
  int max_id = INT_MIN;
  while (std::getline(file, line)) {
    Seat seat = CalculateSeat(line);
    int id = GetSeatId(seat);
    max_id = std::max(max_id, GetSeatId(seat));
    taken_seat_ids[id] = true;
  }

  LOG(INFO) << "PART 1: " << max_id;
  // Part 2: find a single-seat gap.
  for (int i = 1; i < 865; ++i) {
    if (taken_seat_ids[i - 1] && taken_seat_ids[i + 1] && !taken_seat_ids[i]) {
      LOG(INFO) << "PART 2: " << i;
    }
  }
  return 0;
}
