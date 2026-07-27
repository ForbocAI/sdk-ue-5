#pragma once

#include "Components/Soul/SoulTypes.h"
#include "Components/Soul/Storage/StorageTypes.h"

namespace SoulAdapters {

/**
 * User Story: As an NPC owner, I need one pure projection that snapshots all
 * portable character state before encryption.
 * @fn inline FSoul createSoulAdapter(const FString &NpcId, const FString &StructuredPersona, const FAgentState &State, const TArray<FMemoryItem> &Memories, const FString &Name, const FString &Version)
 */
inline FSoul createSoulAdapter(const FString &NpcId,
                               const FString &StructuredPersona,
                               const FAgentState &State,
                               const TArray<FMemoryItem> &Memories,
                               const FString &Name,
                               const FString &Version) {
  FSoul Soul;
  Soul.Id = NpcId;
  Soul.Version = Version;
  Soul.Name = Name;
  Soul.StructuredPersona = StructuredPersona;
  Soul.State = State;
  Soul.Memories = Memories;
  return Soul;
}

/**
 * User Story: As a Soul importer, I need authenticated portable state projected
 * into the canonical NPC contract without API-owned reconstruction.
 * @fn inline FImportedNpc toImportedNpcAdapter(const FSoul &Soul)
 */
inline FImportedNpc toImportedNpcAdapter(const FSoul &Soul) {
  return TypeFactory::ImportedNpc(Soul.Id, Soul.StructuredPersona,
                                  Soul.State.JsonData);
}

/**
 * User Story: As a Soul exporter, I need API authorization rejected unless it
 * binds the exact local transaction, digest, provider, and NPC.
 * @fn inline bool matchesSoulExportPreparationAdapter( const FSoulStoragePreparation &Local, const FSoulExportPreparation &Remote, const FString &NpcId)
 */
inline bool matchesSoulExportPreparationAdapter(
    const FSoulStoragePreparation &Local,
    const FSoulExportPreparation &Remote, const FString &NpcId) {
  return Remote.NpcIdRef == NpcId &&
         Remote.TransactionId == Local.TxId &&
         Remote.PayloadDigest == Local.Digest &&
         Remote.Provider == Local.Provider;
}

/**
 * User Story: As a Soul catalog owner, I need API confirmation rejected unless
 * it names the exact externally retrieved provider transaction.
 * @fn inline bool matchesSoulExportConfirmationAdapter( const FSoulStorageReceipt &Local, const FSoulExportResponse &Remote)
 */
inline bool matchesSoulExportConfirmationAdapter(
    const FSoulStorageReceipt &Local, const FSoulExportResponse &Remote) {
  return Remote.TxId == Local.TxId && Remote.StorageUrl == Local.Url;
}

/**
 * User Story: As a Soul verifier, I need catalog metadata projected into the
 * API verification contract without dispatching or reading runtime state.
 * @fn inline FSoulVerificationRequest createSoulVerificationRequestAdapter(const FSoulCatalogEntry &Entry)
 */
inline FSoulVerificationRequest
createSoulVerificationRequestAdapter(const FSoulCatalogEntry &Entry) {
  FSoulVerificationRequest Request;
  Request.NpcIdRef = Entry.NpcId;
  Request.PayloadDigest = Entry.Digest;
  Request.Provider = Entry.Provider;
  Request.Signature = Entry.Signature;
  return Request;
}

} // namespace SoulAdapters
