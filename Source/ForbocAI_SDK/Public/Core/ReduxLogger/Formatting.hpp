#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Core/ReduxLogger/Options.hpp"

#include <vector>

namespace rtk::logger::detail {

/** User Story: As a core redux logger consumer, I need to invoke has object level through a stable signature so the core redux logger workflow remains explicit and composable. @fn template <typename State> bool hasObjectLevel(const ReduxLoggerOptions<State> &Options) */
template <typename State>
bool hasObjectLevel(const ReduxLoggerOptions<State> &Options) {
  return Options.LevelByType.Num() > FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA || Options.LevelByTypeFn.Num() > FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA;
}

/** User Story: As a core redux logger consumer, I need to invoke default level value through a stable signature so the core redux logger workflow remains explicit and composable. @fn inline FString defaultLevelValue(const FString *FoundLevel) */
inline FString defaultLevelValue(const FString *FoundLevel) {
  return FoundLevel ? *FoundLevel : FString();
}

/** User Story: As a core redux logger consumer, I need to invoke invoke level callback through a stable signature so the core redux logger workflow remains explicit and composable. @fn inline FString invokeLevelCallback(const LevelCallback *FoundCallback, const AnyAction &FormattedAction, const TArray<FString> &Payload, const FString *FoundLevel) */
inline FString invokeLevelCallback(const LevelCallback *FoundCallback,
                                   const AnyAction &FormattedAction,
                                   const TArray<FString> &Payload,
                                   const FString *FoundLevel) {
  return FoundCallback && *FoundCallback
             ? (*FoundCallback)(FormattedAction, Payload)
             : defaultLevelValue(FoundLevel);
}

/** User Story: As a core redux logger consumer, I need to invoke get log level through a stable signature so the core redux logger workflow remains explicit and composable. @fn template <typename State> FString getLogLevel(const ReduxLoggerOptions<State> &Options, const AnyAction &FormattedAction, const TArray<FString> &Payload, const FString &Type) */
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

/** User Story: As a core redux logger consumer, I need to invoke transform state through a stable signature so the core redux logger workflow remains explicit and composable. @fn template <typename State> FString transformState(const ReduxLoggerOptions<State> &Options, const State &StateValue) */
template <typename State>
FString transformState(const ReduxLoggerOptions<State> &Options,
                       const State &StateValue) {
  return Options.Transformer ? Options.Transformer(StateValue)
                             : Options.StateTransformer(StateValue);
}

/** User Story: As a core redux logger consumer, I need to invoke format action text through a stable signature so the core redux logger workflow remains explicit and composable. @fn template <typename State> FString formatActionText(const ReduxLoggerOptions<State> &Options, const AnyAction &Action) */
template <typename State>
FString formatActionText(const ReduxLoggerOptions<State> &Options,
                         const AnyAction &Action) {
  return Options.ActionFormatter ? Options.ActionFormatter(Action)
                                 : formatAction(Action);
}

/** User Story: As a core redux logger consumer, I need to invoke default title formatter through a stable signature so the core redux logger workflow remains explicit and composable. @fn template <typename State> FString defaultTitleFormatter(const ReduxLoggerOptions<State> &Options, const AnyAction &Action, const FString &Time, double Took) */
template <typename State>
FString defaultTitleFormatter(const ReduxLoggerOptions<State> &Options,
                              const AnyAction &Action, const FString &Time,
                              double Took) {
  return TEXT(FORBOCAI_SDK_AUTHORED_STRINGVD7BA0C34F0EC) + Action.Type +
         (Options.bTimestamp ? FString::Printf(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV1DA127672A33), *Time)
                             : FString()) +
         (Options.bDuration
              ? FString::Printf(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV8A8760C2B3F8), Took)
              : FString());
}

