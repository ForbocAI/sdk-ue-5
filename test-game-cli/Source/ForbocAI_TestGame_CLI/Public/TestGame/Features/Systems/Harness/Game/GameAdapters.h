#pragma once

#include "Internationalization/Regex.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "TestGame/Features/Data/DataAdapters.h"
#include "TestGame/Features/Systems/Harness/Game/GameTypes.h"
#include "TestGame/Features/Systems/Harness/Game/Vocabulary/VocabularyAdapters.h"

namespace TestGame::GameAdapters {
namespace detail {

/** User Story: As a harness maintainer, I need configured assertions decoded recursively so contract-command ordering remains deterministic on every host. @fn inline TArray<FOutputAssertion> ReadOutputAssertions( const TArray<TSharedPtr<FJsonValue>> &Values, int32 Index, int32 NextIndex, TArray<FOutputAssertion> Acc) */
inline TArray<FOutputAssertion> ReadOutputAssertions(
    const TArray<TSharedPtr<FJsonValue>> &Values, int32 Index,
    int32 NextIndex, TArray<FOutputAssertion> Acc) {
  return Index >= Values.Num()
             ? Acc
             : [&]() {
                 const TSharedRef<FJsonObject> Object =
                     Values[Index]->AsObject().ToSharedRef();
                 FOutputAssertion Assertion;
                 Assertion.Kind =
                     DataAdapters::ReadStringField(Object, TEXT("kind"));
                 Assertion.Value =
                     DataAdapters::ReadStringField(Object, TEXT("value"));
                 Acc.Add(MoveTemp(Assertion));
                 return ReadOutputAssertions(
                     Values, Index + NextIndex, NextIndex, MoveTemp(Acc));
               }();
}

/** User Story: As a harness presenter, I need template values applied recursively so authored summary text remains separate from selector behavior. @fn inline FString FormatTemplateKeys(const FString &Template, const TMap<FString, FString> &Values, const TArray<FString> &Keys, int32 Index, int32 NextIndex, const FString &TokenPrefix, const FString &TokenSuffix) */
inline FString FormatTemplateKeys(const FString &Template,
                                  const TMap<FString, FString> &Values,
                                  const TArray<FString> &Keys, int32 Index,
                                  int32 NextIndex,
                                  const FString &TokenPrefix,
                                  const FString &TokenSuffix) {
  return Index >= Keys.Num()
             ? Template
             : FormatTemplateKeys(
                   Template.Replace(
                       *(TokenPrefix + Keys[Index] + TokenSuffix),
                       *Values.FindChecked(Keys[Index])),
                   Values, Keys, Index + NextIndex, NextIndex, TokenPrefix,
                   TokenSuffix);
}

} // namespace detail

/** User Story: As a harness maintainer, I need CLI game configuration loaded once from authored data so timing, errors, and the contract command remain portable. @fn inline FGameData ReadGameData() */
inline FGameData ReadGameData() {
  const DataAdapters::FSettingsSource Source =
      DataAdapters::SettingsSource(TEXT("harness/game.json"));
  const TSharedRef<FJsonObject> Autoplay =
      DataAdapters::ReadObjectField(Source, TEXT("autoplay"));
  const TSharedRef<FJsonObject> ContractCommand =
      DataAdapters::ReadObjectField(Source, TEXT("contractCommand"));
  const TSharedRef<FJsonObject> Environment =
      DataAdapters::ReadObjectField(Source, TEXT("environment"));
  const TSharedRef<FJsonObject> Messages =
      DataAdapters::ReadObjectField(Source, TEXT("messages"));
  const TSharedRef<FJsonObject> Numbers =
      DataAdapters::ReadObjectField(Source, TEXT("numbers"));
  const TSharedRef<FJsonObject> Output =
      DataAdapters::ReadObjectField(Source, TEXT("output"));
  FGameData Data;
  Data.autoplay.dots =
      DataAdapters::ReadStringArrayField(Autoplay, TEXT("dots"));
  Data.autoplay.dotDelayMs =
      DataAdapters::ReadNumberField(Autoplay, TEXT("dotDelayMs"));
  Data.contractCommand.Group =
      DataAdapters::ReadStringField(ContractCommand, TEXT("group"));
  Data.contractCommand.Command =
      DataAdapters::ReadStringField(ContractCommand, TEXT("command"));
  Data.contractCommand.ExpectedRoutes = DataAdapters::ReadStringArrayField(
      ContractCommand, TEXT("expectedRoutes"));
  Data.contractCommand.OutputAssertions = detail::ReadOutputAssertions(
      DataAdapters::ReadObjectArrayField(ContractCommand,
                                         TEXT("outputAssertions")),
      GameRuntimeData().numbers.emptyCount,
      GameRuntimeData().numbers.nextIndex, {});
  Data.environment.commandDelayKey =
      DataAdapters::ReadStringField(Environment, TEXT("commandDelayKey"));
  Data.environment.defaultCommandDelayMs = DataAdapters::ReadNumberField(
      Environment, TEXT("defaultCommandDelayMs"));

#define FORBOCAI_READ_GAME_MESSAGE(Type, Name)                            \
  Data.messages.Name =                                                    \
      DataAdapters::ReadStringField(Messages, TEXT(#Name));
  FORBOCAI_GAME_MESSAGE_FIELDS(FORBOCAI_READ_GAME_MESSAGE)
#undef FORBOCAI_READ_GAME_MESSAGE

#define FORBOCAI_READ_GAME_NUMBER(Type, Name)                             \
  Data.numbers.Name =                                                     \
      DataAdapters::ReadNumberField(Numbers, TEXT(#Name));
  FORBOCAI_GAME_NUMBER_FIELDS(FORBOCAI_READ_GAME_NUMBER)
#undef FORBOCAI_READ_GAME_NUMBER

  Data.output.emptyLines =
      DataAdapters::ReadStringArrayField(Output, TEXT("emptyLines"));
  Data.output.emptyText =
      DataAdapters::ReadStringField(Output, TEXT("emptyText"));
  Data.output.lineBreak =
      DataAdapters::ReadStringField(Output, TEXT("lineBreak"));
  check(Data.contractCommand.Group == GameRuntimeData().commandGroups.contract);
  return Data;
}

/** User Story: As a harness consumer, I need one immutable game-configuration instance so every CLI run uses the same authored timing and contract command. @fn inline const FGameData &GameData() */
inline const FGameData &GameData() {
  static const FGameData Data = ReadGameData();
  return Data;
}

/** User Story: As a harness presenter, I need named templates formatted from data so selectors contain behavior but no authored prose. @fn inline FString FormatGameTemplate(const FString &Template, const TMap<FString, FString> &Values) */
inline FString FormatGameTemplate(const FString &Template,
                                  const TMap<FString, FString> &Values) {
  TArray<FString> Keys;
  Values.GetKeys(Keys);
  return detail::FormatTemplateKeys(
      Template, Values, Keys, GameRuntimeData().numbers.emptyCount,
      GameRuntimeData().numbers.nextIndex,
      GameRuntimeData().templateData.tokenPrefix,
      GameRuntimeData().templateData.tokenSuffix);
}

/** User Story: As a systems harness game consumer, I need to invoke parse verdict through a stable signature so the systems harness game workflow remains explicit and composable. @fn inline FParsedVerdict ParseVerdict(const FString &Output) */
inline FParsedVerdict ParseVerdict(const FString &Output) {
  const FGameRuntimeWireFields &Fields = GameRuntimeData().wireFields;
  FParsedVerdict Verdict;
  TSharedPtr<FJsonObject> Root;
  const TSharedRef<TJsonReader<>> Reader =
      TJsonReaderFactory<>::Create(Output);
  return !FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid()
             ? Verdict
             : [&]() {
                 const TSharedPtr<FJsonObject> *Action = nullptr;
                 return !Root->TryGetObjectField(Fields.action, Action) ||
                                !(*Action)->TryGetStringField(
                                    Fields.type, Verdict.ActionType)
                            ? Verdict
                            : [&]() {
                                const TSharedPtr<FJsonObject> *Target = nullptr;
                                double X{};
                                double Y{};
                                ((*Action)->TryGetObjectField(
                                     Fields.targetHex, Target) &&
                                 (*Target)->TryGetNumberField(Fields.x, X) &&
                                 (*Target)->TryGetNumberField(Fields.y, Y))
                                    ? (Verdict.TargetHex = FPosition(
                                           static_cast<int32>(X),
                                           static_cast<int32>(Y)),
                                       void())
                                    : void();

                                const TSharedPtr<FJsonObject> *StateDelta =
                                    nullptr;
                                double Suspicion{};
                                (Root->TryGetObjectField(Fields.stateDelta,
                                                         StateDelta) &&
                                 (*StateDelta)->TryGetNumberField(
                                     Fields.suspicion, Suspicion))
                                    ? (Verdict.SuspicionDelta =
                                           static_cast<int32>(Suspicion),
                                       void())
                                    : void();

                                Verdict.bValid = !Verdict.ActionType.IsEmpty();
                                return Verdict;
                              }();
               }();
}

/** User Story: As a systems harness game consumer, I need NPC identifiers extracted from authored command syntax so command-result reducers do not duplicate token positions. @fn inline FString ExtractNpcId(const FString &Command) */
inline FString ExtractNpcId(const FString &Command) {
  const FRegexPattern Pattern(GameRuntimeData().patterns.npcCommand);
  FRegexMatcher Matcher(Pattern, Command);
  return Matcher.FindNext()
             ? Matcher.GetCaptureGroup(GameRuntimeData().numbers.captureIndex)
             : FString();
}

} // namespace TestGame::GameAdapters
