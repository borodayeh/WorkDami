#include "coordinator.h"
#include "settings.h"
#include "exercise_view_adapter.h"

#include <iostream>
#include <string>

using namespace workledge::core;

int
main(int argc, char **argv)
{
  const std::string data_path = argc > 1 ? argv[1] : "data/test_exercises.tsv";
  const std::string settings_path = argc > 2 ? argv[2] : "maksa_settings.txt";

  ExerciseCatalog catalog;
  catalog.load_tsv(data_path);

  Settings selected;
  selected.selected_regions = {BodyRegion::Neck, BodyRegion::Shoulder};
  selected.exercises_enabled[BreakKind::Rest] = true;

  FileSettingsStore store(settings_path);
  std::string error;
  if (!store.save(selected, error))
    {
      std::cerr << "save failed: " << error << '\n';
      return 1;
    }

  Settings reopened;
  std::vector<std::string> warnings;
  store.load(reopened, warnings);

  const auto plan = plan_routine(catalog, to_planner_input(reopened, 60));
  std::cout << "1) selected regions restored: " << reopened.selected_regions.size() << '\n';
  std::cout << "2) plan size: " << plan.items.size() << " (" << plan.total_seconds << "s)\n";

  BreakExerciseCoordinator session(catalog);
  session.set_plan(plan);
  session.on_break_signal(BreakSignal::ShowBreak, 1, 0);
  if (session.presentation() != nullptr)
    {
      std::cout << "3) showing: " << session.presentation()->title
                << " [" << session.presentation()->step_index << "/"
                << session.presentation()->total_steps << "]\n";
    }

  session.command(ExerciseCommand::Pause, 1);
  std::cout << "4) paused: " << (session.presentation() && session.presentation()->paused ? "yes" : "no") << '\n';
  session.on_break_signal(BreakSignal::BreakTaken, 1, 2);
  std::cout << "5) after break end: active=" << session.active()
            << " presentation=" << (session.presentation() == nullptr ? "none" : "still present") << '\n';
  return 0;
}
