#pragma once

#include "Core/ReduxLogger/Options.hpp"

#include <vector>

namespace rtk::logger::detail {

template <typename State>
bool hasObjectLevel(const ReduxLoggerOptions<State> &Options) {
  return Options.LevelByType.Num() > 0 || Options.LevelByTypeFn.Num() > 0;
}

inline FString defaultLevelValue(const FString *FoundLevel) {
  return FoundLevel ? *FoundLevel : FString();
}

inline FString invokeLevelCallback(const LevelCallback *FoundCallback,
                                   const AnyAction &FormattedAction,
                                   const TArray<FString> &Payload,
                                   const FString *FoundLevel) {
  return FoundCallback && *FoundCallback
             ? (*FoundCallback)(FormattedAction, Payload)
             : defaultLevelValue(FoundLevel);
}

template <typename State>
FString getLogLevel(const ReduxLoggerOptions<State> &Options,
                    const AnyAction &FormattedAction,
                    const TArray<FString> &Payload, const FString &Type) {
  return hasObjectLevel(Options)
             ? invokeLevelCallback(Options.LevelByTypeFn.Find(Type),
                                   FormattedAction, Payload,
                                   Options.LevelByType.Find(Type))
             : (Options.LevelFn ? Options.LevelFn(FormattedAction)
                                : Options.Level);
}

template <typename State>
FString transformState(const ReduxLoggerOptions<State> &Options,
                       const State &StateValue) {
  return Options.Transformer ? Options.Transformer(StateValue)
                             : Options.StateTransformer(StateValue);
}

template <typename State>
FString formatActionText(const ReduxLoggerOptions<State> &Options,
                         const AnyAction &Action) {
  return Options.ActionFormatter ? Options.ActionFormatter(Action)
                                 : formatAction(Action);
}

template <typename State>
FString defaultTitleFormatter(const ReduxLoggerOptions<State> &Options,
                              const AnyAction &Action, const FString &Time,
                              double Took) {
  return TEXT("action ") + Action.Type +
         (Options.bTimestamp ? FString::Printf(TEXT(" @ %s"), *Time)
                             : FString()) +
         (Options.bDuration
              ? FString::Printf(TEXT(" (in %.2f ms)"), Took)
              : FString());
}

inline void logRow(const std::function<void(const FString &)> &Logger,
                   const FString &Level, const FString &Label,
                   const FString &Value) {
  !Level.IsEmpty()
      ? (Logger(FString::Printf(TEXT("  %s %s"), *Label, *Value)), void())
      : void();
}

inline void defaultDiffLogger(
    const FString &PreviousState, const FString &NextState,
    const std::function<void(const FString &)> &Logger, bool bCollapsed) {
  Logger(bCollapsed ? TEXT("  diff (collapsed)") : TEXT("  diff"));
  Logger(PreviousState == NextState
             ? TEXT("    -- no diff --")
             : TEXT("    CHANGED: <state text changed; set DiffLogger for "
                    "field-level diff>"));
  Logger(TEXT("  -- diff end --"));
}

inline FString nextStateForBufferKey(const std::vector<LogEntry> &Buffer,
                                     size_t Index,
                                     const LogEntry &Entry) {
  return Index + 1 < Buffer.size() ? Buffer[Index + 1].PrevState
                                   : Entry.NextState;
}

inline double tookForBufferKey(const std::vector<LogEntry> &Buffer,
                               size_t Index, const LogEntry &Entry) {
  return Index + 1 < Buffer.size()
             ? Buffer[Index + 1].Started - Entry.Started
             : Entry.Took;
}

inline bool resolveCollapsed(
    const std::function<bool(const std::function<FString()> &,
                             const AnyAction &, const LogEntry &)> &Resolver,
    bool bCollapsed, const std::function<FString()> &GetNextState,
    const AnyAction &FormattedAction, const LogEntry &Entry) {
  return Resolver ? Resolver(GetNextState, FormattedAction, Entry)
                  : bCollapsed;
}

inline void logTraceWhen(const std::function<void(const FString &)> &Logger,
                         bool bWithTrace) {
  bWithTrace ? (Logger(TEXT("  TRACE")),
                Logger(TEXT("    <stack trace unavailable in UE_LOG sink>")),
                void())
             : void();
}

inline void logErrorWhen(const std::function<void(const FString &)> &Logger,
                         const FString &ErrorLevel, const LogEntry &Entry) {
  Entry.bHasError
      ? logRow(Logger, ErrorLevel, TEXT("error     "), Entry.Error)
      : void();
}

template <typename State>
void logDiffWhen(const ReduxLoggerOptions<State> &Options,
                 const std::function<void(const FString &)> &Logger,
                 const FString &PreviousState, const FString &NextState,
                 bool bCollapsed) {
  Options.bDiff
      ? (Options.DiffLogger
             ? Options.DiffLogger(PreviousState, NextState, Logger, bCollapsed)
             : defaultDiffLogger(PreviousState, NextState, Logger, bCollapsed))
      : void();
}

template <typename State>
void printBufferEntry(const std::vector<LogEntry> &Buffer, size_t Index,
                      const ReduxLoggerOptions<State> &Options,
                      const std::function<void(const FString &)> &Logger) {
  const LogEntry &Entry = Buffer[Index];
  const double Took = tookForBufferKey(Buffer, Index, Entry);
  const FString NextState = nextStateForBufferKey(Buffer, Index, Entry);
  const AnyAction FormattedAction = Options.ActionTransformer
                                        ? Options.ActionTransformer(Entry.Action)
                                        : Entry.Action;
  const FString FormattedActionText =
      formatActionText(Options, FormattedAction);
  const std::function<FString()> GetNextState = [&NextState]() {
    return NextState;
  };
  const bool bCollapsed =
      resolveCollapsed(Options.CollapsedFn, Options.bCollapsed, GetNextState,
                       FormattedAction, Entry);
  const FString FormattedTime = helpers::formatTime(Entry.StartedTime);
  const FString Title =
      Options.TitleFormatter
          ? Options.TitleFormatter(FormattedAction, FormattedTime, Took)
          : defaultTitleFormatter(Options, FormattedAction, FormattedTime,
                                  Took);
  Logger(Title);
  logRow(Logger,
         getLogLevel(Options, FormattedAction, payloadList(Entry.PrevState),
                     TEXT("prevState")),
         TEXT("prev state"), Entry.PrevState);
  logRow(Logger,
         getLogLevel(Options, FormattedAction,
                     payloadList(FormattedActionText), TEXT("action")),
         TEXT("action    "), FormattedActionText);
  logErrorWhen(Logger,
               getLogLevel(Options, FormattedAction,
                           payloadList(Entry.Error, Entry.PrevState),
                           TEXT("error")),
               Entry);
  logRow(Logger,
         getLogLevel(Options, FormattedAction, payloadList(NextState),
                     TEXT("nextState")),
         TEXT("next state"), NextState);
  logTraceWhen(Logger, Options.bWithTrace);
  logDiffWhen(Options, Logger, Entry.PrevState, NextState, bCollapsed);
}

template <typename State>
void printBufferRecursive(const std::vector<LogEntry> &Buffer, size_t Index,
                          const ReduxLoggerOptions<State> &Options,
                          const std::function<void(const FString &)> &Logger) {
  Index >= Buffer.size()
      ? void()
      : (printBufferEntry(Buffer, Index, Options, Logger),
         printBufferRecursive(Buffer, Index + 1, Options, Logger));
}

template <typename State>
void printBuffer(const std::vector<LogEntry> &Buffer,
                 const ReduxLoggerOptions<State> &Options,
                 const std::function<void(const FString &)> &Logger) {
  printBufferRecursive(Buffer, 0, Options, Logger);
}

} // namespace rtk::logger::detail
