#pragma once

#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "TestGame/Features/Systems/Harness/CommandRunner/CommandRunnerAdapters.h"
#include "TestGame/Features/Systems/Harness/Game/GameTypes.h"

namespace TestGame {
namespace GameAdapters {

/** User Story: As a systems harness game consumer, I need to invoke parse verdict through a stable signature so the systems harness game workflow remains explicit and composable. @fn inline FParsedVerdict ParseVerdict(const FString &Output) */
inline FParsedVerdict ParseVerdict(const FString &Output) {
  FParsedVerdict Verdict;
  TSharedPtr<FJsonObject> Root;
  const TSharedRef<TJsonReader<>> Reader =
      TJsonReaderFactory<>::Create(Output);
  return !FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid()
             ? Verdict
             : [&]() {
                 const TSharedPtr<FJsonObject> *Action = nullptr;
                 return !Root->TryGetObjectField(TEXT("action"), Action) ||
                                !(*Action)->TryGetStringField(
                                    TEXT("type"), Verdict.ActionType)
                            ? Verdict
                            : [&]() {
                                const TSharedPtr<FJsonObject> *Target = nullptr;
                                double X = 0.0;
                                double Y = 0.0;
                                ((*Action)->TryGetObjectField(
                                     TEXT("targetHex"), Target) &&
                                 (*Target)->TryGetNumberField(TEXT("x"), X) &&
                                 (*Target)->TryGetNumberField(TEXT("y"), Y))
                                    ? (Verdict.TargetHex = FPosition(
                                           static_cast<int32>(X),
                                           static_cast<int32>(Y)),
                                       void())
                                    : void();

                                const TSharedPtr<FJsonObject> *StateDelta =
                                    nullptr;
                                double Suspicion = 0.0;
                                (Root->TryGetObjectField(TEXT("stateDelta"),
                                                         StateDelta) &&
                                 (*StateDelta)->TryGetNumberField(
                                     TEXT("suspicion"), Suspicion))
                                    ? (Verdict.SuspicionDelta =
                                           static_cast<int32>(Suspicion),
                                       void())
                                    : void();

                                Verdict.bValid =
                                    !Verdict.ActionType.IsEmpty();
                                return Verdict;
                              }();
               }();
}

/** User Story: As a systems harness game consumer, I need to invoke extract npc id through a stable signature so the systems harness game workflow remains explicit and composable. @fn inline FString ExtractNpcId(const FString &Command) */
inline FString ExtractNpcId(const FString &Command) {
  const TArray<FString> Tokens = CommandRunner::detail::Tokenize(Command);
  return Tokens.Num() > 3 && Tokens[1] == TEXT("npc") &&
                 (Tokens[2] == TEXT("process") || Tokens[2] == TEXT("chat"))
             ? Tokens[3]
             : FString();
}

} // namespace GameAdapters
} // namespace TestGame
