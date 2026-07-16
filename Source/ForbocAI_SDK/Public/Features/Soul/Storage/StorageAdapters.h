#pragma once

#include "Core/fp.hpp"
#include "Core/rtk.hpp"
#include "Features/Soul/Storage/StorageTypes.h"

namespace SoulStorage {

/** User Story: As a Soul exporter, I need portable state encrypted and signed locally before API authorization. @fn FORBOCAI_SDK_API func::AsyncResult<FSoulStoragePreparation> prepareSoulStorageAdapter(const FSoul &Soul) */
FORBOCAI_SDK_API func::AsyncResult<FSoulStoragePreparation>
prepareSoulStorageAdapter(const FSoul &Soul);

/** User Story: As a Soul provider endpoint, I need the exact pending immutable bytes and digest resolved without taking ownership before upload succeeds. @fn FORBOCAI_SDK_API func::AsyncResult<FSoulPreparedUpload> requirePreparedSoulStorageAdapter(const FString &TxId) */
FORBOCAI_SDK_API func::AsyncResult<FSoulPreparedUpload>
requirePreparedSoulStorageAdapter(const FString &TxId);

/** User Story: As a failed Soul workflow, I need uncommitted process-local bytes discarded without altering durable catalog state. @fn FORBOCAI_SDK_API func::AsyncResult<rtk::FEmptyPayload> discardSoulStorageAdapter(const FString &TxId) */
FORBOCAI_SDK_API func::AsyncResult<rtk::FEmptyPayload>
discardSoulStorageAdapter(const FString &TxId);

/** User Story: As a confirmed Soul exporter, I need provider metadata atomically committed to the package-owned catalog. @fn FORBOCAI_SDK_API func::AsyncResult<FSoulCatalogEntry> commitSoulStorageAdapter(const FSoulStorageCommit &Commit) */
FORBOCAI_SDK_API func::AsyncResult<FSoulCatalogEntry>
commitSoulStorageAdapter(const FSoulStorageCommit &Commit);

/** User Story: As a Soul importer, I need provider bytes authenticated, decrypted, and normalized entirely within package-owned storage. @fn FORBOCAI_SDK_API func::AsyncResult<FSoul> decryptSoulStoragePayloadAdapter(const TArray<uint8> &Payload) */
FORBOCAI_SDK_API func::AsyncResult<FSoul>
decryptSoulStoragePayloadAdapter(const TArray<uint8> &Payload);

/** User Story: As a Soul browser, I need the package-owned durable catalog listed without API persistence. @fn FORBOCAI_SDK_API func::AsyncResult<TArray<FSoulListItem>> listSoulStorageAdapter(int32 Limit) */
FORBOCAI_SDK_API func::AsyncResult<TArray<FSoulListItem>>
listSoulStorageAdapter(int32 Limit);

/** User Story: As a Soul protocol verifier, I need the complete local authorization record for one transaction. @fn FORBOCAI_SDK_API func::AsyncResult<FSoulCatalogEntry> getSoulStorageEntryAdapter(const FString &TxId) */
FORBOCAI_SDK_API func::AsyncResult<FSoulCatalogEntry>
getSoulStorageEntryAdapter(const FString &TxId);

} // namespace SoulStorage
