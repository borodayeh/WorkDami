#include "settings.h"

#include <filesystem>
#include <fstream>
#include <system_error>

namespace workledge::core {
namespace {

std::vector<std::string>
split_csv(const std::string &s)
{
  std::vector<std::string> out;
  std::size_t start = 0;
  for (;;)
    {
      const auto pos = s.find(',', start);
      if (pos == std::string::npos)
        {
          if (!s.substr(start).empty())
            {
              out.push_back(s.substr(start));
            }
          break;
        }
      auto part = s.substr(start, pos - start);
      if (!part.empty())
        {
          out.push_back(part);
        }
      start = pos + 1;
    }
  return out;
}

bool
parse_bool_strict(const std::string &s, bool &out)
{
  if (s == "true" || s == "1") { out = true; return true; }
  if (s == "false" || s == "0") { out = false; return true; }
  return false;
}

std::string
bool_str(bool v)
{
  return v ? "true" : "false";
}

bool
region_from(const std::string &s, BodyRegion &out)
{
  if (s == "neck") { out = BodyRegion::Neck; return true; }
  if (s == "shoulder") { out = BodyRegion::Shoulder; return true; }
  if (s == "upper_back") { out = BodyRegion::UpperBack; return true; }
  if (s == "lower_back") { out = BodyRegion::LowerBack; return true; }
  if (s == "hand_wrist_forearm") { out = BodyRegion::HandWristForearm; return true; }
  if (s == "chest") { out = BodyRegion::Chest; return true; }
  if (s == "pelvis") { out = BodyRegion::Pelvis; return true; }
  if (s == "legs") { out = BodyRegion::Legs; return true; }
  if (s == "whole_body") { out = BodyRegion::WholeBody; return true; }
  return false;
}

std::string
region_name(BodyRegion r)
{
  switch (r)
    {
    case BodyRegion::Neck: return "neck";
    case BodyRegion::Shoulder: return "shoulder";
    case BodyRegion::UpperBack: return "upper_back";
    case BodyRegion::LowerBack: return "lower_back";
    case BodyRegion::HandWristForearm: return "hand_wrist_forearm";
    case BodyRegion::Chest: return "chest";
    case BodyRegion::Pelvis: return "pelvis";
    case BodyRegion::Legs: return "legs";
    case BodyRegion::WholeBody: return "whole_body";
    }
  return "unknown";
}

bool
equipment_from(const std::string &s, Equipment &out)
{
  if (s == "none") { out = Equipment::None; return true; }
  if (s == "chair") { out = Equipment::Chair; return true; }
  if (s == "wall") { out = Equipment::Wall; return true; }
  if (s == "other") { out = Equipment::Other; return true; }
  return false;
}

std::string
equipment_name(Equipment e)
{
  switch (e)
    {
    case Equipment::None: return "none";
    case Equipment::Chair: return "chair";
    case Equipment::Wall: return "wall";
    case Equipment::Other: return "other";
    }
  return "none";
}

} // namespace

FileSettingsStore::FileSettingsStore(std::string path)
  : path_(std::move(path))
{
}

bool
FileSettingsStore::load(Settings &out, std::vector<std::string> &warnings)
{
  std::ifstream in(path_);
  if (!in.is_open())
    {
      warnings.push_back("settings file absent, defaults used: " + path_);
      return true; // first run is not an error
    }

  bool ok = true;
  std::string line;
  while (std::getline(in, line))
    {
      if (!line.empty() && line.back() == '\r')
        {
          line.pop_back();
        }
      if (line.empty() || line[0] == '#')
        {
          continue;
        }
      const auto eq = line.find('=');
      if (eq == std::string::npos)
        {
          warnings.push_back("malformed settings line: " + line);
          ok = false;
          continue;
        }
      const auto key = line.substr(0, eq);
      const auto value = line.substr(eq + 1);

      auto bool_field = [&](bool &target) {
        if (!parse_bool_strict(value, target))
          {
            warnings.push_back("unknown boolean value for " + key + ": " + value);
            ok = false;
          }
      };

      if (key == "version")
        {
          if (value != "1")
            {
              warnings.push_back("unknown settings version: " + value);
              ok = false;
            }
        }
      else if (key == "regions")
        {
          out.selected_regions.clear();
          for (const auto &token : split_csv(value))
            {
              BodyRegion region{};
              if (region_from(token, region))
                {
                  out.selected_regions.insert(region);
                }
              else
                {
                  warnings.push_back("unknown body region: " + token);
                  ok = false;
                }
            }
        }
      else if (key == "whole_body") { bool_field(out.whole_body); }
      else if (key == "all_regions_selected") { bool_field(out.all_regions_selected); }
      else if (key == "seated_only") { bool_field(out.seated_only); }
      else if (key == "allow_standing") { bool_field(out.allow_standing); }
      else if (key == "allow_floor") { bool_field(out.allow_floor); }
      else if (key == "allowed_equipment")
        {
          out.allowed_equipment.clear();
          for (const auto &token : split_csv(value))
            {
              Equipment equipment{};
              if (equipment_from(token, equipment))
                {
                  out.allowed_equipment.insert(equipment);
                }
              else
                {
                  warnings.push_back("unknown equipment: " + token);
                  ok = false;
                }
            }
          if (out.allowed_equipment.empty())
            {
              out.allowed_equipment.insert(Equipment::None);
            }
        }
      else if (key == "hidden")
        {
          out.hidden_ids.clear();
          std::size_t start = 0;
          for (;;)
            {
              const auto pos = value.find('|', start);
              const auto token = value.substr(start, pos == std::string::npos ? std::string::npos : pos - start);
              if (!token.empty())
                {
                  out.hidden_ids.insert(token);
                }
              if (pos == std::string::npos)
                {
                  break;
                }
              start = pos + 1;
            }
        }
      else if (key == "exercise_micro") { bool_field(out.exercises_enabled[BreakKind::Micro]); }
      else if (key == "exercise_rest") { bool_field(out.exercises_enabled[BreakKind::Rest]); }
      else if (key == "exercise_daily") { bool_field(out.exercises_enabled[BreakKind::Daily]); }
      else
        {
          warnings.push_back("unknown settings key: " + key);
          ok = false;
        }
    }
  return ok;
}

bool
FileSettingsStore::save(const Settings &in, std::string &error)
{
  const auto path = std::filesystem::path(path_);
  const auto tmp = std::filesystem::path(path_ + ".tmp");

  std::error_code ec;
  if (!path.has_parent_path() || std::filesystem::is_directory(path.parent_path(), ec))
    {
      // parent is fine; a directory at the target path is handled by rename below.
    }

  std::ofstream out(tmp, std::ios::trunc);
  if (!out.is_open())
    {
      error = "cannot open temporary settings file: " + tmp.string();
      return false;
    }

  auto regions = std::string{};
  for (auto region : in.selected_regions)
    {
      if (!regions.empty()) { regions += ","; }
      regions += region_name(region);
    }
  auto equipment = std::string{};
  for (auto item : in.allowed_equipment)
    {
      if (!equipment.empty()) { equipment += ","; }
      equipment += equipment_name(item);
    }
  auto hidden = std::string{};
  for (const auto &id : in.hidden_ids)
    {
      if (!hidden.empty()) { hidden += "|"; }
      hidden += id;
    }

  out << "version=1\n";
  out << "regions=" << regions << '\n';
  out << "whole_body=" << bool_str(in.whole_body) << '\n';
  out << "all_regions_selected=" << bool_str(in.all_regions_selected) << '\n';
  out << "seated_only=" << bool_str(in.seated_only) << '\n';
  out << "allow_standing=" << bool_str(in.allow_standing) << '\n';
  out << "allow_floor=" << bool_str(in.allow_floor) << '\n';
  out << "allowed_equipment=" << equipment << '\n';
  out << "hidden=" << hidden << '\n';
  out << "exercise_micro=" << bool_str(in.exercises_enabled.at(BreakKind::Micro)) << '\n';
  out << "exercise_rest=" << bool_str(in.exercises_enabled.at(BreakKind::Rest)) << '\n';
  out << "exercise_daily=" << bool_str(in.exercises_enabled.at(BreakKind::Daily)) << '\n';
  out.close();

  if (!out)
    {
      std::error_code ignore;
      std::filesystem::remove(tmp, ignore);
      error = "failed while writing settings";
      return false;
    }

  std::filesystem::rename(tmp, path, ec);
  if (ec)
    {
      std::error_code ignore;
      std::filesystem::remove(tmp, ignore);
      error = "failed to replace settings file: " + ec.message();
      return false;
    }
  return true;
}

PlannerInput
to_planner_input(const Settings &settings, int available_seconds)
{
  PlannerInput input;
  input.whole_body = settings.whole_body;
  input.regions = settings.selected_regions;
  input.constraints = settings.constraints();
  input.hidden_ids = settings.hidden_ids;
  input.available_seconds = available_seconds;
  return input;
}

const char *
to_string(BreakKind kind)
{
  switch (kind)
    {
    case BreakKind::Micro: return "micro";
    case BreakKind::Rest: return "rest";
    case BreakKind::Daily: return "daily";
    }
  return "unknown";
}

} // namespace workledge::core
