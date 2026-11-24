#pragma once

// Data model for the exercise library. Pure C++17, no Qt, no QML.
// Extends Workrave's existing Exercise{title,description,duration,sequence}
// rather than replacing it. Display body regions are NOT muscle groups.

#include <string>
#include <vector>

namespace workledge::core {

// Display categories chosen by the user. One movement may belong to several.
enum class BodyRegion {
  Neck,
  Shoulder,
  UpperBack,
  LowerBack,
  HandWristForearm,
  Chest,
  Pelvis,
  Legs,
  WholeBody,
};

enum class Position { Seated, Standing, Floor };
enum class Equipment { None, Chair, Wall, Other };

// Side of execution is independent of media mirroring and of bilateral timing.
enum class Side { Unspecified, Left, Right };
enum class ExecutionMode { Timed, Repetition, BilateralTimed };

// Default is intentionally NOT approved.
enum class ReviewStatus { Draft, NeedsReview, Approved };

struct DurationSpec {
  ExecutionMode mode{ExecutionMode::Timed};
  int hold_seconds{0};            // per side for Timed / BilateralTimed
  int repetitions{0};             // Repetition mode only
  int seconds_per_repetition{0};  // Repetition mode only
  bool bilateral{false};
  int transition_seconds{0};      // side change / setup, counted in the session

  int estimated_seconds() const
  {
    if (mode == ExecutionMode::Repetition)
      {
        return repetitions * seconds_per_repetition + transition_seconds;
      }
    const int sides = bilateral ? 2 : 1;
    return hold_seconds * sides + (bilateral ? transition_seconds : 0);
  }

  bool valid() const
  {
    if (mode == ExecutionMode::Repetition)
      {
        return repetitions > 0 && seconds_per_repetition > 0 && transition_seconds >= 0;
      }
    return hold_seconds > 0 && transition_seconds >= 0;
  }
};

struct MediaRef {
  std::string path;
  std::string kind;
  bool declared{false};
  bool file_missing{false};
};

struct LocalizedText {
  std::string locale; // "fa", "en", ...
  std::string title;
  std::string instructions;
};

struct ExerciseRecord {
  std::string id;
  std::vector<LocalizedText> texts;
  std::vector<BodyRegion> body_regions;
  std::vector<std::string> primary_muscles;
  std::vector<std::string> secondary_muscles;
  std::vector<Position> positions;
  std::vector<Equipment> equipment;
  bool requires_floor{false};
  bool desk_friendly{true};

  // Explicit, optional side data. Empty side_order means "no side information";
  // the adapter must not invent a second side or infer mirroring from `bilateral`.
  std::vector<Side> side_order;
  bool mirror_media_allowed{false};
  std::string side2_media;
  std::string side2_instructions;

  DurationSpec duration;
  MediaRef media;
  std::string source;
  std::string license;
  std::string attribution;
  ReviewStatus review_status{ReviewStatus::Draft};
  bool test_fixture{false};

  const LocalizedText *text_for(const std::string &locale) const
  {
    for (const auto &t : texts)
      {
        if (t.locale == locale)
          {
            return &t;
          }
      }
    for (const auto &t : texts)
      {
        if (t.locale == "en")
          {
            return &t;
          }
      }
    return texts.empty() ? nullptr : &texts.front();
  }

  bool has_region(BodyRegion region) const
  {
    for (auto r : body_regions)
      {
        if (r == region)
          {
            return true;
          }
      }
    return false;
  }
};

} // namespace workledge::core
