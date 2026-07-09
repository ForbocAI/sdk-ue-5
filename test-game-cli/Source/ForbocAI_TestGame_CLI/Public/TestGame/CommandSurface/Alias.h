#pragma once

#include "CoreMinimal.h"
#include "TestGame/CommandSurface/Types.h"
#include "TestGame/TestGameContract.h"

namespace TestGame {
namespace CommandSurface {
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
      : (State.NpcCreateAliasRule = TEXT("substitute_generated_npc_id"),
         State.BridgeValidateCommandRule = TEXT("expand_preset_macro"),
         void());
  return State;
}

} // namespace CommandSurface
} // namespace TestGame
