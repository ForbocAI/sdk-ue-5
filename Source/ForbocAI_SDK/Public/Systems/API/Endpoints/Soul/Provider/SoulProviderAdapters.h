#pragma once

#include "Core/fp.hpp"
#include "Components/API/Endpoints/Soul/Provider/ProviderTypes.h"

namespace APISlice::SoulProviderEndpoint {

/** User Story: As a Soul exporter, I need only failed provider uploads retried so an accepted immutable transaction is never resubmitted during retrieval polling. @fn FORBOCAI_SDK_API func::AsyncResult<FSoulProviderUpload> uploadSoulProviderWithRetryAdapter(const FSoulPreparedUpload &Pending, const FString &ExpectedTxId, int32 Attempt) */
FORBOCAI_SDK_API func::AsyncResult<FSoulProviderUpload>
uploadSoulProviderWithRetryAdapter(const FSoulPreparedUpload &Pending,
                                   const FString &ExpectedTxId,
                                   int32 Attempt);

/** User Story: As a Soul verifier, I need provider gateways rotated through RTK Query until one returns bytes matching the authorized digest. @fn FORBOCAI_SDK_API func::AsyncResult<FSoulProviderPayload> retrieveMatchingSoulPayloadAdapter(const TArray<FString> &Urls, const FString &Digest, int32 Attempt) */
FORBOCAI_SDK_API func::AsyncResult<FSoulProviderPayload>
retrieveMatchingSoulPayloadAdapter(const TArray<FString> &Urls,
                                   const FString &Digest, int32 Attempt);

} // namespace APISlice::SoulProviderEndpoint
