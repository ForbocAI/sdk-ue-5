#pragma once

#include "Core/fp.hpp"
#include "Features/Soul/Storage/StorageTypes.h"

namespace SoulStorage::Provider {

/** User Story: As a Soul retriever, I need provider cache locations normalized and deduplicated without performing network effects. @fn FORBOCAI_SDK_API TArray<FString> soulProviderUrlsAdapter(const FSoulProviderUploadResponse &Response) */
FORBOCAI_SDK_API TArray<FString>
soulProviderUrlsAdapter(const FSoulProviderUploadResponse &Response);

/** User Story: As a Soul verifier, I need retry delay policy calculated independently from transport execution. @fn FORBOCAI_SDK_API int32 soulProviderRetryDelayAdapter(int32 NextAttempt, int32 UrlCount) */
FORBOCAI_SDK_API int32 soulProviderRetryDelayAdapter(int32 NextAttempt,
                                                     int32 UrlCount);

/** User Story: As a Soul verifier, I need retry limits measured in complete gateway cycles so dynamic provider discovery cannot shorten the consistency window. @fn FORBOCAI_SDK_API bool soulProviderRetryExhaustedAdapter(int32 NextAttempt, int32 UrlCount) */
FORBOCAI_SDK_API bool soulProviderRetryExhaustedAdapter(int32 NextAttempt,
                                                        int32 UrlCount);

} // namespace SoulStorage::Provider
