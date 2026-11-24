#include "exercise_view_adapter.h"

namespace workledge::core {

ExerciseViewResult
make_exercise_view(const ExerciseRecord &record, const std::string &locale)
{
  ExerciseViewResult result;
  result.source = &record; // lifetime owned by the catalog, not by this DTO
  result.locale = locale;

  const auto *text = record.text_for(locale);
  const auto base_text = text != nullptr ? text->instructions : std::string{};
  result.view.title = text != nullptr ? text->title : record.id;
  result.view.description = base_text;
  result.view.duration_seconds = record.duration.estimated_seconds();

  const auto base_media = record.media.declared ? record.media.path : std::string{"none"};

  // Side handling is explicit. Bilateral timing alone creates no second side
  // and never enables mirroring.
  const std::size_t side_count = record.side_order.empty() ? 1 : record.side_order.size();
  for (std::size_t i = 0; i < side_count; ++i)
    {
      ExerciseViewDto::Step step;
      step.side = record.side_order.empty() ? Side::Unspecified : record.side_order[i];
      step.media = (i == 1 && !record.side2_media.empty()) ? record.side2_media : base_media;
      step.text = (i == 1 && !record.side2_instructions.empty()) ? record.side2_instructions : base_text;
      step.seconds = record.duration.mode == ExecutionMode::Repetition
                       ? 0
                       : record.duration.hold_seconds;
      step.repetitions = record.duration.mode == ExecutionMode::Repetition
                           ? record.duration.repetitions
                           : 0;
      step.mirror_x = (i > 0) && record.mirror_media_allowed;
      result.view.sequence.push_back(step);
    }

  if (record.duration.transition_seconds > 0)
    {
      ExerciseViewDto::Step transition;
      transition.side = Side::Unspecified;
      transition.media = "none";
      transition.text = {};
      transition.seconds = record.duration.transition_seconds;
      transition.mirror_x = false;
      result.view.sequence.push_back(transition);
    }

  // Fields that Exercise cannot represent are surfaced, never discarded.
  // The original record stays reachable through result.source for licensing
  // and publication control.
  result.unmapped_fields = {
    "id",
    "bodyRegions",
    "primaryMuscles",
    "secondaryMuscles",
    "position",
    "equipment",
    "requiresFloor",
    "deskFriendly",
    "executionMode",
    "reviewStatus",
    "source",
    "license",
    "attribution",
    "testFixture",
    "mediaKind",
    "mediaFileMissing",
    "sideOrder",
    "side2Text",
  };

  return result;
}

} // namespace workledge::core
