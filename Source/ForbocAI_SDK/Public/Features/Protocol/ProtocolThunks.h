#pragma once

#include "Core/rtk.hpp"
#include "Core/fp.hpp"

// Handler classification — pinned by the canonical instruction sequence in
// classified/docs/design/api/architecture.md § Canonical Instruction Sequence.
// Drift between this table and the dispatcher is treated as a contract bug.
//
//   Local        — handler executes a host-side capability (recall, lookup, etc.)
//   Pass-through — handler only acknowledges API-supplied tape state and recurses
//
// | Instruction        | Classification | Reason                                                          |
// | ------------------ | -------------- | --------------------------------------------------------------- |
// | IdentifyActor      | Local          | UE resolves actor info from the runtime registry                |
// | QueryVector        | Local          | UE runs the configured memory engine recall                     |
// | Decision           | Local          | UE applies the local decision policy (2026-04-28)               |
// | Reasoning          | Pass-through   | API hosts the SLM; UE only acks and continues (2026-04-27)      |
// | Finalize           | Local          | UE persists memory, applies state delta, dispatches verdict     |

#include "Core/JsonInterop.h"
#include "Features/Directive/DirectiveSlice.h"
#include "Features/Memory/MemoryThunks.h"
#include "Features/NPC/NPCSlice.h"
#include "Features/Protocol/Requests/RequestsAdapters.h"
#include <memory>

struct FRuntimeState;

namespace rtk {

struct FProtocolRuntime {
  std::function<ThunkAction<FMemoryItem, FRuntimeState>(const FMemoryItem &)>
      StoreMemory;
  std::function<ThunkAction<TArray<FMemoryItem>, FRuntimeState>(
      const FMemoryRecallRequest &)>
      RecallMemory;

