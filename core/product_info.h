#pragma once

// Single source of truth for the provisional product display name.
// Do not rename Workrave / Dynamic Ledge namespaces, paths or package ids for branding.

#include <string_view>

namespace workledge::core {

inline constexpr std::string_view kDisplayNameFa = "مکثا";
inline constexpr std::string_view kDisplayNameLatin = "Maksa";
inline constexpr std::string_view kDisplayName = "مکثا / Maksa";
inline constexpr bool kNameIsProvisional = true;

} // namespace workledge::core
