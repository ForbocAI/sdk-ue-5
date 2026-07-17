#pragma once

#include "Core/rtk.hpp"
#include "TestGame/Features/Data/DataAdapters.h"
#include "TestGame/Features/Systems/Contract/ContractTypes.h"

namespace TestGame::Contract {

/** User Story: As a contract maintainer, I need endpoint policy and wire-field names loaded into one immutable typed value so transport and parsing cannot drift independently. @fn inline const FContractData &ContractData() */
inline const FContractData &ContractData() {
  static const FContractData Data = []() {
    const DataAdapters::FSettingsSource Source =
        DataAdapters::SettingsSource(TEXT("systems/contract.json"));
    const TSharedRef<FJsonObject> Authorization =
        DataAdapters::ReadObjectField(Source, TEXT("authorization"));
    const TSharedRef<FJsonObject> Request =
        DataAdapters::ReadObjectField(Source, TEXT("request"));
    const TSharedRef<FJsonObject> Separators =
        DataAdapters::ReadObjectField(Source, TEXT("separators"));
    const TSharedRef<FJsonObject> Schema =
        DataAdapters::ReadObjectField(Source, TEXT("schema"));
    FContractData Value;
    Value.DefaultApiUrl =
        DataAdapters::ReadStringField(Source.Root, TEXT("defaultApiUrl"));
    Value.Authorization.Header =
        DataAdapters::ReadStringField(Authorization, TEXT("header"));
    Value.Authorization.Template =
        DataAdapters::ReadStringField(Authorization, TEXT("template"));
    Value.Request.Path =
        DataAdapters::ReadStringField(Request, TEXT("path"));
    Value.Request.Method =
        DataAdapters::ReadStringField(Request, TEXT("method"));
    Value.Separators.TrailingUrl =
        DataAdapters::ReadStringField(Separators, TEXT("trailingUrl"));
#define FORBOCAI_READ_CONTRACT_SCHEMA_FIELD(Name)                        \
  Value.Schema.Name = DataAdapters::ReadStringField(Schema, TEXT(#Name));
    FORBOCAI_CONTRACT_SCHEMA_FIELDS(FORBOCAI_READ_CONTRACT_SCHEMA_FIELD)
#undef FORBOCAI_READ_CONTRACT_SCHEMA_FIELD
    return Value;
  }();
  return Data;
}

/** User Story: As a features systems contract consumer, I need runtime URLs normalized from one authored default so test commands remain portable. @fn inline FString resolveTestGameApiUrl(const FString &ApiUrl) */
inline FString resolveTestGameApiUrl(const FString &ApiUrl) {
  const FContractData &Data = ContractData();
  FString Resolved = ApiUrl.TrimStartAndEnd();
  Resolved = Resolved.IsEmpty() ? Data.DefaultApiUrl : Resolved;
  Resolved.RemoveFromEnd(Data.Separators.TrailingUrl);
  return Resolved;
}

/** User Story: As a features systems contract consumer, I need optional API authentication converted to request headers through one adapter. @fn inline TMap<FString, FString> createTestGameAuthHeaders(const FString &ApiKey) */
inline TMap<FString, FString>
createTestGameAuthHeaders(const FString &ApiKey) {
  return func::match(
      func::fromNullable(ApiKey, !ApiKey.IsEmpty()),
      [](const FString &ValidApiKey) {
        const FContractAuthorizationData &Authorization =
            ContractData().Authorization;
        const FString Value = FString::Format(*Authorization.Template,
                                              {ValidApiKey});
        return func::upsert_map_value<FString, FString>(
            TMap<FString, FString>(), Authorization.Header, FString(),
            [Value](const FString &) { return Value; });
      },
      []() { return TMap<FString, FString>(); });
}

/** User Story: As a features systems contract consumer, I need contract requests assembled from authored URL, method, and auth data so RTK Query owns transport consistently. @fn inline FTestGameContractRequest createTestGameContractRequest(const FString &ApiUrl, const FString &ApiKey) */
inline FTestGameContractRequest
createTestGameContractRequest(const FString &ApiUrl, const FString &ApiKey) {
  FTestGameContractRequest Result;
  Result.Args.url = resolveTestGameApiUrl(ApiUrl) + ContractData().Request.Path;
  Result.Args.method = ContractData().Request.Method;
  Result.Headers = createTestGameAuthHeaders(ApiKey);
  return Result;
}

} // namespace TestGame::Contract
