#pragma once

#include "Core/JsonInterop.h"
#include "Features/Contracts/ContractsTypes.h"

namespace JsonInterop {

inline FArweaveUploadInstruction
UploadInstructionFromObject(const TSharedPtr<FJsonObject> &Object) {
  FArweaveUploadInstruction Instruction;
  return !Object.IsValid()
             ? Instruction
             : (Instruction.UploadUrl = OptionalStringFromField(
                    Object, TEXT("auiEndpoint")),
                Instruction.ContentType = OptionalStringFromField(
                    Object, TEXT("auiContentType")),
                Instruction.AuiAuthHeader = OptionalStringFromField(
                    Object, TEXT("auiAuthHeader")),
                Instruction.PayloadJson = JsonStringFromField(
                    Object, TEXT("auiPayload"), TEXT("{}")),
                Instruction);
}

inline TSharedRef<FJsonObject>
UploadResultToObject(const FArweaveUploadResult &Result) {
  const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
  return (Object->SetStringField(TEXT("aurTxId"), Result.TxId),
          Object->SetNumberField(
              TEXT("aurStatus"),
              Result.StatusCode > 0 ? Result.StatusCode
                                    : FCString::Atoi(*Result.Status)),
          Object->SetBoolField(TEXT("aurSuccess"), Result.bSuccess),
          detail::SetIfNonEmpty(Object, TEXT("aurError"), Result.Error),
          Object);
}

inline FArweaveDownloadInstruction
DownloadInstructionFromObject(const TSharedPtr<FJsonObject> &Object) {
  FArweaveDownloadInstruction Instruction;
  return !Object.IsValid()
             ? Instruction
             : (Instruction.GatewayUrl = OptionalStringFromField(
                    Object, TEXT("adiGatewayUrl")),
                Instruction.DownloadUrl = OptionalStringFromField(
                    Object, TEXT("adiDownloadUrl")),
                Instruction.AdiAuthHeader = OptionalStringFromField(
                    Object, TEXT("adiAuthHeader")),
                Instruction.TxId = OptionalStringFromField(
                    Object, TEXT("adiExpectedTxId")),
                Instruction.ExpectedTxId = Instruction.TxId, Instruction);
}

inline TSharedRef<FJsonObject>
DownloadResultToObject(const FArweaveDownloadResult &Result) {
  const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
  return (SetFieldFromJsonString(
              Object, TEXT("adrBody"),
              !Result.BodyJson.IsEmpty() ? Result.BodyJson : Result.Payload,
              false),
          Object->SetNumberField(TEXT("adrStatus"), Result.StatusCode),
          Object->SetBoolField(TEXT("adrSuccess"), Result.bSuccess),
          detail::SetIfNonEmpty(Object, TEXT("adrError"), Result.Error),
          Object);
}

} // namespace JsonInterop
