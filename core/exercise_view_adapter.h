#pragma once

#include "exercise_model.h"

// DTO-only adapter. This translation unit does NOT include or create any
// Workrave type. The real conversion lives in integration/workrave_exercise_bridge.cc
// and must be compiled together with Workrave sources; it is unverified here.

#include <string>
#include <vector>

namespace workledge::core {

// DTO decision (after inspecting Workrave):
//
// Existing model: ui/libs/commonui/include/commonui/Exercise.hh defines
//   Exercise { title, description, duration, sequence<Image{image,duration,mirror_x}> }
//   ExerciseCollection loads XML (Exercise.cc::parse_exercises) and returns a list.
// Existing players: ui/app/toolkits/qt/QmlRestBreakWindow.cc::RestBreakBridge::initExercises
//   and ui/app/toolkits/qt/QmlExercisesDialog.cc::ExercisesBridge both shuffle
//   Exercise objects and run their own QTimer.
//
// This model cannot hold body regions, execution conditions, review status,
// licence or attribution. Replacing it would fork two existing players;
// extending it needs Qt/Workrave rebuilds we cannot verify here.
//
// Chosen path: keep the new core behind a limited interface and add a SMALL
// DTO adapter that describes the exact fields the existing Exercise consumes,
// while an explicit unmapped_fields list records everything the old model
// cannot represent. Nothing is silently dropped.

struct ExerciseViewDto {
  std::string title;
  std::string description;
  int duration_seconds{0};

  struct Step {
    Side side{Side::Unspecified};
    std::string media;
    std::string text;
    int seconds{0};
    int repetitions{0};
    // Mirroring is never inferred from bilateral timing. It is true only when
    // the source explicitly allows it AND this is a non-primary side step.
    bool mirror_x{false};
  };
  std::vector<Step> sequence;
};

// The source pointer must outlive the result: the product keeps the catalog alive
// for the whole break so source/license/attribution/reviewStatus stay reachable.
struct ExerciseViewResult {
  ExerciseViewDto view;
  const ExerciseRecord *source{nullptr};
  std::string locale;
  std::vector<std::string> unmapped_fields;
};

ExerciseViewResult make_exercise_view(const ExerciseRecord &record, const std::string &locale);

} // namespace workledge::core
