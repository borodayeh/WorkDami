#include "exercise_session.h"

namespace workledge::core {

ExerciseSession::ExerciseSession(std::size_t item_count)
  : item_count_(item_count)
{
}

bool
ExerciseSession::can_advance() const
{
  return item_count_ > 0 && index_ + 1 < item_count_;
}

bool
ExerciseSession::apply(SessionEvent event)
{
  // Reopening the card must not create or restart a session.
  if (event == SessionEvent::ReopenCard)
    {
      return true;
    }

  switch (state_)
    {
    case SessionState::Idle:
      if (event == SessionEvent::Start && item_count_ > 0)
        {
          index_ = 0;
          state_ = SessionState::Preparing;
          return true;
        }
      return false;

    case SessionState::Preparing:
      if (event == SessionEvent::Ready)
        {
          state_ = SessionState::Playing;
          return true;
        }
      if (event == SessionEvent::Cancel)
        {
          state_ = SessionState::Cancelled;
          return true;
        }
      return false;

    case SessionState::Playing:
      if (event == SessionEvent::Pause)
        {
          state_ = SessionState::Paused;
          return true;
        }
      if (event == SessionEvent::Next || event == SessionEvent::Skip)
        {
          if (can_advance())
            {
              state_ = SessionState::Transitioning;
              return true;
            }
          state_ = SessionState::Completed;
          return true;
        }
      if (event == SessionEvent::Finish)
        {
          state_ = SessionState::Completed;
          return true;
        }
      if (event == SessionEvent::Cancel)
        {
          state_ = SessionState::Cancelled;
          return true;
        }
      return false;

    case SessionState::Paused:
      if (event == SessionEvent::Resume)
        {
          state_ = SessionState::Playing;
          return true;
        }
      if (event == SessionEvent::Cancel)
        {
          state_ = SessionState::Cancelled;
          return true;
        }
      return false;

    case SessionState::Transitioning:
      if (event == SessionEvent::Ready)
        {
          ++index_;
          state_ = SessionState::Playing;
          return true;
        }
      if (event == SessionEvent::Cancel)
        {
          state_ = SessionState::Cancelled;
          return true;
        }
      return false;

    case SessionState::Completed:
    case SessionState::Cancelled:
      if (event == SessionEvent::Start && item_count_ > 0)
        {
          index_ = 0;
          state_ = SessionState::Preparing;
          return true;
        }
      return false;
    }

  return false;
}

const char *
to_string(SessionState state)
{
  switch (state)
    {
    case SessionState::Idle: return "Idle";
    case SessionState::Preparing: return "Preparing";
    case SessionState::Playing: return "Playing";
    case SessionState::Paused: return "Paused";
    case SessionState::Transitioning: return "Transitioning";
    case SessionState::Completed: return "Completed";
    case SessionState::Cancelled: return "Cancelled";
    }
  return "Unknown";
}

const char *
to_string(SessionEvent event)
{
  switch (event)
    {
    case SessionEvent::Start: return "Start";
    case SessionEvent::Ready: return "Ready";
    case SessionEvent::Tick: return "Tick";
    case SessionEvent::Pause: return "Pause";
    case SessionEvent::Resume: return "Resume";
    case SessionEvent::Next: return "Next";
    case SessionEvent::Skip: return "Skip";
    case SessionEvent::Finish: return "Finish";
    case SessionEvent::Cancel: return "Cancel";
    case SessionEvent::ReopenCard: return "ReopenCard";
    }
  return "Unknown";
}

} // namespace workledge::core
