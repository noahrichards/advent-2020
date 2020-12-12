#include <fstream>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_split.h"
#include "absl/strings/strip.h"
#include "absl/strings/substitute.h"
#include "absl/types/optional.h"
#include "glog/logging.h"

struct Vector {
  char type;
  int distance;
};

// The position of the ship: an x/y coordinate pair and an orientation in
// degrees.
struct ShipPosition {
  int degrees;
  int x;
  int y;
};

// The position of the waypoint: an x/y offset from the ship.
struct WaypointPosition {
  int x;
  int y;
};

struct ShipAndWaypoint {
  ShipPosition ship;
  WaypointPosition waypoint;
};

ShipPosition TurnShip(ShipPosition pos, int degrees) {
  return {pos.degrees + degrees, pos.x, pos.y};
}

template <typename PositionT>
PositionT Move(PositionT pos, int x, int y) {
  PositionT new_pos = pos;
  new_pos.x += x;
  new_pos.y += y;
  return new_pos;
}

int AbsDegrees(int degrees) {
  if (degrees < 0) {
    degrees = 360 - (-degrees % 360);
  }
  return degrees % 360;
}

template <typename PositionT>
PositionT MoveInDirection(PositionT pos, int degrees, int distance) {
  degrees = AbsDegrees(degrees);
  PositionT new_pos = pos;
  switch (degrees) {
    case 0:
      return Move(new_pos, 0, distance);
    case 90:
      return Move(new_pos, distance, 0);
    case 180:
      return Move(new_pos, 0, -distance);
    case 270:
      return Move(new_pos, -distance, 0);
  }
  CHECK(false) << "Unknown degrees: " << degrees;
}

WaypointPosition RotateWaypoint(WaypointPosition pos, int degrees) {
  degrees = AbsDegrees(degrees);
  WaypointPosition new_pos = pos;
  switch (degrees) {
    case 0:
      break;
    case 90:
      new_pos.x = pos.y;
      new_pos.y = -pos.x;
      break;
    case 180:
      new_pos.x = -pos.x;
      new_pos.y = -pos.y;
      break;
    case 270:
      new_pos.x = -pos.y;
      new_pos.y = pos.x;
      break;
    default:
      CHECK(false) << "Unknown degrees: " << degrees;
  }
  return new_pos;
}

int DegreesForCardinal(char c) {
  switch (c) {
    case 'N':
      return 0;
    case 'E':
      return 90;
    case 'S':
      return 180;
    case 'W':
      return 270;
  }
  CHECK(false) << "Unknown cardinal: " << c;
}

ShipPosition ApplyVectorPart1(ShipPosition pos, Vector v) {
  if (v.type == 'F') {
    return MoveInDirection(pos, pos.degrees, v.distance);
  } else if (v.type == 'L' || v.type == 'R') {
    int direction = v.type == 'L' ? -1 : 1;
    return TurnShip(pos, v.distance * direction);
  } else {
    return MoveInDirection(pos, DegreesForCardinal(v.type), v.distance);
  }
}

ShipAndWaypoint ApplyVectorPart2(ShipAndWaypoint pos, Vector v) {
  auto [ship, waypoint] = pos;
  if (v.type == 'F') {
    // Apply the waypoint offset |distance| times. The Waypoint doesn't move.
    auto new_ship = ship;
    new_ship.x += waypoint.x * v.distance;
    new_ship.y += waypoint.y * v.distance;
    return {new_ship, waypoint};
  } else if (v.type == 'L' || v.type == 'R') {
    // Rotate the waypoint, ship doesn't move.
    int direction = v.type == 'L' ? -1 : 1;
    return {ship, RotateWaypoint(waypoint, v.distance * direction)};
  } else {
    // Move the waypoint N/E/S/W.
    return {ship,
            MoveInDirection(waypoint, DegreesForCardinal(v.type), v.distance)};
  }
}

template <typename PositionT>
int ManhattanDistance(PositionT p1, PositionT p2) {
  return std::abs(p1.x - p2.x) + std::abs(p1.y - p2.y);
}

int main(int argc, char** argv) {
  google::InstallFailureSignalHandler();
  google::InitGoogleLogging(argv[0]);
  FLAGS_logtostderr = 1;

  std::ifstream file(argv[1]);
  CHECK(file);

  std::vector<Vector> moves;
  std::string line;
  while (std::getline(file, line)) {
    int distance;
    CHECK(absl::SimpleAtoi(line.substr(1), &distance));
    moves.push_back({line[0], distance});
  }

  // Part 1: the moves apply to the ship.
  {
    ShipPosition pos = {90, 0, 0};
    for (auto v : moves) {
      pos = ApplyVectorPart1(pos, v);
    }
    LOG(INFO) << "PART 1: " << ManhattanDistance(pos, {0, 0, 0});
  }
  // Part 2: the moves mostly affect the waypoint.
  {
    ShipAndWaypoint pos = {{0, 0, 0}, {10, 1}};
    for (auto v : moves) {
      pos = ApplyVectorPart2(pos, v);
    }
    LOG(INFO) << "PART 2: " << ManhattanDistance(pos.ship, {0, 0, 0});
  }

  return 0;
}
