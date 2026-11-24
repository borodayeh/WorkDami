#pragma once

// Product storage backend (NOT compiled in the g++ core tests).
//
// Existing Workrave infrastructure chosen for the product:
//   libs/config/include/config/IConfigurator.hh  — typed get_value/set_value
//   libs/config/include/config/SettingCache.hh   — cached Setting<T>
//   libs/config/src/windows/W32Configurator.cc   — Windows registry backend
//   ui/app/GUIConfig.cc                          — key convention gui/...
//
// No parallel product settings file is introduced. The file backend in
// settings.h exists only for core tests and standalone development runs.
//
// Key mapping (all under gui/break_exercise/):
//   regions              -> gui/break_exercise/regions               string csv
//   whole_body           -> gui/break_exercise/whole_body            bool
//   all_regions_selected -> gui/break_exercise/all_regions_selected  bool
//   seated_only          -> gui/break_exercise/seated_only           bool
//   allow_standing       -> gui/break_exercise/allow_standing        bool
//   allow_floor          -> gui/break_exercise/allow_floor           bool
//   allowed_equipment    -> gui/break_exercise/allowed_equipment     string csv
//   hidden_ids           -> gui/break_exercise/hidden                string "|"-separated
//   exercises_enabled    -> gui/break_exercise/exercise_<kind>       bool
//
// Save must call IConfigurator::set_value for each key; a failed write must
// be reported to the caller, never reported as success. Windows build is not
// verified in this sandbox, so this backend remains UNVERIFIED until built.

#include "settings.h"

namespace workledge::core {

class WorkraveConfigStore; // declared for integration; definition lives in the Qt build.

} // namespace workledge::core
