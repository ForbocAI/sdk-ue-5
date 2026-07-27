#pragma once

#include "CoreMinimal.h"
#include "Components/Soul/Storage/StorageTypes.h"

namespace SoulStorage::Provider::Response {

/**
 * User Story: As a Soul exporter, I need provider response text structurally
 * decoded before immutable transaction metadata is trusted.
 * @fn bool decodeSoulProviderUploadResponseAdapter( const FString &Json, FSoulProviderUploadResponse &Response)
 */
bool decodeSoulProviderUploadResponseAdapter(
    const FString &Json, FSoulProviderUploadResponse &Response);

} // namespace SoulStorage::Provider::Response