  /**
   * Returns whether local memory store and recall handlers are configured.
   * User Story: As protocol orchestration, I need to know whether memory
   * support exists before executing instructions that depend on it.
   */
  bool HasMemory() const {
    return static_cast<bool>(StoreMemory) && static_cast<bool>(RecallMemory);
  }

};

/**
 * Builds the default local protocol runtime backed by node memory.
 * User Story: As local protocol execution, I need a ready-made runtime so the
 * protocol loop can call local memory services consistently.
 */
inline FProtocolRuntime LocalProtocolRuntime() {
  FProtocolRuntime Runtime;
  Runtime.StoreMemory = [](const FMemoryItem &Item) {
    return nodeMemoryStoreThunk(Item);
  };
  Runtime.RecallMemory = [](const FMemoryRecallRequest &Request) {
    return nodeMemoryRecallThunk(Request);
  };
  return Runtime;
}

/**
 * Builds a per-run in-memory protocol runtime.
 * User Story: As CLI protocol turns, I need TS parity with
 * createInMemoryMemoryEngine() so default processing does not depend on
 * separately initialized native memory.
 */
inline FProtocolRuntime InMemoryProtocolRuntime() {
  const std::shared_ptr<TArray<FMemoryItem>> Memories =
      std::make_shared<TArray<FMemoryItem>>();

  FProtocolRuntime Runtime;
  Runtime.StoreMemory = [Memories](const FMemoryItem &Item) {
    return [Memories, Item](std::function<AnyAction(const AnyAction &)> Dispatch,
                            std::function<const FRuntimeState &()> GetState)
               -> func::AsyncResult<FMemoryItem> {
      Dispatch(MemorySlice::Actions::memoryStoreStart());
      Memories->Add(Item);
      Dispatch(MemorySlice::Actions::memoryStoreSuccess(Item));
      return detail::ResolveAsync(Item);
    };
  };
  Runtime.RecallMemory = [Memories](const FMemoryRecallRequest &Request) {
    return [Memories, Request](
               std::function<AnyAction(const AnyAction &)> Dispatch,
               std::function<const FRuntimeState &()> GetState)
               -> func::AsyncResult<TArray<FMemoryItem>> {
      Dispatch(MemorySlice::Actions::memoryRecallStart());
      TArray<FMemoryItem> Results;
      const int32 Limit = Request.Limit <= 0 ? Memories->Num() : Request.Limit;
      const int32 Count = FMath::Min(Limit, Memories->Num());

      struct PopulateResults {
        static void apply(const TArray<FMemoryItem> &Source, int32 Index,
                          int32 Count, TArray<FMemoryItem> &Out) {
          Index >= Count ? void()
                         : (Out.Add(Source[Index]),
                            apply(Source, Index + 1, Count, Out), void());
        }
      };
      PopulateResults::apply(*Memories, 0, Count, Results);

      Dispatch(MemorySlice::Actions::memoryRecallSuccess(Results));
      return detail::ResolveAsync(Results);
    };
  };
  return Runtime;
}

namespace detail {

/**
 * Normalizes empty JSON payloads to an empty object literal.
 * User Story: As serializer helpers, I need empty payloads normalized so
 * outbound JSON contracts never receive blank strings unexpectedly.
 */
inline FString SafeJson(const FString &Json) {
  return Json.IsEmpty() ? TEXT("{}") : Json;
}

/**
 * Extracts the state JSON payload and normalizes empty values.
 * User Story: As protocol serializers, I need state payloads normalized so
 * downstream requests always receive valid JSON text.
 */
inline FString SafeStateJson(const FAgentState &State) {
  return SafeJson(State.JsonData);
}

/**
 * Reports whether the state carries a non-empty JSON payload.
 * User Story: As request builders, I need to detect meaningful state payloads
 * so I only serialize optional state when there is real data to send.
 */
inline bool HasStatePayload(const FAgentState &State) {
  return !State.JsonData.IsEmpty() && State.JsonData != TEXT("{}");
}

/**
 * Serializes a JSON object pointer into a string payload.
 * User Story: As protocol serializers, I need a shared object-to-string helper
 * so JSON payload generation stays consistent across instructions.
 */
inline FString JsonObjectToString(const TSharedPtr<FJsonObject> &Object) {
  return JsonInterop::StringifyObject(Object);
}

/**
 * Serializes an identify-actor result payload for protocol tooling.
 * User Story: As protocol execution, I need actor-identification results
 * wrapped in a stable JSON envelope so later instructions can consume them.
 */
inline FString SerializeIdentifyActorResult(const FNPCActorInfo &Actor) {
  const TSharedPtr<FJsonObject> ActorObject = MakeShared<FJsonObject>();
  ActorObject->SetStringField(TEXT("npcId"), Actor.NpcId);
  ActorObject->SetStringField(TEXT("persona"), Actor.Persona);
  ActorObject->SetObjectField(TEXT("structuredPersona"),
                              JsonInterop::StructuredPersonaToObject(
                                  Actor.Persona));
  ActorObject->SetObjectField(TEXT("data"),
                              JsonInterop::StateToObject(Actor.Data));

  const TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
  Root->SetStringField(TEXT("type"), TEXT("IdentifyActorResult"));
  Root->SetObjectField(TEXT("actor"), ActorObject);
  return JsonObjectToString(Root);
}

/**
 * Serializes a decision intent into a decision result payload.
 * User Story: As protocol execution, I need decision intent wrapped in a
 * stable JSON envelope so later instructions can consume it consistently.
 */
inline FString SerializeDecisionResult(const FString &Goal,
                                       const FString &ActionType,
                                       const FString &Target = TEXT("")) {
  const TSharedPtr<FJsonObject> IntentObject = MakeShared<FJsonObject>();
  IntentObject->SetStringField(TEXT("goal"), Goal);
  IntentObject->SetStringField(TEXT("actionType"), ActionType);
  !Target.IsEmpty()
      ? (IntentObject->SetStringField(TEXT("target"), Target), void())
      : void();

  const TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
  Root->SetStringField(TEXT("type"), TEXT("Decision"));
  Root->SetObjectField(TEXT("decisionIntent"), IntentObject);
  return JsonObjectToString(Root);
}

/**
 * Serializes reasoning output into a reasoning result payload.
 * User Story: As protocol execution, I need reasoning output wrapped in a
 * stable JSON envelope so later instructions can consume it consistently.
 */
inline FString SerializeReasoningResult(const FString &ReasoningText,
                                        const FString &ResponseText) {
  const TSharedPtr<FJsonObject> ReasoningObject = MakeShared<FJsonObject>();
  ReasoningObject->SetStringField(TEXT("reasoningText"), ReasoningText);
  ReasoningObject->SetStringField(TEXT("responseText"), ResponseText);

  const TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
  Root->SetStringField(TEXT("type"), TEXT("Reasoning"));
  Root->SetObjectField(TEXT("reasoningOutput"), ReasoningObject);
  return JsonObjectToString(Root);
}

/**
 * Recursively serializes memory items into a JSON value array.
 * User Story: As protocol memory serialization, I need recursive array
 * building so query-vector result payloads stay expression-style.
 */
inline void SerializeMemoryItemsRecursive(
    const TArray<FMemoryItem> &Memories,
    TArray<TSharedPtr<FJsonValue>> &Out, int32 Index) {
  Index < Memories.Num()
      ? [&]() {
          const FMemoryItem &Memory = Memories[Index];
          const TSharedPtr<FJsonObject> MemoryObject =
              MakeShared<FJsonObject>();
          MemoryObject->SetStringField(TEXT("text"), Memory.Text);
          MemoryObject->SetStringField(TEXT("type"), Memory.Type);
          MemoryObject->SetNumberField(TEXT("importance"), Memory.Importance);
          MemoryObject->SetNumberField(TEXT("similarity"), Memory.Similarity);
          Out.Add(MakeShared<FJsonValueObject>(MemoryObject));
          SerializeMemoryItemsRecursive(Memories, Out, Index + 1);
        }()
      : void();
}

/**
 * Serializes recalled memories into a query-vector result payload.
 * User Story: As protocol execution, I need recalled memories wrapped in a
 * stable JSON envelope so follow-up instructions can read them consistently.
 */
inline FString SerializeQueryVectorResult(
    const TArray<FMemoryItem> &Memories) {
  TArray<TSharedPtr<FJsonValue>> MemoryValues;
  SerializeMemoryItemsRecursive(Memories, MemoryValues, 0);

  const TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
  Root->SetStringField(TEXT("type"), TEXT("QueryVectorResult"));
  Root->SetArrayField(TEXT("memories"), MemoryValues);
  return JsonObjectToString(Root);
}

/**
 * Converts a protocol instruction into the public agent response shape.
 * User Story: As protocol callers, I need typed instructions converted into
 * agent responses so runtime code can consume dialogue and actions directly.
 */
inline FAgentResponse BuildAgentResponse(const FNPCInstruction &Instruction) {
  FAgentResponse Response;
  return (Response.Dialogue = Instruction.Dialogue,
          Response.Thought = Instruction.Dialogue,
          Instruction.bHasAction
              ? (Response.Action = Instruction.Action, void())
              : void(),
          Response);
}

/**
 * Converts a single memory item into its recalled counterpart.
 * User Story: As protocol memory flows, I need item conversion so recalled
 * memory arrays can be built without imperative loop code.
 */
inline FRecalledMemory MemoryItemToRecalled(const FMemoryItem &Item) {
  FRecalledMemory M;
  M.Text = Item.Text;
  M.Type = Item.Type;
  M.Importance = Item.Importance;
  M.Similarity = Item.Similarity;
  return M;
}

/**
 * Recursively populates a recalled-memory array from memory items.
 * User Story: As protocol memory flows, I need recursive array building so
 * memory conversion stays expression-style without imperative loops.
 */
inline TArray<FRecalledMemory>
PopulateRecalledMemoriesRecursive(const TArray<FMemoryItem> &Memories,
                                  int32 Index,
                                  TArray<FRecalledMemory> Result) {
  return Index >= Memories.Num()
             ? Result
             : (Result.Add(MemoryItemToRecalled(Memories[Index])),
                PopulateRecalledMemoriesRecursive(Memories, Index + 1,
                                                  MoveTemp(Result)));
}

inline func::AsyncResult<rtk::FEmptyPayload>
PersistMemoryInstructions(const TArray<FMemoryStoreInstruction> &Instructions,
                          int32 Index, const FProtocolRuntime &Runtime,
                          std::function<AnyAction(const AnyAction &)> Dispatch,
                          std::function<const FRuntimeState &()> GetState);

func::AsyncResult<FAgentResponse>
RunProtocolTurn(const FString &NpcId, const FString &Input,
                const FString &RunId, const FNPCProcessTape &Tape,
                const FString &LastResult, bool bHasLastResult,
                int32 Turn, const FProtocolRuntime &Runtime,
                std::function<AnyAction(const AnyAction &)> Dispatch,
                std::function<const FRuntimeState &()> GetState);

/**
 * Handles the IdentifyActor protocol instruction by serializing actor info
 * and recursing into the next protocol turn.
 * User Story: As protocol instruction dispatch, I need actor identification
 * handled as a pure expression so the instruction ternary stays flat.
 */
inline func::AsyncResult<FAgentResponse>
HandleIdentifyActor(const FNPCProcessResponse &Response,
                    const FString &NpcId, const FString &Input,
                    const FString &RunId, int32 Turn,
                    const FProtocolRuntime &Runtime,
                    std::function<AnyAction(const AnyAction &)> Dispatch,
                    std::function<const FRuntimeState &()> GetState) {
  FNPCActorInfo Actor;
  Actor.NpcId = NpcId;
  Actor.Persona = Response.Tape.Persona;
  Actor.Data = Response.Tape.NpcState;
  return RunProtocolTurn(NpcId, Input, RunId, Response.Tape,
                         SerializeIdentifyActorResult(Actor), true, Turn + 1,
                         Runtime, Dispatch, GetState);
}

/**
 * Handles the QueryVector protocol instruction by dispatching a memory recall
 * and building the recalled-memory tape for the next turn.
 * User Story: As protocol instruction dispatch, I need vector queries handled
 * as a pure expression so the instruction ternary stays flat.
 */
inline func::AsyncResult<FAgentResponse>
HandleQueryVector(const FNPCProcessResponse &Response,
                  const FNPCInstruction &Instruction,
                  const FString &NpcId, const FString &Input,
                  const FString &RunId, int32 Turn,
                  const FProtocolRuntime &Runtime,
                  std::function<AnyAction(const AnyAction &)> Dispatch,
                  std::function<const FRuntimeState &()> GetState) {
  return !Runtime.HasMemory()
             ? (Dispatch(DirectiveSlice::Actions::directiveRunFailed(
                    RunId,
                    TEXT("API requested memory recall, but no memory engine "
                         "is configured"))),
                RejectAsync<FAgentResponse>(
                    TEXT("API requested memory recall, but no memory engine "
                         "is configured")))
             : [&]() -> func::AsyncResult<FAgentResponse> {
    FDirectiveResponse Directive;
    Directive.recallMemory = TypeFactory::MemoryRecallInstruction(
        Instruction.Query, Instruction.Limit, Instruction.Threshold);
    Dispatch(
        DirectiveSlice::Actions::directiveReceived(RunId, Directive));

    FMemoryRecallRequest RecallRequest;
    RecallRequest.Query = Instruction.Query;
    RecallRequest.Limit = Instruction.Limit;
    RecallRequest.Threshold = Instruction.Threshold;

    return func::AsyncChain::then<TArray<FMemoryItem>, FAgentResponse>(
        Runtime.RecallMemory(RecallRequest)(Dispatch, GetState),
        [NpcId, Input, RunId, Response, Turn, Dispatch, GetState,
         Runtime](const TArray<FMemoryItem> &Memories) {
          FNPCProcessTape NextTape = Response.Tape;
          NextTape.Memories =
              PopulateRecalledMemoriesRecursive(Memories, 0, TArray<FRecalledMemory>());
          NextTape.bVectorQueried = true;
          return RunProtocolTurn(
              NpcId, Input, RunId, NextTape,
              SerializeQueryVectorResult(Memories), true, Turn + 1,
              Runtime, Dispatch, GetState);
        });
  }();
}

/**
 * Handles the Decision protocol instruction by returning a decision intent.
 * User Story: As protocol instruction dispatch, I need the Decision step
 * handled so the multi-round loop can advance past the decision phase
 * without stalling. The API issues DecisionInstruction after QueryVector;
 * the SDK must return a DecisionResult with goal and actionType so the
 * tape includes decisionIntent on subsequent /process calls.
 *
 * Implementation (2026-04-28): Derives goal and actionType from observation
 * and memories, mirroring the API Orchestrator.hs logic.
 */
inline func::AsyncResult<FAgentResponse>
HandleDecision(const FNPCProcessResponse &Response,
               const FString &NpcId, const FString &Input,
               const FString &RunId, int32 Turn,
               const FProtocolRuntime &Runtime,
               std::function<AnyAction(const AnyAction &)> Dispatch,
               std::function<const FRuntimeState &()> GetState) {
  FNPCProcessTape NextTape = Response.Tape;
  
  const FString ObsLower = Response.Tape.Observation.ToLower();
  const TArray<FString> ActionVerbs = {TEXT("attack"), TEXT("move"), TEXT("take"), TEXT("give"), TEXT("use"), TEXT("open"), TEXT("close"), TEXT("pick")};
  
  /**
   * Recursive predicate to detect action verbs without imperative loops.
   * User Story: As a maintainer, I need this note so the surrounding code intent
   * stays clear during maintenance and debugging.
   */
  const std::function<bool(int32)> ContainsActionVerb = [&](int32 Index) -> bool {
    return Index >= ActionVerbs.Num()
               ? false
               : (ObsLower.Contains(ActionVerbs[Index])
                      ? true
                      : ContainsActionVerb(Index + 1));
  };
  
  NextTape.DecisionIntent.ActionType = ContainsActionVerb(0) ? TEXT("INTERACT") : TEXT("SPEAK");

  const TArray<FString> TargetTokens = {TEXT("to"), TEXT("at"), TEXT("on"), TEXT("with")};

  /**
   * Recursive target extractor — walks the preposition list without an
   * imperative loop. Returns the trimmed substring after the first matching
   * " <token> " separator, or empty when no token matches.
   */
  const std::function<FString(const FString &, int32)> ExtractTargetRecursive =
      [&](const FString &Obs, int32 Index) -> FString {
    return Index >= TargetTokens.Num()
               ? FString(TEXT(""))
               : [&]() -> FString {
                   const FString &Token = TargetTokens[Index];
                   const int32 Pos = Obs.Find(TEXT(" ") + Token + TEXT(" "));
                   return Pos != INDEX_NONE
                              ? Obs.RightChop(Pos + Token.Len() + 2)
                                    .TrimStartAndEnd()
                              : ExtractTargetRecursive(Obs, Index + 1);
                 }();
  };

  NextTape.DecisionIntent.Target =
      NextTape.DecisionIntent.ActionType == TEXT("INTERACT")
          ? ExtractTargetRecursive(ObsLower, 0)
          : FString(TEXT(""));

  NextTape.DecisionIntent.Goal =
      Response.Tape.Memories.Num() > 0
          ? FString::Printf(
                TEXT("Respond to: %s (with %d recalled memories)"),
                *Response.Tape.Observation, Response.Tape.Memories.Num())
          : FString::Printf(TEXT("Respond to: %s"),
                            *Response.Tape.Observation);

  NextTape.bDecisionCompleted = true;

  return RunProtocolTurn(
      NpcId, Input, RunId, NextTape,
      SerializeDecisionResult(NextTape.DecisionIntent.Goal, NextTape.DecisionIntent.ActionType, NextTape.DecisionIntent.Target),
      true, Turn + 1, Runtime, Dispatch, GetState);
}

/**
 * Handles the Reasoning protocol instruction.
 * User Story: As protocol instruction dispatch, I need the Reasoning step
 * acknowledged so the multi-round loop can advance past it without stalling.
 *
 * Architectural note (2026-04-28): SLM inference is now API-hosted. The API
 * runs the model itself and populates Tape.ReasoningText / Tape.ResponseText
 * before returning. The SDK's only remaining responsibility for a Reasoning
 * step is to mark the tape completed and continue. SDK-local inference is
 * deliberately not invoked here.
 */
inline func::AsyncResult<FAgentResponse>
HandleReasoning(const FNPCProcessResponse &Response,
                const FString &NpcId, const FString &Input,
                const FString &RunId, int32 Turn,
                const FProtocolRuntime &Runtime,
                std::function<AnyAction(const AnyAction &)> Dispatch,
                std::function<const FRuntimeState &()> GetState) {
  FNPCProcessTape NextTape = Response.Tape;
  NextTape.bReasoningCompleted = true;

  return RunProtocolTurn(
      NpcId, Input, RunId, NextTape,
      SerializeReasoningResult(NextTape.ReasoningOutput.ReasoningText, NextTape.ReasoningOutput.ResponseText),
      true, Turn + 1, Runtime, Dispatch, GetState);
}

/**
 * Handles the Finalize protocol instruction by validating the verdict,
 * persisting memory, and applying state transforms.
 * User Story: As protocol instruction dispatch, I need finalization handled
 * as a pure expression so the instruction ternary stays flat.
 */
inline func::AsyncResult<FAgentResponse>
HandleFinalize(const FNPCInstruction &Instruction,
               const FString &NpcId, const FString &Input,
               const FString &RunId,
               const FProtocolRuntime &Runtime,
               std::function<AnyAction(const AnyAction &)> Dispatch,
               std::function<const FRuntimeState &()> GetState) {
  FVerdictResponse Verdict;
  Verdict.bValid = Instruction.bValid;
  Verdict.Signature = Instruction.Signature;
  Verdict.storeMemory = Instruction.storeMemory;
  Verdict.StateDelta = Instruction.StateTransform;
  Verdict.Dialogue = Instruction.Dialogue;
  Verdict.bHasAction = Instruction.bHasAction;
  Verdict.Action = Instruction.Action;
  Dispatch(DirectiveSlice::Actions::verdictValidated(RunId, Verdict));

  return !Instruction.bValid
             ? (Dispatch(NPCSlice::Actions::blockAction(
                    NpcId, Instruction.Dialogue.IsEmpty()
                               ? FString(TEXT("Validation failed"))
                               : Instruction.Dialogue)),
                ResolveAsync(BuildAgentResponse(Instruction)))
             : func::AsyncChain::then<rtk::FEmptyPayload, FAgentResponse>(
                   PersistMemoryInstructions(Instruction.storeMemory, 0,
                                             Runtime, Dispatch, GetState),
                   [NpcId, Input, Instruction, Dispatch,
                    GetState](const rtk::FEmptyPayload &) {
                     HasStatePayload(Instruction.StateTransform)
                         ? (Dispatch(NPCSlice::Actions::updateNPCState(
                                NpcId, Instruction.StateTransform)),
                            void())
                         : void();

                     Dispatch(NPCSlice::Actions::setLastAction(
                         NpcId, Instruction.Action, Instruction.bHasAction));

                     Dispatch(NPCSlice::Actions::addToHistory(
                         NpcId, TEXT("user"), Input));
                     Dispatch(NPCSlice::Actions::addToHistory(
                         NpcId, TEXT("assistant"), Instruction.Dialogue));

                     return ResolveAsync(BuildAgentResponse(Instruction));
                   });
}

inline func::AsyncResult<FAgentResponse>
RunProtocolTurn(const FString &NpcId, const FString &Input,
                const FString &RunId, const FNPCProcessTape &Tape,
                const FString &LastResult, bool bHasLastResult,
                int32 Turn, const FProtocolRuntime &Runtime,
                std::function<AnyAction(const AnyAction &)> Dispatch,
                std::function<const FRuntimeState &()> GetState) {
  return Turn >= 12
             ? (Dispatch(DirectiveSlice::Actions::directiveRunFailed(
                    RunId, TEXT("Max turns exceeded"))),
                RejectAsync<FAgentResponse>(
                    TEXT("Protocol loop exceeded max turns")))
             : [&]() -> func::AsyncResult<FAgentResponse> {
    FNPCProcessRequest Request;
    Request.Tape = Tape;
    Request.PreviousResult = LastResult;
    Request.bHasPreviousResult = bHasLastResult;

    return func::AsyncChain::then<FNPCProcessResponse, FAgentResponse>(
               APISlice::Endpoints::postNpcProcess(NpcId, Request)(Dispatch,
                                                                   GetState),
               [NpcId, Input, RunId, Tape, Turn, Runtime, Dispatch,
                GetState](const FNPCProcessResponse &Response)
                   -> func::AsyncResult<FAgentResponse> {
                 const FNPCInstruction &Instruction = Response.Instruction;

                 return Instruction.Type ==
                                ENPCInstructionType::IdentifyActor
                            ? HandleIdentifyActor(Response, NpcId, Input,
                                                  RunId, Turn, Runtime,
                                                  Dispatch, GetState)
                        : Instruction.Type ==
                                  ENPCInstructionType::QueryVector
                            ? HandleQueryVector(Response, Instruction, NpcId,
                                                Input, RunId, Turn, Runtime,
                                                Dispatch, GetState)
                        : Instruction.Type ==
                                  ENPCInstructionType::Decision
                            ? HandleDecision(Response, NpcId, Input, RunId,
                                             Turn, Runtime, Dispatch, GetState)
                        : Instruction.Type ==
                                  ENPCInstructionType::Reasoning
                            ? HandleReasoning(Response, NpcId, Input, RunId,
                                              Turn, Runtime, Dispatch, GetState)
                        : Instruction.Type == ENPCInstructionType::Finalize
                            ? HandleFinalize(Instruction, NpcId, Input, RunId,
                                             Runtime, Dispatch, GetState)
                            : (Dispatch(
                                   DirectiveSlice::Actions::directiveRunFailed(
                                       RunId,
                                       FString::Printf(
                                           TEXT("Unsupported protocol "
                                                "instruction type: %d"),
                                           static_cast<int32>(
                                               Instruction.Type)))),
                               RejectAsync<FAgentResponse>(FString::Printf(
                                   TEXT("Unsupported protocol instruction "
                                        "type: %d"),
                                   static_cast<int32>(Instruction.Type))));
               })
        .catch_([RunId, Dispatch](std::string Error) {
          Dispatch(DirectiveSlice::Actions::directiveRunFailed(
              RunId, FString(UTF8_TO_TCHAR(Error.c_str()))));
        });
  }();
}

inline func::AsyncResult<rtk::FEmptyPayload>
PersistMemoryInstructions(const TArray<FMemoryStoreInstruction> &Instructions,
                          int32 Index, const FProtocolRuntime &Runtime,
                          std::function<AnyAction(const AnyAction &)> Dispatch,
                          std::function<const FRuntimeState &()> GetState) {
  return Index >= Instructions.Num()
             ? ResolveAsync(rtk::FEmptyPayload{})
         : !Runtime.StoreMemory
             ? RejectAsync<rtk::FEmptyPayload>(
                   TEXT("API returned memoryStore instructions, but no memory "
                        "engine is configured"))
             : func::AsyncChain::then<FMemoryItem, rtk::FEmptyPayload>(
                   Runtime.StoreMemory(MakeMemoryItem(Instructions[Index]))(
                       Dispatch, GetState),
                   [Instructions, Index, Runtime, Dispatch,
                    GetState](const FMemoryItem &Stored) {
                     return PersistMemoryInstructions(Instructions, Index + 1,
                                                      Runtime, Dispatch,
                                                      GetState);
                   });
}

} // namespace detail

/**
 * Protocol thunks
 * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
 */

template <typename RuntimeState = FRuntimeState>
inline ThunkAction<FAgentResponse, RuntimeState>
processNPC(const FString &NpcId, const FString &Input = TEXT(""),
           const FString &ContextJson = TEXT("{}"),
           const FString &Persona = TEXT(""),
           const FAgentState &InitialState = FAgentState(),
           const FProtocolRuntime &Runtime = FProtocolRuntime()) {
  return [NpcId, Input, ContextJson, Persona, InitialState, Runtime](
             std::function<AnyAction(const AnyAction &)> Dispatch,
             std::function<const RuntimeState &()> GetState)
             -> func::AsyncResult<FAgentResponse> {
    const auto ExistingNpc = NPCSlice::selectNPCById(GetState().NPCs, NpcId);
    const bool bHasExplicitState =
        !InitialState.JsonData.IsEmpty() && InitialState.JsonData != TEXT("{}");

    const FString ResolvedPersona =
        ExistingNpc.hasValue && Persona.IsEmpty()
            ? ExistingNpc.value.Persona
            : Persona;

    const FAgentState CurrentState =
        ExistingNpc.hasValue && !bHasExplicitState
            ? ExistingNpc.value.State
            : InitialState;

    return [&]() -> func::AsyncResult<FAgentResponse> {
      !ExistingNpc.hasValue
          ? [&]() {
              FNPCInternalState Info;
              Info.Id = NpcId;
              Info.Persona = ResolvedPersona;
              Info.State = InitialState;
              Dispatch(NPCSlice::Actions::setNPCInfo(Info));
            }()
          : (NPCSlice::selectActiveNpcId(GetState().NPCs) != NpcId
                 ? (Dispatch(NPCSlice::Actions::setActiveNPC(NpcId)), void())
                 : void());

      const FString RunId = FString::Printf(
          TEXT("%s:%lld"), *NpcId, FDateTime::UtcNow().ToUnixTimestamp());
      Dispatch(
          DirectiveSlice::Actions::directiveRunStarted(RunId, NpcId, Input));

      FNPCProcessTape Tape = ProtocolRequests::ProcessTape(
          Input, ContextJson, CurrentState, ResolvedPersona);
      Tape.Memories.Empty();
      Tape.bVectorQueried = false;

      return detail::RunProtocolTurn(NpcId, Input, RunId, Tape, TEXT(""), false,
                                     0, Runtime, Dispatch, GetState);
    }();
  };
}

} // namespace rtk
