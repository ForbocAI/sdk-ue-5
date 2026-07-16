#pragma once

#include "Core/fp.hpp"
#include "Features/CLI/Soul/CLISoulTypes.h"
#include "Features/Soul/SoulTypes.h"

namespace ForbocAI {
namespace CLI {
struct FCLIState;
namespace Soul {

/** User Story: As a CLI Soul consumer, I need the Soul CLI subdomain selected from the root CLI state through a stable signature so one store remains authoritative. @fn const FCLISoulState &selectCliSoul(const FCLIState &State) */
const FCLISoulState &selectCliSoul(const FCLIState &State);

/** User Story: As a CLI Soul consumer, I need Soul transaction identifiers derived through a stable signature so list output proves which records are observable. @fn inline TArray<FString> selectSoulTransactionIds(const TArray<FSoulListItem> &Souls) */
inline TArray<FString>
selectSoulTransactionIds(const TArray<FSoulListItem> &Souls) {
  return func::map_array<FSoulListItem, FString>(
      Souls, [](const FSoulListItem &Soul) { return Soul.TxId; });
}

/** User Story: As an SDK verifier, I need Soul list output derived through a stable signature so semantic assertions can inspect the actual transaction identifiers. @fn inline FString selectSoulListOutput(const TArray<FSoulListItem> &Souls, const FCLISoulState &State) */
inline FString selectSoulListOutput(const TArray<FSoulListItem> &Souls,
                                    const FCLISoulState &State) {
  const TArray<FString> TransactionIds = selectSoulTransactionIds(Souls);
  return TransactionIds.Num() == State.Limits.EmptyItemCount
             ? State.Messages.ListEmpty
             : FString::Join(TransactionIds, *State.Syntax.ListSeparator);
}

} // namespace Soul
} // namespace CLI
} // namespace ForbocAI
