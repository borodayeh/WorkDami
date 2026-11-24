#include "workrave_config_store.h"

#include <sstream>

namespace workledge::integration {
namespace {

constexpr const char *kPrefix = "gui/break_exercise/";

std::string
key(const char *name)
{
  return std::string(kPrefix) + name;
}

std::string
join_regions(const std::set<workledge::core::BodyRegion> &regions)
{
  std::string out;
  for (auto r : regions)
    {
      using workledge::core::BodyRegion;
      const char *name = "";
      switch (r)
        {
        case BodyRegion::Neck: name = "neck"; break;
        case BodyRegion::Shoulder: name = "shoulder"; break;
        case BodyRegion::UpperBack: name = "upper_back"; break;
        case BodyRegion::LowerBack: name = "lower_back"; break;
        case BodyRegion::HandWristForearm: name = "hand_wrist_forearm"; break;
        case BodyRegion::Chest: name = "chest"; break;
        case BodyRegion::Pelvis: name = "pelvis"; break;
        case BodyRegion::Legs: name = "legs"; break;
        case BodyRegion::WholeBody: name = "whole_body"; break;
        }
      if (!out.empty()) out += ",";
      out += name;
    }
  return out;
}

std::set<workledge::core::BodyRegion>
parse_regions(const std::string &s)
{
  using workledge::core::BodyRegion;
  std::set<BodyRegion> out;
  std::size_t start = 0;
  for (;;)
    {
      const auto pos = s.find(',', start);
      const auto token = s.substr(start, pos == std::string::npos ? std::string::npos : pos - start);
      if (token == "neck") out.insert(BodyRegion::Neck);
      else if (token == "shoulder") out.insert(BodyRegion::Shoulder);
      else if (token == "upper_back") out.insert(BodyRegion::UpperBack);
      else if (token == "lower_back") out.insert(BodyRegion::LowerBack);
      else if (token == "hand_wrist_forearm") out.insert(BodyRegion::HandWristForearm);
      else if (token == "chest") out.insert(BodyRegion::Chest);
      else if (token == "pelvis") out.insert(BodyRegion::Pelvis);
      else if (token == "legs") out.insert(BodyRegion::Legs);
      else if (token == "whole_body") out.insert(BodyRegion::WholeBody);
      if (pos == std::string::npos) break;
      start = pos + 1;
    }
  return out;
}

} // namespace

WorkraveConfigStore::WorkraveConfigStore(workrave::config::IConfigurator::Ptr config)
  : config_(std::move(config))
{
}

bool
WorkraveConfigStore::load(workledge::core::Settings &out, std::vector<std::string> &warnings)
{
  if (!config_)
    {
      warnings.push_back("no IConfigurator available");
      return false;
    }

  std::string regions;
  config_->get_value_with_default(key("regions"), regions, std::string{});
  out.selected_regions = parse_regions(regions);
  config_->get_value_with_default(key("whole_body"), out.whole_body, false);
  config_->get_value_with_default(key("all_regions_selected"), out.all_regions_selected, false);
  config_->get_value_with_default(key("seated_only"), out.seated_only, true);
  config_->get_value_with_default(key("allow_standing"), out.allow_standing, false);
  config_->get_value_with_default(key("allow_floor"), out.allow_floor, false);

  std::string hidden;
  config_->get_value_with_default(key("hidden"), hidden, std::string{});
  out.hidden_ids.clear();
  std::size_t start = 0;
  for (;;)
    {
      const auto pos = hidden.find('|', start);
      const auto token = hidden.substr(start, pos == std::string::npos ? std::string::npos : pos - start);
      if (!token.empty()) out.hidden_ids.insert(token);
      if (pos == std::string::npos) break;
      start = pos + 1;
    }

  config_->get_value_with_default(key("exercise_micro"), out.exercises_enabled[workledge::core::BreakKind::Micro], false);
  config_->get_value_with_default(key("exercise_rest"), out.exercises_enabled[workledge::core::BreakKind::Rest], true);
  config_->get_value_with_default(key("exercise_daily"), out.exercises_enabled[workledge::core::BreakKind::Daily], false);
  return true;
}

bool
WorkraveConfigStore::save(const workledge::core::Settings &in, std::string &error)
{
  if (!config_)
    {
      error = "no IConfigurator available";
      return false;
    }

  config_->set_value(key("regions"), join_regions(in.selected_regions));
  config_->set_value(key("whole_body"), in.whole_body);
  config_->set_value(key("all_regions_selected"), in.all_regions_selected);
  config_->set_value(key("seated_only"), in.seated_only);
  config_->set_value(key("allow_standing"), in.allow_standing);
  config_->set_value(key("allow_floor"), in.allow_floor);

  std::string hidden;
  for (const auto &id : in.hidden_ids)
    {
      if (!hidden.empty()) hidden += "|";
      hidden += id;
    }
  config_->set_value(key("hidden"), hidden);

  config_->set_value(key("exercise_micro"), in.exercises_enabled.at(workledge::core::BreakKind::Micro));
  config_->set_value(key("exercise_rest"), in.exercises_enabled.at(workledge::core::BreakKind::Rest));
  config_->set_value(key("exercise_daily"), in.exercises_enabled.at(workledge::core::BreakKind::Daily));

  // BLOCKER: this revision's IConfigurator::set_value and save() return void.
  // There is no failure channel to report, so "true" here only means the calls
  // were made. A real integration test must verify persistence before this is
  // trusted. Do not generalize FileSettingsStore's failure guarantees here.
  return true;
}

} // namespace workledge::integration
