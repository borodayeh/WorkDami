#include "coordinator.h"

namespace workledge::core {

BreakExerciseCoordinator::BreakExerciseCoordinator(const ExerciseCatalog &catalog)
  : catalog_(catalog)
{
}

void
BreakExerciseCoordinator::set_plan(PlanResult plan)
{
  plan_ = std::move(plan);
}

bool
BreakExerciseCoordinator::on_break_signal(BreakSignal signal, int break_id, int now_ticks)
{
  if (signal == BreakSignal::ShowBreak)
    {
      if (active_ && active_break_id_ == break_id)
        {
          return false; // duplicate start: keep the existing session
        }
      if (active_)
        {
          clear_session(now_ticks);
        }
      if (plan_.items.empty())
        {
          return false;
        }

      session_ = ExerciseSession(plan_.items.size());
      session_.apply(SessionEvent::Start);
      session_.apply(SessionEvent::Ready);
      active_ = true;
      paused_ = false;
      active_break_id_ = break_id;
      ++session_id_;
      last_tick_ = now_ticks;
      ledger_ = Ledger{};
      ledger_.shown = 1;
      refresh_presentation();
      return true;
    }

  if (signal == BreakSignal::BreakTaken || signal == BreakSignal::Cancel)
    {
      if (!active_)
        {
          return false;
        }
      clear_session(now_ticks);
      return true;
    }
  return false;
}

bool
BreakExerciseCoordinator::command(ExerciseCommand cmd, int now_ticks)
{
  if (cmd == ExerciseCommand::CloseCard)
    {
      return true; // deliberately a no-op
    }
  if (!active_)
    {
      return false;
    }

  switch (cmd)
    {
    case ExerciseCommand::Pause:
      if (session_.state() != SessionState::Playing)
        {
          return false;
        }
      session_.apply(SessionEvent::Pause);
      paused_ = true;
      last_tick_ = now_ticks;
      refresh_presentation();
      return true;

    case ExerciseCommand::Resume:
      if (session_.state() != SessionState::Paused)
        {
          return false;
        }
      session_.apply(SessionEvent::Resume);
      paused_ = false;
      last_tick_ = now_ticks;
      refresh_presentation();
      return true;

    case ExerciseCommand::Confirm:
      advance(now_ticks, true);
      return true;

    case ExerciseCommand::Skip:
      advance(now_ticks, false);
      return true;

    case ExerciseCommand::HideCurrent:
      if (presentation_.has_value())
        {
          hidden_ids_.insert(presentation_->id);
        }
      advance(now_ticks, false);
      return true;

    case ExerciseCommand::Finish:
      session_.apply(SessionEvent::Finish);
      presentation_.reset();
      last_tick_ = now_ticks;
      return true;

    case ExerciseCommand::CloseCard:
      break;
    }
  return false;
}

void
BreakExerciseCoordinator::tick(int now_ticks)
{
  if (!active_ || paused_ || session_.state() != SessionState::Playing)
    {
      last_tick_ = now_ticks;
      return;
    }
  if (now_ticks > last_tick_)
    {
      ledger_.elapsed += now_ticks - last_tick_;
    }
  last_tick_ = now_ticks;
}

void
BreakExerciseCoordinator::on_system_resume(int now_ticks, int gap_ticks)
{
  if (gap_ticks > kLongGapSeconds)
    {
      long_gap_seen_ = true;
      if (active_ && session_.state() == SessionState::Playing)
        {
          session_.apply(SessionEvent::Pause);
          paused_ = true;
          refresh_presentation();
        }
      // If the break already ended there is no session to pause.
    }
  // Rebase the monotonic step clock. No elapsed/confirmed credit for unseen time.
  last_tick_ = now_ticks;
}

const Presentation *
BreakExerciseCoordinator::presentation() const
{
  return presentation_ ? &*presentation_ : nullptr;
}

void
BreakExerciseCoordinator::clear_session(int now_ticks)
{
  if (active_)
    {
      session_.apply(SessionEvent::Cancel);
    }
  active_ = false;
  paused_ = false;
  presentation_.reset();
  active_break_id_ = -1;
  last_tick_ = now_ticks;
}

void
BreakExerciseCoordinator::advance(int now_ticks, bool confirmed)
{
  if (confirmed)
    {
      ++ledger_.confirmed;
    }
  if (session_.apply(SessionEvent::Next))
    {
      if (session_.state() == SessionState::Transitioning)
        {
          session_.apply(SessionEvent::Ready);
          ++ledger_.shown;
          refresh_presentation();
        }
      else
        {
          presentation_.reset(); // last movement finished
        }
    }
  else
    {
      presentation_.reset();
    }
  last_tick_ = now_ticks;
}

void
BreakExerciseCoordinator::refresh_presentation()
{
  presentation_.reset();
  if (!active_ || plan_.items.empty() || session_.index() >= plan_.items.size())
    {
      return;
    }

  const auto &item = plan_.items[session_.index()];
  const auto *record = catalog_.find(item.id);
  if (record == nullptr)
    {
      return;
    }
  const auto *text = record->text_for("fa");
  if (text == nullptr)
    {
      text = record->text_for("en");
    }

  Presentation view;
  view.id = item.id;
  view.title = text != nullptr ? text->title : item.id;
  view.instructions = text != nullptr ? text->instructions : std::string{};
  view.step_index = static_cast<int>(session_.index()) + 1;
  view.total_steps = static_cast<int>(plan_.items.size());
  view.step_seconds = item.seconds;
  view.side = record->duration.bilateral ? (session_.index() % 2 == 0 ? "right" : "left") : "";
  view.paused = paused_;
  presentation_ = view;
}

const char *
to_string(BreakSignal signal)
{
  switch (signal)
    {
    case BreakSignal::ShowBreak: return "ShowBreak";
    case BreakSignal::BreakTaken: return "BreakTaken";
    case BreakSignal::Cancel: return "Cancel";
    }
  return "Unknown";
}

const char *
to_string(ExerciseCommand command)
{
  switch (command)
    {
    case ExerciseCommand::Confirm: return "Confirm";
    case ExerciseCommand::Pause: return "Pause";
    case ExerciseCommand::Resume: return "Resume";
    case ExerciseCommand::Skip: return "Skip";
    case ExerciseCommand::Finish: return "Finish";
    case ExerciseCommand::HideCurrent: return "HideCurrent";
    case ExerciseCommand::CloseCard: return "CloseCard";
    }
  return "Unknown";
}

} // namespace workledge::core
