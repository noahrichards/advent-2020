#include <fstream>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_split.h"
#include "absl/strings/strip.h"
#include "absl/strings/substitute.h"
#include "absl/types/optional.h"
#include "glog/logging.h"

typedef std::vector<std::string> SeatMap;

std::string PrintSeats(const SeatMap& seats) {
  return absl::StrJoin(seats, "\n\t");
}

struct Location {
  int row;
  int col;
  std::string ToString() const { return absl::Substitute("$0,$1", row, col); }
};

struct Direction {
  int row_delta;
  int col_delta;
};

Location Move(Location location, Direction direction) {
  return {location.row + direction.row_delta,
          location.col + direction.col_delta};
}

int CountOccupied(SeatMap seats) {
  int count = 0;
  for (auto row : seats) {
    for (auto seat : row) {
      count += seat == '#' ? 1 : 0;
    }
  }
  return count;
}

void CheckAdjacent(const SeatMap& seats, Location location,
                   std::function<bool(char)> check_seat_func) {
  int height = seats.size();
  int width = seats[0].size();

  // All combinations of +1, 0, -1 and +1, 0, -1 (except for 0,0).
  for (int row_delta = -1; row_delta <= 1; ++row_delta) {
    int row = location.row + row_delta;
    if (row < 0 || row >= height) continue;
    for (int col_delta = -1; col_delta <= 1; ++col_delta) {
      if (row_delta == 0 && col_delta == 0) continue;
      int col = location.col + col_delta;
      if (col < 0 || col >= width) continue;
      if (!check_seat_func(seats[row][col])) return;
    }
  }
}

// Walks in a single direction. Returns true to keep walking.
bool Walk(const SeatMap& seats, Location location, Direction walk_direction,
          std::function<bool(char)> func) {
  int height = seats.size();
  int width = seats[0].size();

  for (auto pos = Move(location, walk_direction);;
       pos = Move(pos, walk_direction)) {
    auto [r, c] = pos;
    if (r < 0 || c < 0) return true;
    if (c >= width || r >= height) return true;
    char seat = seats[r][c];
    if (seat == '.') continue;
    return func(seats[r][c]);
  };
  return true;
}

void WalkFirstSeen(const SeatMap& seats, Location location,
                   std::function<bool(char)> func) {
  // Do a walk for all combinations of +1, 0, -1 and +1, 0, -1 (except for
  // 0,0) until we reach a seat (empty or taken).
  for (int row_delta = -1; row_delta <= 1; ++row_delta) {
    for (int col_delta = -1; col_delta <= 1; ++col_delta) {
      if (row_delta == 0 && col_delta == 0) continue;
      // Stop if Walk/the func says we're done.
      if (!Walk(seats, location, {row_delta, col_delta}, func)) {
        return;
      }
    }
  }
}

std::tuple<char, bool> StepSeat(const SeatMap& seats, bool adjacent_only,
                                int tolerance, Location location) {
  char current_seat = seats[location.row][location.col];
  if (current_seat == '.') return {current_seat, false};

  bool was_empty = current_seat == 'L';
  int count = 0;
  std::function<bool(char)> check_func = [&](char c) {
    if (c == '#') {
      ++count;
      // Empty seats with any adjacent don't get filled.
      if (was_empty) return false;
      // Taken seats with tolerance+ adjacent are cleared.
      if (count >= tolerance) return false;
    }
    return true;
  };

  if (adjacent_only) {
    CheckAdjacent(seats, location, check_func);

  } else {
    WalkFirstSeen(seats, location, check_func);
  }

  if (was_empty && count == 0) {
    return {'#', true};
  } else if (!was_empty && count >= tolerance) {
    return {'L', true};
  }
  return {current_seat, false};
}

// Run a step of the simulation. Returns the new seatmap and true if the
// state has changed.
std::tuple<SeatMap, bool> Step(const SeatMap& seats, bool adjacent_only,
                               int tolerance) {
  SeatMap next = seats;
  bool has_changed = false;

  for (int row = 0; row < seats.size(); ++row) {
    for (int col = 0; col < seats[0].size(); ++col) {
      auto [new_seat, seat_changed] =
          StepSeat(seats, adjacent_only, tolerance, {row, col});
      has_changed = has_changed || seat_changed;
      next[row][col] = new_seat;
    }
  }
  return {next, has_changed};
}

std::tuple<int, int> RunSimulation(SeatMap seats, bool adjacent_only,
                                   int tolerance) {
  int step_count = 0;
  for (;; ++step_count) {
    auto [next, has_changed] = Step(seats, adjacent_only, tolerance);
    seats = next;
    VLOG(1) << "STEP " << step_count << "\n" << PrintSeats(seats);
    if (!has_changed) break;
  }
  return {step_count, CountOccupied(seats)};
}

int main(int argc, char** argv) {
  google::InstallFailureSignalHandler();
  google::InitGoogleLogging(argv[0]);
  FLAGS_logtostderr = 1;

  std::ifstream file(argv[1]);
  CHECK(file);

  SeatMap seats;
  int width = 0;
  std::string line;
  while (std::getline(file, line)) {
    CHECK(width == 0 || line.size() == width);
    width = line.size();
    seats.push_back(line);
  }

  // Part 1: tolerance 4, only adjacent.
  {
    auto [step_count, occupied] = RunSimulation(seats, true, 4);
    LOG(INFO) << "PART 1: " << step_count << ": " << occupied;
  }
  // Part 2: tolerance 5, all visible.
  {
    auto [step_count, occupied] = RunSimulation(seats, false, 5);
    LOG(INFO) << "PART 2: " << step_count << ": " << occupied;
  }

  return 0;
}
