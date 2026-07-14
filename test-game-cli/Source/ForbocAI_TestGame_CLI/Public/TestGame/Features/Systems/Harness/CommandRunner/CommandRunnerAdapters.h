#pragma once

#include "TestGame/Features/Systems/Contract/ContractTypes.h"
#include "TestGame/Features/Systems/Harness/CommandRunner/CommandRunnerTypes.h"

namespace TestGame {
namespace CommandRunner {

namespace detail {

inline FString ResolveNpcAlias(const FAliasState &Aliases,
                               const FString &Candidate) {
  const FString *Found = Aliases.NpcAliases.Find(Candidate);
  return Found ? *Found : Candidate;
}

inline FString ExpandBridgePreset(const FAliasState &Aliases,
                                  const FString &RawPayload) {
  return RawPayload.EndsWith(TEXT("-jump"))
             ? [&]() {
                 const FString Alias = RawPayload.LeftChop(5);
                 const FString NpcId = ResolveNpcAlias(Aliases, Alias);
                 return FString::Printf(
                     TEXT("{\"action\":{\"type\":\"jump\",\"distance\":3},"
                          "\"context\":{\"constraints\":{\"maxDistance\":2}},"
                          "\"npcId\":\"%s\"}"),
                     *NpcId);
               }()
             : RawPayload;
}

inline FString MapToCommandKey(const TArray<FString> &Tokens) {
  return Tokens.Num() < 2
             ? FString(TEXT("unknown"))
             : Tokens.Num() < 3
                   ? Tokens[1]
                   : Tokens[1] + FString(TEXT("_")) + Tokens[2];
}

inline TArray<FString> ExtractArgsRecursive(const TArray<FString> &Tokens,
                                            int32 Index,
                                            TArray<FString> Acc) {
  return Index >= Tokens.Num()
             ? Acc
             : (Tokens[Index] == TEXT("--text")
                    ? ExtractArgsRecursive(Tokens, Index + 1, MoveTemp(Acc))
                    : (Acc.Add(Tokens[Index]),
                       ExtractArgsRecursive(Tokens, Index + 1,
                                            MoveTemp(Acc))));
}

inline TArray<FString> ExtractArgs(const TArray<FString> &Tokens) {
  return Tokens.Num() <= 3
             ? TArray<FString>()
             : ExtractArgsRecursive(Tokens, 3, TArray<FString>());
}

inline void TokenizeRecursive(const FString &Command, int32 Index,
                              bool bQuoted, FString Current,
                              TArray<FString> &Tokens) {
  Index >= Command.Len()
      ? (!Current.IsEmpty() ? (Tokens.Add(MoveTemp(Current)), void()) : void())
  : Command[Index] == TEXT('"')
      ? TokenizeRecursive(Command, Index + 1, !bQuoted, MoveTemp(Current),
                          Tokens)
  : !bQuoted && FChar::IsWhitespace(Command[Index])
      ? (!Current.IsEmpty() ? (Tokens.Add(MoveTemp(Current)), void()) : void(),
         TokenizeRecursive(Command, Index + 1, bQuoted, FString(), Tokens))
      : (Current.AppendChar(Command[Index]),
         TokenizeRecursive(Command, Index + 1, bQuoted, MoveTemp(Current),
                           Tokens));
}

inline TArray<FString> Tokenize(const FString &Command) {
  TArray<FString> Tokens;
  TokenizeRecursive(Command, 0, false, FString(), Tokens);
  return Tokens;
}

} // namespace detail

inline FAliasState CreateAliasState(
    const Contract::FContractResponse &ContractResponse) {
  FAliasState State;
  ContractResponse.bValid
      ? (State.NpcCreateAliasRule =
             ContractResponse.AliasRules.NpcCreateAlias,
         State.BridgeValidateCommandRule =
             ContractResponse.AliasRules.BridgeValidateCommand,
         void())
      : void();
  return State;
}

} // namespace CommandRunner
} // namespace TestGame
