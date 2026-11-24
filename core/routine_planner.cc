#include "routine_planner.h"

#include <algorithm>
#include <optional>

namespace workledge::core {
namespace {

bool
matches_region(const ExerciseRecord &record, const PlannerInput &input)
{
  if (input.whole_body)
    {
      return !record.body_regions.empty();
    }
  for (auto region : input.regions)
    {
      if (record.has_region(region))
        {
          return true;
        }
    }
  return false;
}

bool
has_position(const ExerciseRecord &record, Position position)
{
  return std::find(record.positions.begin(), record.positions.end(), position) != record.positions.end();
}

bool
matches_constraints(const ExerciseRecord &record, const ExecutionConstraints &c)
{
  if (c.seated_only && !has_position(record, Position::Seated))
    {
      return false;
    }
  if (!c.allow_standing && has_position(record, Position::Standing))
    {
      return false;
    }
  if (!c.allow_floor && (has_position(record, Position::Floor) || record.requires_floor))
    {
      return false;
    }
  if (c.seated_only && !record.desk_friendly)
    {
      return false;
    }

  for (auto equipment : record.equipment)
    {
      if (equipment == Equipment::None)
        {
          continue;
        }
      if (c.no_equipment_only)
        {
          return false;
        }
      if (equipment == Equipment::Chair && !c.chair_available)
        {
          return false;
        }
      if (equipment == Equipment::Wall && !c.wall_available)
        {
          return false;
        }
    }
  return true;
}

int
score(const ExerciseRecord &record, const PlannerInput &input,
       const std::set<BodyRegion> &already_covered)
{
  int value = 100;

  if (std::find(input.recent_ids.begin(), input.recent_ids.end(), record.id) != input.recent_ids.end())
    {
      value -= 40;
    }

  // Favourites only add a small bonus after all hard constraints passed.
  if (input.favorites.count(record.id) != 0)
    {
      value += 10;
    }

  if (input.whole_body)
    {
      for (auto region : record.body_regions)
        {
          if (already_covered.count(region) != 0)
            {
              value -= 15;
            }
        }
    }

  return value;
}

} // namespace

PlanResult
plan_routine(const ExerciseCatalog &catalog, const PlannerInput &input)
{
  PlanResult result;
  std::size_t approved_count = 0;
  std::size_t after_hidden = 0;
  std::size_t hidden_matching = 0;
  std::size_t region_count = 0;
  std::size_t constraint_count = 0;

  std::vector<const ExerciseRecord *> eligible;

  for (const auto &record : catalog.records())
    {
      if (record.review_status != ReviewStatus::Approved)
        {
          continue;
        }
      ++approved_count;
      const bool hidden = input.hidden_ids.count(record.id) != 0;
      if (hidden && matches_region(record, input))
        {
          ++hidden_matching;
        }
      if (hidden)
        {
          continue;
        }
      ++after_hidden;
      if (!matches_region(record, input))
        {
          continue;
        }
      ++region_count;
      if (!matches_constraints(record, input.constraints))
        {
          continue;
        }
      ++constraint_count;
      eligible.push_back(&record);
    }

  if (approved_count == 0)
    {
      result.reason = NoCandidateReason::NoApprovedContent;
      result.message = "No reviewed and approved exercise content is available.";
      return result;
    }
  if (after_hidden == 0)
    {
      result.reason = NoCandidateReason::AllHidden;
      result.message = "All approved exercises are hidden by the user.";
      return result;
    }
  if (region_count == 0)
    {
      if (hidden_matching > 0)
        {
          result.reason = NoCandidateReason::AllHidden;
          result.message = "All approved exercises for the selected regions are hidden by the user.";
        }
      else
        {
          result.reason = NoCandidateReason::NoMatchForRegions;
          result.message = "No approved exercise matches the selected body regions.";
        }
      return result;
    }
  if (constraint_count == 0)
    {
      result.reason = NoCandidateReason::NoMatchForConstraints;
      result.message = "No approved exercise matches the selected execution conditions.";
      return result;
    }

  std::set<BodyRegion> covered;
  std::set<std::string> used;
  int remaining = input.available_seconds;

  for (;;)
    {
      const ExerciseRecord *best = nullptr;
      int best_score = 0;

      for (const auto *record : eligible)
        {
          if (used.count(record->id) != 0)
            {
              continue;
            }
          const int need = record->duration.estimated_seconds();
          if (need > remaining)
            {
              continue; // never shorten a defined duration
            }
          const int candidate_score = score(*record, input, covered);
          if (best == nullptr || candidate_score > best_score
              || (candidate_score == best_score && record->id < best->id))
            {
              best = record;
              best_score = candidate_score;
            }
        }

      if (best == nullptr)
        {
          if (result.items.empty())
            {
              result.reason = NoCandidateReason::NotEnoughTime;
              result.message = "No approved exercise fits into the available time.";
            }
          break;
        }

      const int seconds = best->duration.estimated_seconds();
      result.items.push_back({best->id, seconds, best->duration.transition_seconds});
      result.total_seconds += seconds;
      remaining -= seconds;
      used.insert(best->id);
      for (auto region : best->body_regions)
        {
          covered.insert(region);
        }
    }

  if (result.items.empty() && result.reason == NoCandidateReason::None)
    {
      result.reason = NoCandidateReason::NotEnoughTime;
      result.message = "No approved exercise fits into the available time.";
    }
  return result;
}

const char *
to_string(NoCandidateReason reason)
{
  switch (reason)
    {
    case NoCandidateReason::None: return "none";
    case NoCandidateReason::NoApprovedContent: return "no_approved_content";
    case NoCandidateReason::AllHidden: return "all_hidden";
    case NoCandidateReason::NoMatchForRegions: return "no_match_for_regions";
    case NoCandidateReason::NoMatchForConstraints: return "no_match_for_constraints";
    case NoCandidateReason::NotEnoughTime: return "not_enough_time";
    }
  return "unknown";
}

} // namespace workledge::core
