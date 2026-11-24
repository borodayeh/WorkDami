#pragma once

#include "exercise_model.h"
#include "routine_planner.h"

#include <map>
#include <set>
#include <string>
#include <vector>

namespace workledge::core {

enum class BreakKind { Micro, Rest, Daily };

struct Settings {
  std::set<BodyRegion> selected_regions;
  bool whole_body{false};
  // Explicitly different from whole_body: "select all regions" is a list,
  // "whole body" is a mode that asks the planner to diversify.
  bool all_regions_selected{false};
  bool seated_only{true};
  bool allow_standing{false};
  bool allow_floor{false};
  std::set<Equipment> allowed_equipment{Equipment::None};
  std::set<std::string> hidden_ids;
  std::map<BreakKind, bool> exercises_enabled{
    {BreakKind::Micro, false}, {BreakKind::Rest, true}, {BreakKind::Daily, false}};

  ExecutionConstraints constraints() const
  {
    ExecutionConstraints c;
    c.seated_only = seated_only;
    c.allow_standing = allow_standing;
    c.allow_floor = allow_floor;
    c.chair_available = allowed_equipment.count(Equipment::Chair) != 0;
    c.wall_available = allowed_equipment.count(Equipment::Wall) != 0;
    c.no_equipment_only = allowed_equipment.size() == 1
                          && allowed_equipment.count(Equipment::None) != 0;
    return c;
  }
};

// Core stays independent of Workrave's IConfigurator. The product backend is
// WorkraveConfigStore (see workrave_config_backend.h): it writes the same
// values under gui/maksa/... using workrave::config::IConfigurator, which on
// Windows is W32Configurator. No parallel, incompatible product file is used.
class ISettingsStore
{
public:
  virtual ~ISettingsStore() = default;
  virtual bool load(Settings &out, std::vector<std::string> &warnings) = 0;
  virtual bool save(const Settings &in, std::string &error) = 0;
};

class FileSettingsStore : public ISettingsStore
{
public:
  explicit FileSettingsStore(std::string path);

  bool load(Settings &out, std::vector<std::string> &warnings) override;
  bool save(const Settings &in, std::string &error) override;

  const std::string &path() const { return path_; }

private:
  std::string path_;
};

PlannerInput to_planner_input(const Settings &settings, int available_seconds);

const char *to_string(BreakKind kind);

} // namespace workledge::core
