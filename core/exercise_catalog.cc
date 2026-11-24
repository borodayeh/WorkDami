#include "exercise_catalog.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <system_error>

namespace workledge::core {
namespace {

std::vector<std::string>
split(const std::string &s, char sep)
{
  std::vector<std::string> out;
  std::size_t start = 0;
  for (;;)
    {
      const std::size_t pos = s.find(sep, start);
      if (pos == std::string::npos)
        {
          out.push_back(s.substr(start));
          break;
        }
      out.push_back(s.substr(start, pos - start));
      start = pos + 1;
    }
  return out;
}

std::string
trim(const std::string &s)
{
  const auto first = s.find_first_not_of(" \t\r\n");
  if (first == std::string::npos)
    {
      return {};
    }
  const auto last = s.find_last_not_of(" \t\r\n");
  return s.substr(first, last - first + 1);
}

std::vector<std::string>
split_list(const std::string &s)
{
  std::vector<std::string> out;
  for (const auto &raw : split(s, ','))
    {
      auto value = trim(raw);
      if (!value.empty())
        {
          out.push_back(value);
        }
    }
  return out;
}

bool
parse_bool(const std::string &s, bool &out)
{
  const auto v = trim(s);
  if (v == "1" || v == "true" || v == "yes")
    {
      out = true;
      return true;
    }
  if (v.empty() || v == "0" || v == "false" || v == "no")
    {
      out = false;
      return true;
    }
  return false;
}

int
parse_int(const std::string &s, int fallback)
{
  try
    {
      return std::stoi(trim(s));
    }
  catch (...)
    {
      return fallback;
    }
}

bool
parse_region(const std::string &s, BodyRegion &out)
{
  const auto v = trim(s);
  if (v == "neck") { out = BodyRegion::Neck; return true; }
  if (v == "shoulder") { out = BodyRegion::Shoulder; return true; }
  if (v == "upper_back") { out = BodyRegion::UpperBack; return true; }
  if (v == "lower_back") { out = BodyRegion::LowerBack; return true; }
  if (v == "hand_wrist_forearm") { out = BodyRegion::HandWristForearm; return true; }
  if (v == "chest") { out = BodyRegion::Chest; return true; }
  if (v == "pelvis") { out = BodyRegion::Pelvis; return true; }
  if (v == "legs") { out = BodyRegion::Legs; return true; }
  if (v == "whole_body") { out = BodyRegion::WholeBody; return true; }
  return false;
}

bool
parse_position(const std::string &s, Position &out)
{
  const auto v = trim(s);
  if (v == "seated") { out = Position::Seated; return true; }
  if (v == "standing") { out = Position::Standing; return true; }
  if (v == "floor") { out = Position::Floor; return true; }
  return false;
}

bool
parse_equipment(const std::string &s, Equipment &out)
{
  const auto v = trim(s);
  if (v == "none" || v.empty()) { out = Equipment::None; return true; }
  if (v == "chair") { out = Equipment::Chair; return true; }
  if (v == "wall") { out = Equipment::Wall; return true; }
  if (v == "other") { out = Equipment::Other; return true; }
  return false;
}

bool
parse_side(const std::string &s, Side &out)
{
  const auto v = trim(s);
  if (v == "left") { out = Side::Left; return true; }
  if (v == "right") { out = Side::Right; return true; }
  if (v == "unspecified") { out = Side::Unspecified; return true; }
  return false;
}

bool
parse_mode(const std::string &s, ExecutionMode &out)
{
  const auto v = trim(s);
  if (v == "timed") { out = ExecutionMode::Timed; return true; }
  if (v == "repetition") { out = ExecutionMode::Repetition; return true; }
  if (v == "bilateral" || v == "bilateral_timed") { out = ExecutionMode::BilateralTimed; return true; }
  return false;
}

ReviewStatus
parse_review(const std::string &s, bool &known)
{
  const auto v = trim(s);
  known = true;
  if (v == "approved") { return ReviewStatus::Approved; }
  if (v == "needs_review") { return ReviewStatus::NeedsReview; }
  if (v == "draft" || v.empty()) { return ReviewStatus::Draft; }
  known = false;
  return ReviewStatus::Draft; // fail closed: never approve by default
}

bool
parse_row(const std::vector<std::string> &c, std::size_t line_no,
          ExerciseRecord &out, std::vector<std::string> &warnings)
{
  if (c.size() < 27)
    {
      warnings.push_back("line " + std::to_string(line_no) + ": skipped, expected 27 columns, got "
                         + std::to_string(c.size()));
      return false;
    }

  out.id = trim(c[0]);
  if (out.id.empty())
    {
      warnings.push_back("line " + std::to_string(line_no) + ": skipped, empty id");
      return false;
    }

  const std::string en_title = trim(c[1]);
  const std::string en_instructions = trim(c[2]);
  const std::string fa_title = trim(c[3]);
  const std::string fa_instructions = trim(c[4]);
  if (!en_title.empty())
    {
      out.texts.push_back({"en", en_title, en_instructions});
    }
  if (!fa_title.empty())
    {
      out.texts.push_back({"fa", fa_title, fa_instructions});
    }
  if (out.texts.empty())
    {
      warnings.push_back(out.id + ": skipped, no localized title");
      return false;
    }

  for (const auto &r : split_list(c[5]))
    {
      BodyRegion parsed{};
      if (!parse_region(r, parsed))
        {
          warnings.push_back(out.id + ": unknown body region '" + r + "'");
          continue;
        }
      out.body_regions.push_back(parsed);
    }
  if (out.body_regions.empty())
    {
      warnings.push_back(out.id + ": skipped, no usable body region");
      return false;
    }

  out.primary_muscles = split_list(c[6]);
  out.secondary_muscles = split_list(c[7]);

  for (const auto &p : split_list(c[8]))
    {
      Position parsed{};
      if (!parse_position(p, parsed))
        {
          warnings.push_back(out.id + ": unknown position '" + p + "'");
          continue;
        }
      out.positions.push_back(parsed);
    }
  if (out.positions.empty())
    {
      warnings.push_back(out.id + ": skipped, no usable position");
      return false;
    }

  for (const auto &e : split_list(c[9]))
    {
      Equipment parsed{};
      if (!parse_equipment(e, parsed))
        {
          warnings.push_back(out.id + ": unknown equipment '" + e + "'");
          continue;
        }
      out.equipment.push_back(parsed);
    }
  if (out.equipment.empty())
    {
      out.equipment.push_back(Equipment::None);
    }

  if (!parse_bool(c[10], out.requires_floor))
    {
      warnings.push_back(out.id + ": bad requires_floor, assumed false");
      out.requires_floor = false;
    }
  if (!parse_bool(c[11], out.desk_friendly))
    {
      warnings.push_back(out.id + ": bad desk_friendly, assumed false");
      out.desk_friendly = false;
    }

  if (!parse_mode(c[12], out.duration.mode))
    {
      warnings.push_back(out.id + ": unknown execution_mode, skipped");
      return false;
    }
  out.duration.hold_seconds = parse_int(c[13], 0);
  out.duration.repetitions = parse_int(c[14], 0);
  out.duration.seconds_per_repetition = parse_int(c[15], 0);
  if (!parse_bool(c[16], out.duration.bilateral))
    {
      out.duration.bilateral = false;
    }
  out.duration.transition_seconds = parse_int(c[17], 0);
  if (!out.duration.valid())
    {
      warnings.push_back(out.id + ": skipped, invalid duration definition");
      return false;
    }

  const auto media_path = trim(c[18]);
  if (!media_path.empty())
    {
      out.media.declared = true;
      out.media.path = media_path;
      out.media.kind = media_path.size() > 4 && media_path.substr(media_path.size() - 4) == ".png"
                         ? "image/png"
                         : "unknown";
    }

  out.source = trim(c[19]);
  out.license = trim(c[20]);
  out.attribution = trim(c[21]);

  bool known_review = false;
  out.review_status = parse_review(c[22], known_review);
  if (!known_review)
    {
      warnings.push_back(out.id + ": unknown review_status, kept as draft");
    }

  for (const auto &s : split_list(c[23]))
    {
      Side side{};
      if (!parse_side(s, side))
        {
          warnings.push_back(out.id + ": unknown side '" + s + "'");
          continue;
        }
      out.side_order.push_back(side);
    }

  if (!parse_bool(c[24], out.mirror_media_allowed))
    {
      warnings.push_back(out.id + ": bad mirror_media_allowed, assumed false");
      out.mirror_media_allowed = false;
    }
  out.side2_media = trim(c[25]);
  out.side2_instructions = trim(c[26]);

  out.test_fixture = out.source.rfind("TEST_FIXTURE", 0) == 0;
  return true;
}

} // namespace

void
ExerciseCatalog::load_tsv(const std::string &path)
{
  records_.clear();
  warnings_.clear();

  std::ifstream in(path);
  if (!in.is_open())
    {
      warnings_.push_back("cannot open exercise file: " + path);
      return;
    }

  std::error_code ec;
  const auto base = std::filesystem::path(path).parent_path();

  std::string line;
  std::size_t line_no = 0;
  while (std::getline(in, line))
    {
      ++line_no;
      if (!line.empty() && line.back() == '\r')
        {
          line.pop_back();
        }
      const auto trimmed = trim(line);
      if (trimmed.empty() || trimmed.rfind("#", 0) == 0)
        {
          continue;
        }
      auto columns = split(line, '\t');
      if (!columns.empty() && trim(columns[0]) == "id")
        {
          continue; // header
        }

      ExerciseRecord record;
      if (!parse_row(columns, line_no, record, warnings_))
        {
          continue;
        }

      if (record.media.declared)
        {
          const auto resolved = base / record.media.path;
          if (!std::filesystem::exists(resolved, ec) && !ec)
            {
              record.media.file_missing = true;
              warnings_.push_back(record.id + ": declared media missing: " + record.media.path);
            }
        }

      records_.push_back(record);
    }
}

const ExerciseRecord *
ExerciseCatalog::find(const std::string &id) const
{
  for (const auto &record : records_)
    {
      if (record.id == id)
        {
          return &record;
        }
    }
  return nullptr;
}

} // namespace workledge::core
