#pragma once

#include "exercise_model.h"

#include <string>
#include <vector>

namespace workledge::core {

// Loads a tab-separated fixture file. Invalid rows are skipped with warnings;
// missing media is recorded but never aborts loading.
class ExerciseCatalog
{
public:
  void load_tsv(const std::string &path);

  const ExerciseRecord *find(const std::string &id) const;
  const std::vector<ExerciseRecord> &records() const { return records_; }
  const std::vector<std::string> &warnings() const { return warnings_; }

private:
  std::vector<ExerciseRecord> records_;
  std::vector<std::string> warnings_;
};

} // namespace workledge::core
