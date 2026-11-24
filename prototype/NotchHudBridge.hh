// Workledge — NotchHudBridge (integration skeleton)
//
// This is the QObject contract that NotchHud.qml expects as `bridge`.
// Copy it into workrave/ui/app/toolkits/qt/ and connect the TODOs to the
// real Workrave core (IApplicationContext / StatusWindowBridge / GUIConfig).
//
// It intentionally does NOT depend on Workrave headers so it can be reviewed
// standalone. Needs moc (Q_OBJECT).

#pragma once

#include <QObject>
#include <QString>

class NotchHudBridge : public QObject
{
  Q_OBJECT

  Q_PROPERTY(QString microTime READ microTime NOTIFY stateChanged)
  Q_PROPERTY(QString restTime READ restTime NOTIFY stateChanged)
  Q_PROPERTY(QString dailyTime READ dailyTime NOTIFY stateChanged)
  Q_PROPERTY(qreal microProgress READ microProgress NOTIFY stateChanged)
  Q_PROPERTY(qreal restProgress READ restProgress NOTIFY stateChanged)
  Q_PROPERTY(qreal dailyProgress READ dailyProgress NOTIFY stateChanged)
  Q_PROPERTY(QString nextBreakLabel READ nextBreakLabel NOTIFY stateChanged)
  Q_PROPERTY(QString nextBreakTime READ nextBreakTime NOTIFY stateChanged)

public:
  explicit NotchHudBridge(QObject *parent = nullptr);

  QString microTime() const { return m_microTime; }
  QString restTime() const { return m_restTime; }
  QString dailyTime() const { return m_dailyTime; }
  qreal microProgress() const { return m_microProgress; }
  qreal restProgress() const { return m_restProgress; }
  qreal dailyProgress() const { return m_dailyProgress; }
  QString nextBreakLabel() const { return m_nextBreakLabel; }
  QString nextBreakTime() const { return m_nextBreakTime; }

  // TODO: call these setters from Workrave core timer events, then emit stateChanged().
  void setMicroTime(const QString &v);
  void setRestTime(const QString &v);
  void setDailyTime(const QString &v);
  void setProgress(qreal micro, qreal rest, qreal daily);
  void setNextBreak(const QString &label, const QString &time);

signals:
  void stateChanged();
  void takeBreakRequested();
  void postponeRequested();
  void settingsRequested();

public slots:
  // TODO: forward to workrave::IApplicationContext / core commands.
  void takeBreak() { emit takeBreakRequested(); }
  void postpone() { emit postponeRequested(); }
  void openSettings() { emit settingsRequested(); }

private:
  QString m_microTime{"--:--"};
  QString m_restTime{"--:--"};
  QString m_dailyTime{"--:--:--"};
  qreal m_microProgress{0.0};
  qreal m_restProgress{0.0};
  qreal m_dailyProgress{0.0};
  QString m_nextBreakLabel{"Next break"};
  QString m_nextBreakTime{"--:--"};
};

inline NotchHudBridge::NotchHudBridge(QObject *parent)
  : QObject(parent)
{
}

inline void NotchHudBridge::setMicroTime(const QString &v)
{
  if (m_microTime != v) { m_microTime = v; emit stateChanged(); }
}

inline void NotchHudBridge::setRestTime(const QString &v)
{
  if (m_restTime != v) { m_restTime = v; emit stateChanged(); }
}

inline void NotchHudBridge::setDailyTime(const QString &v)
{
  if (m_dailyTime != v) { m_dailyTime = v; emit stateChanged(); }
}

inline void NotchHudBridge::setProgress(qreal micro, qreal rest, qreal daily)
{
  m_microProgress = micro;
  m_restProgress = rest;
  m_dailyProgress = daily;
  emit stateChanged();
}

inline void NotchHudBridge::setNextBreak(const QString &label, const QString &time)
{
  m_nextBreakLabel = label;
  m_nextBreakTime = time;
  emit stateChanged();
}
