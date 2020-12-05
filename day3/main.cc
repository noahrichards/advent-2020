#include <fstream>

#include "glog/logging.h"

struct PasswordEntry {
  int min;
  int max;
  char c;
  std::string pwd;
};

typedef std::vector<std::string> MapT;

inline bool IsTree(char c) { return c == '#'; }

int CountTreesHit(MapT map, int x_delta, int y_delta) {
  int map_width = map[0].size();
  int x = 0;
  int y = 0;
  int hit_count = IsTree(map[y][x]);
  x = (x + x_delta) % map_width;
  y += y_delta;
  for (; y < map.size(); x = (x + x_delta) % map_width, y += y_delta) {
    // std::string this_line = map[x];
    // bool is_hit = IsTree(map[y][x]);
    // this_line[x] = is_hit ? 'X' : 'O';
    // LOG(INFO) << "coord: " << x << "," << y << ": " << this_line;
    if (IsTree(map[y][x])) ++hit_count;
  }
  return hit_count;
}

int main(int argc, char** argv) {
  google::InstallFailureSignalHandler();
  google::InitGoogleLogging(argv[0]);
  FLAGS_logtostderr = 1;

  std::ifstream file(argv[1]);
  CHECK(file);

  MapT map;
  std::string line;
  while (std::getline(file, line)) {
    map.push_back(line);
  }

  std::vector<std::tuple<int, int>> to_check = {
      {1, 1}, {3, 1}, {5, 1}, {7, 1}, {1, 2}};
  std::vector<int> tree_hits;
  long long product = 1;
  for (auto [x, y] : to_check) {
    int count = CountTreesHit(map, x, y);
    tree_hits.push_back(count);
    product *= count;
  }
  LOG(INFO) << "PART 1: " << tree_hits[1];
  LOG(INFO) << "PART 2: " << product;
  return 0;
}