/** User Story: As a core redux logger consumer, I need to invoke log row through a stable signature so the core redux logger workflow remains explicit and composable. @fn inline void logRow(const std::function<void(const FString &)> &Logger, const FString &Level, const FString &Label, const FString &Value) */
inline void logRow(const std::function<void(const FString &)> &Logger,
                   const FString &Level, const FString &Label,
                   const FString &Value) {
  !Level.IsEmpty()
      ? (Logger(FString::Printf(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVE1B1F3E7AE9B), *Label, *Value)), void())
      : void();
}

/** User Story: As a core redux logger consumer, I need to invoke default diff logger through a stable signature so the core redux logger workflow remains explicit and composable. @fn inline void defaultDiffLogger( const FString &PreviousState, const FString &NextState, const std::function<void(const FString &)> &Logger, bool bCollapsed) */
inline void defaultDiffLogger(
    const FString &PreviousState, const FString &NextState,
    const std::function<void(const FString &)> &Logger, bool bCollapsed) {
  Logger(bCollapsed ? TEXT(FORBOCAI_SDK_AUTHORED_STRINGV5CE3F491DE41) : TEXT(FORBOCAI_SDK_AUTHORED_STRINGV414EF201B87B));
  Logger(PreviousState == NextState
             ? TEXT(FORBOCAI_SDK_AUTHORED_STRINGV44E6FCD85721)
             : TEXT(FORBOCAI_SDK_AUTHORED_STRINGV6DE9607253F8
                    FORBOCAI_SDK_AUTHORED_STRINGVA36C3C45544C));
  Logger(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV91F62130B097));
}

/** User Story: As a core redux logger consumer, I need to invoke next state for buffer key through a stable signature so the core redux logger workflow remains explicit and composable. @fn inline FString nextStateForBufferKey(const std::vector<LogEntry> &Buffer, size_t Index, const LogEntry &Entry) */
inline FString nextStateForBufferKey(const std::vector<LogEntry> &Buffer,
                                     size_t Index,
                                     const LogEntry &Entry) {
  return Index + FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4 < Buffer.size() ? Buffer[Index + FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4].PrevState
                                   : Entry.NextState;
}

/** User Story: As a core redux logger consumer, I need to invoke took for buffer key through a stable signature so the core redux logger workflow remains explicit and composable. @fn inline double tookForBufferKey(const std::vector<LogEntry> &Buffer, size_t Index, const LogEntry &Entry) */
inline double tookForBufferKey(const std::vector<LogEntry> &Buffer,
                               size_t Index, const LogEntry &Entry) {
  return Index + FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4 < Buffer.size()
             ? Buffer[Index + FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4].Started - Entry.Started
             : Entry.Took;
}

/** User Story: As a core redux logger consumer, I need to invoke resolve collapsed through a stable signature so the core redux logger workflow remains explicit and composable. @fn inline bool resolveCollapsed( const std::function<bool(const std::function<FString()> &, const AnyAction &, const LogEntry &)> &Resolver, bool bCollapsed, const std::function<FString()> &GetNextState, const AnyAction &FormattedAction, const LogEntry &Entry) */
inline bool resolveCollapsed(
    const std::function<bool(const std::function<FString()> &,
                             const AnyAction &, const LogEntry &)> &Resolver,
    bool bCollapsed, const std::function<FString()> &GetNextState,
    const AnyAction &FormattedAction, const LogEntry &Entry) {
  return Resolver ? Resolver(GetNextState, FormattedAction, Entry)
                  : bCollapsed;
}

/** User Story: As a core redux logger consumer, I need to invoke log trace when through a stable signature so the core redux logger workflow remains explicit and composable. @fn inline void logTraceWhen(const std::function<void(const FString &)> &Logger, bool bWithTrace) */
inline void logTraceWhen(const std::function<void(const FString &)> &Logger,
                         bool bWithTrace) {
  bWithTrace ? (Logger(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV5C007CAFBA32)),
                Logger(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV20B8F0723021)),
                void())
             : void();
}

