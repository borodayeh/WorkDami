#include "exercise_catalog.h"
#include "exercise_session.h"
#include "product_info.h"
#include "routine_planner.h"

#include <iostream>
#include <string>

using namespace workledge::core;

int
main(int argc, char **argv)
{
  const std::string path = argc > 1 ? argv[1] : "data/test_exercises.tsv";

  ExerciseCatalog catalog;
  catalog.load_tsv(path);

  PlannerInput input;
  input.regions = {BodyRegion::Neck, BodyRegion::Shoulder, BodyRegion::HandWristForearm};
  input.available_seconds = 60;

  std::cout << "Product (provisional): " << kDisplayName << '\n';
  std::cout << "Fixture file: " << path << '\n';
  std::cout << "Records loaded: " << catalog.records().size()
            << " | warnings: " << catalog.warnings().size() << '\n';

  const auto plan = plan_routine(catalog, input);
  if (plan.items.empty())
    {
      std::cout << "No plan: " << to_string(plan.reason) << " — " << plan.message << '\n';
      return 0;
    }

  std::cout << "Planned routine (" << plan.total_seconds << "s):\n";
  int number = 0;
  for (const auto &item : plan.items)
    {
      const auto *record = catalog.find(item.id);
      const auto *text = record != nullptr ? record->text_for("fa") : nullptr;
      const std::string title = text != nullptr ? text->title : item.id;
      std::cout << "  " << ++number << ". " << title << " [" << item.id << "] "
                << item.seconds << "s (transition " << item.transition_seconds << "s)\n";
    }

  ExerciseSession session(plan.items.size());
  session.apply(SessionEvent::Start);
  std::cout << "Session state: " << to_string(session.state()) << '\n';
  return 0;
}
