#pragma once

#include "exercise_catalog.h"

#include <set>
#include <string>
#include <vector>

namespace workledge::core {

struct ExecutionConstraints {
  bool seated_only{true};
  bool allow_standing{false};
  bool allow_floor{false};
  bool chair_available{false};
  bool wall_available{false};
  bool no_equipment_only{true};
};

struct PlannerInput {
  std::set<BodyRegion> regions;
  bool whole_body{false};
  ExecutionConstraints constraints;
  int available_seconds{0};
  std::set<std::string> hidden_ids;
  std::set<std::string> favorites;
  std::vector<std::string> recent_ids;
};

enum class NoCandidateReason {
  None,
  NoApprovedContent,
  AllHidden,
  NoMatchForRegions,
  NoMatchForConstraints,
  NotEnoughTime,
};

struct PlannedExercise {
  std::string id;
  int seconds{0};
  int transition_seconds{0};
};

struct PlanResult {
  std::vector<PlannedExercise> items;
  int total_seconds{0};
  NoCandidateReason reason{NoCandidateReason::None};
  std::string message;
};

// Pure logic: no GUI, no timers, no Qt. Duration definitions are never shortened.
PlanResult plan_routine(const ExerciseCatalog &catalog, const PlannerInput &input);

const char *to_string(NoCandidateReason reason);

} // namespace workledge::core
