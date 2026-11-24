#pragma once

#include "exercise_catalog.h"
#include "exercise_session.h"
#include "routine_planner.h"

#include <optional>
#include <set>
#include <string>

namespace workledge::core {

// Ownership contract:
//  - Workrave core owns the break (start/end/cancel). This class only listens.
//  - ExerciseSession owns the current movement and stage.
//  - Pausing a movement does NOT pause the Workrave break. This class never
//    calls into the core; the break keeps running on Workrave's own clock.
//  - End or cancel of the break always clears the active exercise session.
//  - A repeated ShowBreak for the same break_id is idempotent: no second session.
//  - Closing the educational card is a no-op, not an end-of-session command.
//  - After suspend / a long clock gap, unseen movements are NOT recorded as
//    elapsed or confirmed. The default behaviour is to pause the session and
//    require an explicit Resume, unless the break has already ended.
//
// Time source: std::chrono::steady_clock in the product (monotonic). Tests
// inject integer seconds, so 1 tick == 1 second and no test sleeps. Wall-clock
// changes (system_clock) cannot affect session accounting.

enum class BreakSignal { ShowBreak, BreakTaken, Cancel };
enum class ExerciseCommand { Confirm, Pause, Resume, Skip, Finish, HideCurrent, CloseCard };

struct Presentation {
  std::string id;
  std::string title;
  std::string instructions;
  int step_index{0};
  int total_steps{0};
  int step_seconds{0};
  std::string side;
  bool paused{false};
};

struct Ledger {
  int shown{0};
  int elapsed{0};
  int confirmed{0};
};

class BreakExerciseCoordinator
{
public:
  explicit BreakExerciseCoordinator(const ExerciseCatalog &catalog);

  void set_plan(PlanResult plan);

  bool on_break_signal(BreakSignal signal, int break_id, int now_ticks);
  bool command(ExerciseCommand cmd, int now_ticks);
  void tick(int now_ticks);
  void on_system_resume(int now_ticks, int gap_ticks);

  const Presentation *presentation() const;
  const Ledger &ledger() const { return ledger_; }
  SessionState state() const { return session_.state(); }
  bool active() const { return active_; }
  int session_id() const { return session_id_; }
  bool long_gap_seen() const { return long_gap_seen_; }
  const std::set<std::string> &hidden_ids() const { return hidden_ids_; }

  // 1 tick == 1 second. 5 seconds covers a short event-loop stall; anything
  // longer is treated as suspend/resume, not as exercised time.
  static constexpr int kLongGapSeconds = 5;

private:
  void clear_session(int now_ticks);
  void advance(int now_ticks, bool confirmed);
  void refresh_presentation();

  const ExerciseCatalog &catalog_;
  PlanResult plan_;
  ExerciseSession session_{0};
  std::optional<Presentation> presentation_;
  Ledger ledger_;
  std::set<std::string> hidden_ids_;
  int active_break_id_{-1};
  int session_id_{0};
  int last_tick_{0};
  bool active_{false};
  bool paused_{false};
  bool long_gap_seen_{false};
};

const char *to_string(BreakSignal signal);
const char *to_string(ExerciseCommand command);

} // namespace workledge::core
