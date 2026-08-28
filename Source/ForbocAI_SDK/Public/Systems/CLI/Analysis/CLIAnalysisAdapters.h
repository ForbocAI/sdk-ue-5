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

/** User Story: As Ghost diagnostics, I need the API-owned diagnostic result selected from thought metadata without reproducing diagnosis. @fn inline func::Maybe<FString> selectDiagnosticResult(const FDecisionIntent &Thought, const ForbocAI::CLI::NPC::FCLINPCState &State) */
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

/** User Story: As either decision CLI, I need API-owned thought, reasoning, and diagnostic results rendered verbatim. @fn inline void logAnalyticalResults(const FAgentResponse &Response, const ForbocAI::CLI::NPC::FCLINPCState &State) */
inline void logAnalyticalResults(
    const FAgentResponse &Response,
    const ForbocAI::CLI::NPC::FCLINPCState &State) {
  Response.bHasThoughtResult
      ? (ForbocAI::CLI::Presentation::logCliMessage(
             State.Messages.ThoughtResultPrefix +
             serializeThoughtResult(Response.ThoughtResult)),
         void())
      : void();
  Response.bHasReasoningResult
      ? (ForbocAI::CLI::Presentation::logCliMessage(
             State.Messages.ReasoningResultPrefix +
             serializeReasoningResult(Response.ReasoningResult)),
         void())
      : void();
  const func::Maybe<FString> Diagnostic =
      Response.bHasThoughtResult
          ? selectDiagnosticResult(Response.ThoughtResult, State)
          : func::nothing<FString>();
  func::match(
      Diagnostic,
      [&State](const FString &Value) {
        ForbocAI::CLI::Presentation::logCliMessage(
            State.Messages.DiagnosticResultPrefix + Value);
      },
      []() {});
}

/** User Story: As explicit prompt diagnostics, I need the API-rendered prompt emitted only when the authored trace switch is set. @fn inline void logPromptTrace(const FAgentResponse &Response, const ForbocAI::CLI::NPC::FCLINPCState &State) */
inline void logPromptTrace(
    const FAgentResponse &Response,
    const ForbocAI::CLI::NPC::FCLINPCState &State) {
  const FString Trace = FPlatformMisc::GetEnvironmentVariable(
      *State.Messages.PromptTraceEnvKey);
  !Trace.IsEmpty() && Response.bHasPrompt
      ? (ForbocAI::CLI::Presentation::logCliMessage(
             State.Messages.PromptTraceHeader),
         ForbocAI::CLI::Presentation::logCliMessage(Response.Prompt),
         ForbocAI::CLI::Presentation::logCliMessage(
             State.Messages.PromptTraceFooter),
         void())
      : void();
}

} // namespace ForbocAI::CLI::Analysis
