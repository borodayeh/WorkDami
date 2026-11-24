#pragma once

// UNVERIFIED product backend. Compile only together with Workrave config libs.
// Keys are brand-independent and use gui/break_exercise/... so the provisional
// display name "مکثا / Maksa" cannot orphan stored settings.

#include "../core/settings.h"
#include "config/IConfigurator.hh"

namespace workledge::integration {

class WorkraveConfigStore final : public workledge::core::ISettingsStore
{
public:
  explicit WorkraveConfigStore(workrave::config::IConfigurator::Ptr config);

  bool load(workledge::core::Settings &out, std::vector<std::string> &warnings) override;
  bool save(const workledge::core::Settings &in, std::string &error) override;

private:
  workrave::config::IConfigurator::Ptr config_;
};

} // namespace workledge::integration
