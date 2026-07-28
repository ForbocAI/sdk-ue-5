#pragma once

#include "Core/JsonInterop.h"
#include "Systems/API/Endpoints/NPC/Generate/Configuration/GenerateConfigurationAdapters.h"
#include "Components/NPC/Generate/GenerateTypes.h"

namespace APISlice::Detail {

/** User Story: As NPC generate endpoint callers, I need the granular attribute request encoded so the prior attributes supplied as context reach the API contract. @fn inline FString EncodeNpcAttributeGenerateRequest(const FNpcAttributeGenerateRequest &Request) */
inline FString EncodeNpcAttributeGenerateRequest(
    const FNpcAttributeGenerateRequest &Request) {
  const TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
  Root->SetStringField(TEXT("context"), Request.Context);
  return JsonInterop::ToJsonString(Root);
}

/** User Story: As the SDK cache owner, I need a malformed generated-attribute response rejected before cache insertion so callers only ever compose valid attribute values. @fn inline bool DecodeNpcAttributeGenerateResponse(const FString &Json, FNpcAttributeGenerateResponse &Response) */
inline bool DecodeNpcAttributeGenerateResponse(
    const FString &Json, FNpcAttributeGenerateResponse &Response) {
  using namespace APISlice::Endpoints::NPCGenerateConfiguration;
  const FGenerateConfigurationData &Data = generateConfigurationData();
  const FGenerateFields &Fields = Data.Fields;
  TSharedPtr<FJsonObject> Root;
  const bool bShape =
      JsonInterop::ParseJsonObject(Json, Root) && Root.IsValid() &&
      Root->HasTypedField<EJson::String>(Fields.Attribute) &&
      Root->HasTypedField<EJson::String>(Fields.Value);
  return !bShape
             ? false
             : [&]() {
                 Response.Attribute = Root->GetStringField(Fields.Attribute);
                 Response.Value = Root->GetStringField(Fields.Value);
                 return true;
               }();
}

} // namespace APISlice::Detail
