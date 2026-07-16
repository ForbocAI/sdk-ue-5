#include "Features/Soul/Storage/Provider/StorageProviderAdapters.h"

#include "Features/Soul/Storage/Configuration/StorageConfigurationAdapters.h"
#include "Internationalization/Regex.h"

namespace SoulStorage::Provider {
namespace {

/** User Story: As a provider URL builder, I need only trailing separators removed so URI schemes and paths remain intact. @fn FString trimSoulProviderTrailingSeparatorsAdapter(const FString &Value) */
FString trimSoulProviderTrailingSeparatorsAdapter(const FString &Value) {
  const Configuration::FSoulStorageConfigurationData &Data =
      Configuration::soulStorageData();
  return Value.EndsWith(Data.Provider.GatewayPathSeparator)
             ? trimSoulProviderTrailingSeparatorsAdapter(
                   Value.LeftChop(Data.Numbers.One))
             : Value;
}

/** User Story: As a provider URL normalizer, I need absolute and host-only cache locations represented by the same canonical base contract. @fn FString soulProviderBaseUrlAdapter(const FString &Cache) */
FString soulProviderBaseUrlAdapter(const FString &Cache) {
  const Configuration::FProviderData &Data =
      Configuration::soulStorageData().Provider;
  const FString Normalized = trimSoulProviderTrailingSeparatorsAdapter(Cache);
  FRegexMatcher Matcher(FRegexPattern(Data.AbsoluteUrlPattern), Normalized);
  return Matcher.FindNext() ? Normalized : Data.GatewayScheme + Normalized;
}

} // namespace

/**
 * User Story: As a Soul retriever, I need canonical gateways and every provider-advertised cache normalized and deduplicated before polling.
 * @fn TArray<FString> soulProviderUrlsAdapter( const FSoulProviderUploadResponse &Response)
 */
TArray<FString> soulProviderUrlsAdapter(
    const FSoulProviderUploadResponse &Response) {
  const Configuration::FProviderData &Data =
      Configuration::soulStorageData().Provider;
  const TArray<FString> Caches = func::append_values<FString>(
      func::append_values<FString>(Data.RetrievalGateways,
                                   Response.DataCaches),
      Response.FastFinalityIndexes);
  return func::fold_array<FString, TArray<FString>>(
      Caches, TArray<FString>(),
      [&Data, &Response](const TArray<FString> &Urls, const FString &Cache) {
        return func::append_unique_value<FString>(
            Urls, soulProviderBaseUrlAdapter(Cache) +
                      Data.GatewayPathSeparator + Response.Id);
      });
}

/** User Story: As a Soul verifier, I need one eventual-consistency delay per complete gateway cycle so unavailable edges do not slow canonical polling. @fn int32 soulProviderRetryDelayAdapter(int32 NextAttempt, int32 UrlCount) */
int32 soulProviderRetryDelayAdapter(int32 NextAttempt, int32 UrlCount) {
  const Configuration::FSoulStorageConfigurationData &Data =
      Configuration::soulStorageData();
  return UrlCount > Data.Numbers.Zero &&
                 NextAttempt % UrlCount == Data.Numbers.Zero
             ? Data.Retrieval.DelayMs
             : Data.Numbers.Zero;
}

/** User Story: As a Soul verifier, I need retry limits measured in complete gateway cycles so dynamic provider discovery cannot shorten the consistency window. @fn bool soulProviderRetryExhaustedAdapter(int32 NextAttempt, int32 UrlCount) */
bool soulProviderRetryExhaustedAdapter(int32 NextAttempt, int32 UrlCount) {
  const Configuration::FSoulStorageConfigurationData &Data =
      Configuration::soulStorageData();
  return UrlCount <= Data.Numbers.Zero ||
         NextAttempt >= Data.Retrieval.MaximumCycles * UrlCount;
}

} // namespace SoulStorage::Provider
