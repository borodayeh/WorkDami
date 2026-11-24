#pragma once

// UNVERIFIED integration unit. This file MUST be compiled together with
// Workrave sources (include dirs: workrave/ui/libs/commonui/include, etc.).
// It is deliberately NOT part of the g++ core tests.

#include "../core/exercise_view_adapter.h"
#include "commonui/Exercise.hh"

namespace workledge::integration {

struct BridgedExercise {
  workrave::Exercise value;
  // Kept for the whole session: licence, attribution, source and reviewStatus
  // are not representable in workrave::Exercise.
  const workledge::core::ExerciseRecord *source{nullptr};
  bool lossy{false};
};

bool bridge_exercise(const workledge::core::ExerciseViewDto &dto,
                     const workledge::core::ExerciseRecord *source,
                     BridgedExercise &out);

} // namespace workledge::integration
