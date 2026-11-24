#include "exercise_catalog.h"
#include "exercise_session.h"
#include "product_info.h"
#include "routine_planner.h"

#include <filesystem>
#include <iostream>
#include <set>
#include <string>

using namespace workledge::core;

namespace {

int failures = 0;

void
check(bool condition, const std::string &name)
{
  std::cout << (condition ? "[PASS] " : "[FAIL] ") << name << '\n';
  if (!condition)
    {
      ++failures;
    }
}

std::string
default_data_path()
{
  std::string file = __FILE__;
  const auto slash = file.find_last_of("/\\");
  if (slash != std::string::npos)
    {
      return file.substr(0, slash) + "/../data/test_exercises.tsv";
    }
  return "data/test_exercises.tsv";
}

bool
contains(const PlanResult &plan, const std::string &id)
{
  for (const auto &item : plan.items)
    {
      if (item.id == id)
        {
          return true;
        }
    }
  return false;
}

} // namespace

int
main(int argc, char **argv)
{
  const std::string data_path = argc > 1 ? argv[1] : default_data_path();

  ExerciseCatalog catalog;
  catalog.load_tsv(data_path);

  // --- catalog robustness -------------------------------------------------
  check(catalog.records().size() >= 10, "catalog loads usable test fixtures");
  check(!catalog.warnings().empty(), "catalog records warnings instead of aborting");
  check(catalog.find("test_incomplete") == nullptr, "incomplete row is skipped");
  const auto *bad_media = catalog.find("test_bad_media");
  check(bad_media != nullptr && bad_media->media.file_missing,
        "missing media is flagged but the record survives");
  const auto *draft = catalog.find("test_draft_neck");
  check(draft != nullptr && draft->review_status == ReviewStatus::Draft,
        "unknown/unreviewed content is not approved by default");
  check(std::string(kDisplayName).find("مکثا") != std::string::npos && kNameIsProvisional,
        "provisional display name lives in one central header");

  // --- planner: regions, drafts, time -------------------------------------
  PlannerInput neck;
  neck.regions = {BodyRegion::Neck};
  neck.available_seconds = 120;
  const auto neck_plan = plan_routine(catalog, neck);
  check(!contains(neck_plan, "test_draft_neck"), "draft exercises never enter normal selection");
  check(neck_plan.items.size() >= 2, "planner fills the time with more than one movement");
  check(neck_plan.total_seconds <= 120, "planner never exceeds available time");

  PlannerInput multi;
  multi.regions = {BodyRegion::Neck, BodyRegion::Shoulder};
  multi.available_seconds = 120;
  const auto multi_plan = plan_routine(catalog, multi);
  check(multi_plan.items.size() >= 2, "multiple selected body regions are combined");

  PlannerInput whole;
  whole.whole_body = true;
  whole.available_seconds = 30;
  const auto whole_plan = plan_routine(catalog, whole);
  std::set<BodyRegion> covered;
  for (const auto &item : whole_plan.items)
    {
      const auto *record = catalog.find(item.id);
      if (record != nullptr)
        {
          for (auto region : record->body_regions)
            {
              covered.insert(region);
            }
        }
    }
  check(whole_plan.items.size() >= 2 && covered.size() >= 2,
        "whole-body mode diversifies across regions");

  // --- planner: hard constraints ------------------------------------------
  PlannerInput seated;
  seated.whole_body = true;
  seated.available_seconds = 90;
  const auto seated_plan = plan_routine(catalog, seated);
  check(!contains(seated_plan, "test_floor_hip") && !contains(seated_plan, "test_chair_support"),
        "floor and equipment movements are excluded by default constraints");

  PlannerInput favorite;
  favorite.regions = {BodyRegion::LowerBack};
  favorite.favorites.insert("test_chair_support");
  favorite.available_seconds = 60;
  const auto favorite_plan = plan_routine(catalog, favorite);
  check(!contains(favorite_plan, "test_chair_support"),
        "favourites cannot bypass equipment or position constraints");

  PlannerInput recent;
  recent.regions = {BodyRegion::Neck};
  recent.available_seconds = 30;
  recent.recent_ids = {"test_bad_media"};
  const auto recent_plan = plan_routine(catalog, recent);
  check(!recent_plan.items.empty() && recent_plan.items.front().id == "test_neck_tilt",
        "recently shown movements are deprioritised");

  PlannerInput too_short;
  too_short.regions = {BodyRegion::Neck};
  too_short.available_seconds = 3;
  const auto short_plan = plan_routine(catalog, too_short);
  check(short_plan.items.empty() && short_plan.reason == NoCandidateReason::NotEnoughTime,
        "a movement that does not fit is skipped, not shortened");

  PlannerInput hidden;
  hidden.regions = {BodyRegion::Neck};
  hidden.available_seconds = 60;
  hidden.hidden_ids = {"test_bad_media", "test_neck_tilt"};
  const auto hidden_plan = plan_routine(catalog, hidden);
  check(hidden_plan.items.empty() && hidden_plan.reason == NoCandidateReason::AllHidden,
        "hiding all candidates produces an explicit reason");

  // --- session state machine ----------------------------------------------
  ExerciseSession session(3);
  check(session.state() == SessionState::Idle, "session starts Idle");
  check(session.apply(SessionEvent::Start) && session.state() == SessionState::Preparing, "Start -> Preparing");
  check(session.apply(SessionEvent::Ready) && session.state() == SessionState::Playing, "Ready -> Playing");
  check(session.apply(SessionEvent::Pause) && session.state() == SessionState::Paused, "Pause -> Paused");
  check(session.apply(SessionEvent::Resume) && session.state() == SessionState::Playing, "Resume -> Playing");

  const auto state_before_reopen = session.state();
  const auto index_before_reopen = session.index();
  check(session.apply(SessionEvent::ReopenCard) && session.state() == state_before_reopen
          && session.index() == index_before_reopen,
        "reopening the card does not create or reset a session");

  check(session.apply(SessionEvent::Next) && session.state() == SessionState::Transitioning,
        "Next -> Transitioning");
  check(session.apply(SessionEvent::Ready) && session.index() == 1 && session.state() == SessionState::Playing,
        "Ready advances the movement index");
  check(session.apply(SessionEvent::Skip) && session.apply(SessionEvent::Ready) && session.index() == 2,
        "Skip advances to the next movement");
  check(session.apply(SessionEvent::Next) && session.state() == SessionState::Completed,
        "Next on the last movement completes the session");
  check(!session.apply(SessionEvent::Ready), "illegal transitions are rejected");

  ExerciseSession cancelled(1);
  cancelled.apply(SessionEvent::Start);
  cancelled.apply(SessionEvent::Ready);
  check(cancelled.apply(SessionEvent::Cancel) && cancelled.state() == SessionState::Cancelled,
        "Cancel ends the session");

  std::cout << '\n' << (failures == 0 ? "ALL TESTS PASSED" : "TEST FAILURES: " + std::to_string(failures)) << '\n';
  return failures == 0 ? 0 : 1;
}
