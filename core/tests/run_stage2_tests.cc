#include "coordinator.h"
#include "settings.h"
#include "exercise_view_adapter.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
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

std::string
temp_path(const std::string &name)
{
  const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
  return (std::filesystem::temp_directory_path()
          / ("workledge_" + name + "_" + std::to_string(stamp) + ".txt"))
           .string();
}

class FailingStore : public ISettingsStore
{
public:
  bool load(Settings &, std::vector<std::string> &) override { return false; }
  bool save(const Settings &, std::string &error) override
  {
    error = "simulated write failure";
    return false;
  }
};

Settings
neck_and_shoulder()
{
  Settings s;
  s.selected_regions = {BodyRegion::Neck, BodyRegion::Shoulder};
  s.whole_body = false;
  s.all_regions_selected = false;
  s.seated_only = true;
  s.allow_floor = false;
  s.allowed_equipment = {Equipment::None};
  s.exercises_enabled[BreakKind::Rest] = true;
  return s;
}

} // namespace

int
main(int argc, char **argv)
{
  const std::string data_path = argc > 1 ? argv[1] : default_data_path();
  ExerciseCatalog catalog;
  catalog.load_tsv(data_path);

  // --- persistence --------------------------------------------------------
  const auto path = temp_path("settings");
  FileSettingsStore store(path);
  Settings first = neck_and_shoulder();
  first.hidden_ids.insert("test_bad_media");

  std::string save_error;
  check(store.save(first, save_error) && save_error.empty(), "settings save succeeds");

  Settings restored;
  std::vector<std::string> warnings;
  check(store.load(restored, warnings) && warnings.empty(), "settings reload succeeds");
  check(restored.selected_regions == first.selected_regions
          && restored.whole_body == first.whole_body
          && restored.seated_only == first.seated_only
          && restored.allowed_equipment == first.allowed_equipment
          && restored.hidden_ids == first.hidden_ids
          && restored.exercises_enabled == first.exercises_enabled,
        "round-trip preserves all required settings");

  FileSettingsStore missing(temp_path("missing"));
  Settings defaults;
  warnings.clear();
  check(missing.load(defaults, warnings) && !warnings.empty()
          && defaults.selected_regions.empty(),
        "missing settings file yields defaults with a warning");

  const auto corrupt_path = temp_path("corrupt");
  {
    std::ofstream out(corrupt_path);
    out << "version=1\nregions=neck\nseated_only=maybe\nwhole_body=true\n";
  }
  FileSettingsStore corrupt(corrupt_path);
  Settings partial;
  warnings.clear();
  const bool corrupt_ok = corrupt.load(partial, warnings);
  check(!corrupt_ok && !warnings.empty() && partial.selected_regions.count(BodyRegion::Neck) != 0
          && partial.seated_only,
        "corrupt value is reported; valid fields are still loaded");

  const auto unknown_path = temp_path("unknown");
  {
    std::ofstream out(unknown_path);
    out << "bogus_key=1\nregions=neck,unknown_region\n";
  }
  FileSettingsStore unknown(unknown_path);
  Settings unknown_settings;
  warnings.clear();
  check(!unknown.load(unknown_settings, warnings)
          && warnings.size() >= 2
          && unknown_settings.selected_regions.count(BodyRegion::Neck) != 0,
        "unknown key and unknown value are both reported");

  const auto preserved_path = temp_path("preserve");
  FileSettingsStore preserved_store(preserved_path);
  Settings old_settings = neck_and_shoulder();
  preserved_store.save(old_settings, save_error);
  FailingStore failing;
  check(!failing.save(first, save_error) && save_error == "simulated write failure",
        "failed save returns false with an error");
  Settings still_old;
  warnings.clear();
  check(preserved_store.load(still_old, warnings) && still_old.selected_regions == old_settings.selected_regions,
        "old settings survive a failed save");

  Settings all_regions = neck_and_shoulder();
  all_regions.all_regions_selected = true;
  all_regions.whole_body = false;
  Settings whole_mode = neck_and_shoulder();
  whole_mode.all_regions_selected = false;
  whole_mode.whole_body = true;
  check(all_regions.all_regions_selected != all_regions.whole_body
          && whole_mode.whole_body != whole_mode.all_regions_selected,
        "select-all-regions and whole-body mode remain distinct");

  // --- planner edge cases through settings --------------------------------
  Settings no_region;
  no_region.selected_regions.clear();
  const auto no_region_plan = plan_routine(catalog, to_planner_input(no_region, 60));
  check(no_region_plan.items.empty()
          && no_region_plan.reason == NoCandidateReason::NoMatchForRegions,
        "no selected region produces an explicit reason");

  Settings chest_only;
  chest_only.selected_regions = {BodyRegion::Chest};
  chest_only.seated_only = true;
  chest_only.allow_standing = false;
  const auto chest_plan = plan_routine(catalog, to_planner_input(chest_only, 60));
  check(chest_plan.items.empty()
          && chest_plan.reason == NoCandidateReason::NoMatchForConstraints,
        "no movement matching the conditions produces an explicit reason");

  Settings hidden_all = neck_and_shoulder();
  hidden_all.selected_regions = {BodyRegion::Neck};
  hidden_all.hidden_ids = {"test_bad_media", "test_neck_tilt"};
  const auto hidden_plan = plan_routine(catalog, to_planner_input(hidden_all, 60));
  check(hidden_plan.items.empty() && hidden_plan.reason == NoCandidateReason::AllHidden,
        "all suitable movements hidden produces an explicit reason");

  const auto tiny_plan = plan_routine(catalog, to_planner_input(neck_and_shoulder(), 2));
  check(tiny_plan.items.empty() && tiny_plan.reason == NoCandidateReason::NotEnoughTime,
        "movement that does not fit is skipped without shortening");

  Settings limited_whole = neck_and_shoulder();
  limited_whole.whole_body = true;
  limited_whole.selected_regions.clear();
  const auto limited_plan = plan_routine(catalog, to_planner_input(limited_whole, 30));
  std::set<BodyRegion> limited_regions;
  for (const auto &item : limited_plan.items)
    {
      const auto *record = catalog.find(item.id);
      if (record != nullptr)
        {
          for (auto region : record->body_regions)
            {
              limited_regions.insert(region);
            }
        }
    }
  check(limited_plan.items.size() >= 2 && limited_regions.size() >= 2,
        "whole-body mode with limited data still diversifies");

  // --- ownership / coordinator --------------------------------------------
  const auto base_plan = plan_routine(catalog, to_planner_input(neck_and_shoulder(), 60));
  BreakExerciseCoordinator coordinator(catalog);
  coordinator.set_plan(base_plan);

  check(coordinator.on_break_signal(BreakSignal::ShowBreak, 1, 0)
          && coordinator.active() && coordinator.presentation() != nullptr,
        "break start creates one presentation");
  const int first_session = coordinator.session_id();
  check(!coordinator.on_break_signal(BreakSignal::ShowBreak, 1, 1)
          && coordinator.session_id() == first_session,
        "duplicate break start does not create a second session");

  coordinator.tick(2);
  coordinator.on_system_resume(5, 2); // short event-loop delay: 2 s, below the 5 s threshold
  coordinator.tick(6);
  check(coordinator.state() == SessionState::Playing
          && coordinator.ledger().elapsed == 3
          && !coordinator.long_gap_seen(),
        "short event-loop delay keeps playing and still counts real seconds");

  coordinator.tick(4); // simulated wall-clock jump backwards
  check(coordinator.ledger().elapsed == 3,
        "monotonic accounting ignores a backwards clock change");

  coordinator.on_system_resume(100, 50); // suspend/resume: 50 s gap
  coordinator.tick(101);
  check(coordinator.ledger().elapsed == 3
          && coordinator.ledger().confirmed == 0
          && coordinator.state() == SessionState::Paused
          && coordinator.presentation() != nullptr
          && coordinator.presentation()->paused
          && coordinator.long_gap_seen(),
        "long suspend pauses the session and grants no elapsed/confirmed credit");

  check(coordinator.command(ExerciseCommand::Resume, 102), "resume after suspend is explicit");
  coordinator.tick(103);
  check(coordinator.state() == SessionState::Playing && coordinator.ledger().elapsed == 4,
        "time accounting resumes only after explicit Resume");

  check(coordinator.command(ExerciseCommand::Pause, 104)
          && coordinator.presentation() != nullptr
          && coordinator.presentation()->paused,
        "pausing a movement is reflected in the presentation");
  coordinator.on_system_resume(200, 60);
  coordinator.tick(201);
  check(coordinator.state() == SessionState::Paused && coordinator.ledger().elapsed == 4,
        "resume while already paused stays paused and grants no credit");

  check(coordinator.command(ExerciseCommand::CloseCard, 202)
          && coordinator.active(),
        "closing the card is not an end-of-session command");
  check(coordinator.on_break_signal(BreakSignal::Cancel, 1, 203)
          && !coordinator.active() && coordinator.presentation() == nullptr
          && coordinator.state() == SessionState::Cancelled,
        "cancel while paused clears the exercise session even if UI was inactive");

  BreakExerciseCoordinator playing(catalog);
  playing.set_plan(base_plan);
  playing.on_break_signal(BreakSignal::ShowBreak, 2, 0);
  check(playing.on_break_signal(BreakSignal::BreakTaken, 2, 1)
          && !playing.active() && playing.presentation() == nullptr,
        "end of break clears the exercise session");

  // --- full non-GUI path --------------------------------------------------
  const auto e2e_path = temp_path("e2e");
  FileSettingsStore e2e_store(e2e_path);
  Settings chosen = neck_and_shoulder();
  check(e2e_store.save(chosen, save_error), "e2e: settings saved");
  Settings reopened;
  warnings.clear();
  check(e2e_store.load(reopened, warnings)
          && reopened.selected_regions == chosen.selected_regions,
        "e2e: selections recovered after close/reopen");

  const auto e2e_plan = plan_routine(catalog, to_planner_input(reopened, 60));
  BreakExerciseCoordinator e2e(catalog);
  e2e.set_plan(e2e_plan);
  check(e2e.on_break_signal(BreakSignal::ShowBreak, 3, 0)
          && e2e.presentation() != nullptr
          && !e2e.presentation()->title.empty(),
        "e2e: break start shows a movement from the plan");
  check(e2e.command(ExerciseCommand::Pause, 1)
          && e2e.command(ExerciseCommand::Resume, 2),
        "e2e: pause and resume work");
  check(e2e.command(ExerciseCommand::Skip, 3)
          && e2e.ledger().confirmed == 0,
        "e2e: skipping does not count as confirmed");
  check(e2e.on_break_signal(BreakSignal::BreakTaken, 3, 4)
          && !e2e.active() && e2e.presentation() == nullptr,
        "e2e: break end clears the session");

  // --- DTO adapter (not a Workrave connection) -----------------------------
  const auto *record = catalog.find("test_bad_media");
  check(record != nullptr, "dto: fixture exists");
  if (record != nullptr)
    {
      const auto mapped = make_exercise_view(*record, "fa");
      check(mapped.view.title == "رها کردن گردن (آزمایشی)",
            "dto: Persian localized title is used");
      check(mapped.view.duration_seconds == record->duration.estimated_seconds()
              && mapped.view.sequence.size() == 3,
            "dto: duration and explicit side steps are preserved");
      check(mapped.view.sequence[0].side == Side::Right
              && mapped.view.sequence[1].side == Side::Left
              && !mapped.view.sequence[0].mirror_x
              && !mapped.view.sequence[1].mirror_x,
            "dto: bilateral timing does not imply media mirroring");
      check(mapped.source == record && mapped.source->license == "TEST_FIXTURE"
              && mapped.source->attribution == "TEST_FIXTURE"
              && mapped.source->review_status == ReviewStatus::Approved,
            "dto: original record keeps licence, attribution and review status alive");
      bool found_regions = false;
      bool found_license = false;
      for (const auto &field : mapped.unmapped_fields)
        {
          if (field == "bodyRegions") found_regions = true;
          if (field == "license") found_license = true;
        }
      check(found_regions && found_license,
            "dto: fields unknown to Exercise are reported, not silently dropped");
    }

  const auto *shoulder = catalog.find("test_shoulder_roll");
  check(shoulder != nullptr, "dto: shoulder fixture exists");
  if (shoulder != nullptr)
    {
      const auto mapped = make_exercise_view(*shoulder, "fa");
      check(mapped.view.sequence.size() >= 2
              && mapped.view.sequence[1].mirror_x
              && mapped.view.sequence[1].text.find("دیگر") != std::string::npos,
            "dto: mirroring and second-side text require explicit data");
    }

  const auto *wrist = catalog.find("test_wrist_stretch");
  check(wrist != nullptr, "dto: wrist fixture exists");
  if (wrist != nullptr)
    {
      const auto mapped = make_exercise_view(*wrist, "en");
      bool any_mirror = false;
      for (const auto &step : mapped.view.sequence)
        {
          if (step.mirror_x) any_mirror = true;
        }
      check(!any_mirror && mapped.view.sequence.size() == 2,
            "dto: bilateral flag with no side data is not guessed");
    }

  std::cout << '\n' << (failures == 0 ? "ALL STAGE 2 TESTS PASSED" : "STAGE 2 FAILURES: " + std::to_string(failures)) << '\n';
  return failures == 0 ? 0 : 1;
}
