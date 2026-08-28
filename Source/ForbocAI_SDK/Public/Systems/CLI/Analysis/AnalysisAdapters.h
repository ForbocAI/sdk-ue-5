#pragma once

#include "Components/CLI/NPC/CLINPCTypes.h"
#include "Components/NPC/Agent/AgentTypes.h"
#include "HAL/PlatformMisc.h"
#include "Systems/API/Endpoints/NPC/Process/Configuration/ProcessConfigurationAdapters.h"
#include "Core/JsonInterop.h"
#include "Systems/CLI/Presentation/PresentationAdapters.h"
#include "Systems/Data/DataAdapters.h"

namespace ForbocAI::CLI::Analysis {

/** User Story: As an analytical CLI consumer, I need API-owned thought fields serialized without reinterpretation. @fn inline FString serializeThoughtResult(const FDecisionIntent &Thought) */
inline FString serializeThoughtResult(const FDecisionIntent &Thought) {
  const auto &Fields =
      APISlice::NPCProcessConfiguration::processContractData().DecisionIntent;
  const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
  Object->SetStringField(Fields.Goal, Thought.Goal);
  Object->SetStringField(Fields.ActionType, Thought.ActionType);
  JsonInterop::detail::SetIfNonEmpty(Object, Fields.Target, Thought.Target);
  Thought.bHasMetadata
      ? (JsonInterop::SetFieldFromJsonString(
             Object, Fields.Metadata, Thought.MetadataJson, false),
         void())
      : void();
  return DataAdapters::SerializeObject(Object);
}

/** User Story: As an analytical CLI consumer, I need API-owned reasoning fields serialized without reinterpretation. @fn inline FString serializeReasoningResult(const FReasoningOutput &Reasoning) */
inline FString serializeReasoningResult(const FReasoningOutput &Reasoning) {
  const auto &Fields =
      APISlice::NPCProcessConfiguration::processContractData().ReasoningOutput;
  const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
  Object->SetStringField(Fields.ReasoningText, Reasoning.ReasoningText);
  Object->SetStringField(Fields.ResponseText, Reasoning.ResponseText);
  return DataAdapters::SerializeObject(Object);
}

/**
 * User Story: As Ghost diagnostics, I need the API-owned diagnostic result selected from thought metadata without reproducing diagnosis.
 * @fn inline func::Maybe<FString> selectDiagnosticResult( const FDecisionIntent &Thought, const ForbocAI::CLI::NPC::FCLINPCState &State)
 */
inline func::Maybe<FString>
selectDiagnosticResult(
    const FDecisionIntent &Thought,
    const ForbocAI::CLI::NPC::FCLINPCState &State) {
  TSharedPtr<FJsonObject> Metadata;
  const bool bParsed =
      Thought.bHasMetadata &&
      JsonInterop::ParseJsonObject(Thought.MetadataJson, Metadata);
  const TSharedPtr<FJsonObject> Diagnosis =
      bParsed && Metadata->HasTypedField<EJson::Object>(
                     State.Analysis.DiagnosisContainer)
          ? Metadata->GetObjectField(State.Analysis.DiagnosisContainer)
          : nullptr;
  return Diagnosis.IsValid() &&
                 Diagnosis->HasTypedField<EJson::String>(
                     State.Analysis.DiagnosticResultField)
             ? func::just(Diagnosis->GetStringField(
                   State.Analysis.DiagnosticResultField))
             : func::nothing<FString>();
}

/**
 * User Story: As either decision CLI, I need API-owned thought, reasoning, and diagnostic values selected as immutable output lines.
 * @fn inline TArray<FString> selectAnalyticalResultLines( const FAgentResponse &Response, const ForbocAI::CLI::NPC::FCLINPCState &State)
 */
inline TArray<FString> selectAnalyticalResultLines(
    const FAgentResponse &Response,
    const ForbocAI::CLI::NPC::FCLINPCState &State) {
  const func::Maybe<FString> Diagnostic =
      Response.bHasThoughtResult
          ? selectDiagnosticResult(Response.ThoughtResult, State)
          : func::nothing<FString>();
  const TArray<FString> DiagnosticLines = func::match(
      Diagnostic,
      [&State](const FString &Value) {
        return TArray<FString>{State.Messages.DiagnosticResultPrefix + Value};
      },
      []() { return TArray<FString>(); });
  return func::concat_arrays<FString>({
      Response.bHasThoughtResult
          ? TArray<FString>{State.Messages.ThoughtResultPrefix +
                            serializeThoughtResult(Response.ThoughtResult)}
          : TArray<FString>(),
      Response.bHasReasoningResult
          ? TArray<FString>{State.Messages.ReasoningResultPrefix +
                            serializeReasoningResult(
                                Response.ReasoningResult)}
          : TArray<FString>(),
      DiagnosticLines,
  });
}

/**
 * User Story: As explicit prompt diagnostics, I need API-rendered prompt lines selected only when the authored trace switch is set.
 * @fn inline TArray<FString> selectPromptTraceLines( const FAgentResponse &Response, const ForbocAI::CLI::NPC::FCLINPCState &State)
 */
inline TArray<FString> selectPromptTraceLines(
    const FAgentResponse &Response,
    const ForbocAI::CLI::NPC::FCLINPCState &State) {
  const FString Trace = FPlatformMisc::GetEnvironmentVariable(
      *State.Messages.PromptTraceEnvKey);
  return !Trace.IsEmpty() && Response.bHasPrompt
             ? TArray<FString>{State.Messages.PromptTraceHeader,
                               Response.Prompt,
                               State.Messages.PromptTraceFooter}
             : TArray<FString>();
}

/**
 * User Story: As an embedded or standalone CLI host, I need the same dialogue, action, analysis, and optional prompt lines returned from one thin response projection.
 * @fn inline TArray<FString> selectAgentResponseLines( const FAgentResponse &Response, const ForbocAI::CLI::NPC::FCLINPCState &State, const FString &DialogueLine, const FString &ActionLine, bool bTracePrompt)
 */
inline TArray<FString> selectAgentResponseLines(
    const FAgentResponse &Response,
    const ForbocAI::CLI::NPC::FCLINPCState &State,
    const FString &DialogueLine, const FString &ActionLine,
    bool bTracePrompt) {
  return func::concat_arrays<FString>({
      {DialogueLine},
      Response.bHasAction ? TArray<FString>{ActionLine} : TArray<FString>(),
      selectAnalyticalResultLines(Response, State),
      bTracePrompt ? selectPromptTraceLines(Response, State)
                   : TArray<FString>(),
  });
}

/** User Story: As CLI presentation, I need selected response lines emitted without changing their transport representation. @fn inline void logAgentResponseLines(const TArray<FString> &Lines) */
inline void logAgentResponseLines(const TArray<FString> &Lines) {
  func::for_each_array<FString>(
      Lines, [](const FString &Line) {
        ForbocAI::CLI::Presentation::logCliMessage(Line);
      });
}

/**
 * User Story: As an embedded CLI host, I need selected response lines serialized with the authored line separator.
 * @fn inline FString serializeAgentResponseLines( const TArray<FString> &Lines, const ForbocAI::CLI::NPC::FCLINPCState &State)
 */
inline FString serializeAgentResponseLines(
    const TArray<FString> &Lines,
    const ForbocAI::CLI::NPC::FCLINPCState &State) {
  return FString::Join(Lines, *State.Analysis.OutputLineSeparator);
}

/**
 * User Story: As either decision CLI, I need API-owned thought, reasoning, and diagnostic results rendered verbatim.
 * @fn inline void logAnalyticalResults( const FAgentResponse &Response, const ForbocAI::CLI::NPC::FCLINPCState &State)
 */
inline void logAnalyticalResults(
    const FAgentResponse &Response,
    const ForbocAI::CLI::NPC::FCLINPCState &State) {
  logAgentResponseLines(selectAnalyticalResultLines(Response, State));
}

/**
 * User Story: As explicit prompt diagnostics, I need the API-rendered prompt emitted only when the authored trace switch is set.
 * @fn inline void logPromptTrace( const FAgentResponse &Response, const ForbocAI::CLI::NPC::FCLINPCState &State)
 */
inline void logPromptTrace(
    const FAgentResponse &Response,
    const ForbocAI::CLI::NPC::FCLINPCState &State) {
  logAgentResponseLines(selectPromptTraceLines(Response, State));
}

} // namespace ForbocAI::CLI::Analysis
