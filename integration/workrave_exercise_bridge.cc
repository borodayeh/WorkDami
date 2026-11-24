#include "workrave_exercise_bridge.h"

#include <string>

namespace workledge::integration {

bool
bridge_exercise(const workledge::core::ExerciseViewDto &dto,
                const workledge::core::ExerciseRecord *source,
                BridgedExercise &out)
{
  if (source == nullptr)
    {
      return false;
    }

  out.source = source;
  out.value.title = dto.title;
  out.value.description = dto.description;
  out.value.duration = dto.duration_seconds;
  out.value.sequence.clear();

  // Workrave::Exercise::Image has {image, duration, mirror_x}; it has no side
  // or repetition fields. Repetition-based movements are marked lossy and the
  // original record remains reachable through `source`.
  for (const auto &step : dto.sequence)
    {
      const int seconds_or_reps = step.seconds > 0 ? step.seconds : step.repetitions;
      if (step.repetitions > 0)
        {
          out.lossy = true;
        }
      out.value.sequence.emplace_back(step.media, seconds_or_reps, step.mirror_x);
    }

  return !out.value.sequence.empty();
}

} // namespace workledge::integration
