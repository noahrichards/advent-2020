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

struct Location {
  int x, y, z, w;
  template <typename H>
  friend H AbslHashValue(H h, const Location& l) {
    return H::combine(std::move(h), l.x, l.y, l.z, l.w);
  }
  bool operator==(const Location& other) const {
    return x == other.x && y == other.y && z == other.z && w == other.w;
  }
};

typedef absl::flat_hash_set<Location> World;

void VisitNeighbors(Location location, int dimensions,
                    std::function<void(Location)> visit) {
  auto [x, y, z, w] = location;
  for (int i = -1; i <= 1; ++i) {
    for (int j = -1; j <= 1; ++j) {
      for (int k = -1; k <= 1; ++k) {
        for (int l = -1; l <= 1; ++l) {
          // In 3 dimensions, w is always zero.
          if (dimensions == 3 && l != 0) continue;
          if (i == 0 && j == 0 && k == 0 && l == 0) continue;
          visit({x + i, y + j, z + k, w + l});
        }
      }
    }
  }
}

World RunCycle(World world, int dimensions) {
  absl::flat_hash_map<Location, int> neighbor_count;
  for (auto cube : world) {
    VisitNeighbors(cube, dimensions,
                   [&](Location loc) { neighbor_count[loc]++; });
  }

  World new_world;
  for (auto [location, count] : neighbor_count) {
    if (count == 3 || (world.count(location) > 0 && count == 2)) {
      new_world.insert(location);
    }
  }
  return new_world;
}

int main(int argc, char** argv) {
  google::InstallFailureSignalHandler();
  google::InitGoogleLogging(argv[0]);
  FLAGS_logtostderr = 1;

  std::ifstream file(argv[1]);
  CHECK(file);

  std::string line;
  World starting_world;
  int y = 0;
  while (std::getline(file, line)) {
    for (int x = 0; x < line.size(); ++x) {
      if (line[x] == '#') {
        starting_world.insert({x, y, /*z=*/0});
      }
    }
    ++y;
  }

  // Part 1: run the simulation 6 times in 3 dimensions.
  World world = starting_world;
  for (int i = 0; i < 6; ++i) {
    world = RunCycle(world, 3);
  }
  LOG(INFO) << "PART 1: " << world.size();
  // Part 2: run the simulation 6 times in 4 dimensions.
  world = starting_world;
  for (int i = 0; i < 6; ++i) {
    world = RunCycle(world, 4);
  }
  LOG(INFO) << "PART 2: " << world.size();
  return 0;
}
