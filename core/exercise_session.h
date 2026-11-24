#pragma once

#include <cstddef>

namespace workledge::core {

enum class SessionState { Idle, Preparing, Playing, Paused, Transitioning, Completed, Cancelled };
enum class SessionEvent { Start, Ready, Tick, Pause, Resume, Next, Skip, Finish, Cancel, ReopenCard };

// State machine only. No wall clock, no UI, no Workrave coupling.
// ReopenCard is a no-op so reopening the notched card cannot create a second session.
class ExerciseSession
{
public:
  explicit ExerciseSession(std::size_t item_count = 0);

  SessionState state() const { return state_; }
  std::size_t index() const { return index_; }
  std::size_t item_count() const { return item_count_; }

  bool apply(SessionEvent event);

private:
  bool can_advance() const;

  SessionState state_{SessionState::Idle};
  std::size_t index_{0};
  std::size_t item_count_{0};
};

const char *to_string(SessionState state);
const char *to_string(SessionEvent event);

} // namespace workledge::core