/** User Story: As a core redux logger consumer, I need to invoke log error when through a stable signature so the core redux logger workflow remains explicit and composable. @fn inline void logErrorWhen(const std::function<void(const FString &)> &Logger, const FString &ErrorLevel, const LogEntry &Entry) */
inline void logErrorWhen(const std::function<void(const FString &)> &Logger,
                         const FString &ErrorLevel, const LogEntry &Entry) {
  Entry.bHasError
      ? logRow(Logger, ErrorLevel, TEXT(FORBOCAI_SDK_AUTHORED_STRINGV42D78FBFA467), Entry.Error)
      : void();
}

/** User Story: As a core redux logger consumer, I need to invoke log diff when through a stable signature so the core redux logger workflow remains explicit and composable. @fn template <typename State> void logDiffWhen(const ReduxLoggerOptions<State> &Options, const std::function<void(const FString &)> &Logger, const FString &PreviousState, const FString &NextState, bool bCollapsed) */
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

/** User Story: As a core redux logger consumer, I need to invoke print buffer entry through a stable signature so the core redux logger workflow remains explicit and composable. @fn template <typename State> void printBufferEntry(const std::vector<LogEntry> &Buffer, size_t Index, const ReduxLoggerOptions<State> &Options, const std::function<void(const FString &)> &Logger) */
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
                     TEXT(FORBOCAI_SDK_AUTHORED_STRINGVE9C123916D77)),
         TEXT(FORBOCAI_SDK_AUTHORED_STRINGVDF3862BC982B), Entry.PrevState);
  logRow(Logger,
         getLogLevel(Options, FormattedAction,
                     payloadList(FormattedActionText), TEXT(FORBOCAI_SDK_AUTHORED_STRINGVF673571534B7)),
         TEXT(FORBOCAI_SDK_AUTHORED_STRINGV4853A4E6FEA7), FormattedActionText);
  logErrorWhen(Logger,
               getLogLevel(Options, FormattedAction,
                           payloadList(Entry.Error, Entry.PrevState),
                           TEXT(FORBOCAI_SDK_AUTHORED_STRINGV2851A113080D)),
               Entry);
  logRow(Logger,
         getLogLevel(Options, FormattedAction, payloadList(NextState),
                     TEXT(FORBOCAI_SDK_AUTHORED_STRINGV465B0A4B2D3E)),
         TEXT(FORBOCAI_SDK_AUTHORED_STRINGVD9EED4E7F70D), NextState);
  logTraceWhen(Logger, Options.bWithTrace);
  logDiffWhen(Options, Logger, Entry.PrevState, NextState, bCollapsed);
}

/** User Story: As a core redux logger consumer, I need to invoke print buffer recursive through a stable signature so the core redux logger workflow remains explicit and composable. @fn template <typename State> void printBufferRecursive(const std::vector<LogEntry> &Buffer, size_t Index, const ReduxLoggerOptions<State> &Options, const std::function<void(const FString &)> &Logger) */
template <typename State>
void printBufferRecursive(const std::vector<LogEntry> &Buffer, size_t Index,
                          const ReduxLoggerOptions<State> &Options,
                          const std::function<void(const FString &)> &Logger) {
  Index >= Buffer.size()
      ? void()
      : (printBufferEntry(Buffer, Index, Options, Logger),
         printBufferRecursive(Buffer, Index + FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4, Options, Logger));
}

/** User Story: As a core redux logger consumer, I need to invoke print buffer through a stable signature so the core redux logger workflow remains explicit and composable. @fn template <typename State> void printBuffer(const std::vector<LogEntry> &Buffer, const ReduxLoggerOptions<State> &Options, const std::function<void(const FString &)> &Logger) */
template <typename State>
void printBuffer(const std::vector<LogEntry> &Buffer,
                 const ReduxLoggerOptions<State> &Options,
                 const std::function<void(const FString &)> &Logger) {
  printBufferRecursive(Buffer, FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA, Options, Logger);
}

} // namespace rtk::logger::detail
