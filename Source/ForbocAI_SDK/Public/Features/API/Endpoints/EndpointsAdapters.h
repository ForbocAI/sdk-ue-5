#pragma once

#include "EndpointsTypes.h"

namespace APISlice {

namespace Detail {

inline FString ArweaveUploadUrl(const FArweaveUploadInstruction &Instruction) {
  return !Instruction.UploadUrl.IsEmpty() ? Instruction.UploadUrl
                                          : Instruction.GatewayUrl;
}

inline FString ArweaveDownloadUrl(
    const FArweaveDownloadInstruction &Instruction) {
  return !Instruction.DownloadUrl.IsEmpty() ? Instruction.DownloadUrl
                                            : Instruction.GatewayUrl;
}

inline FString ArweaveUploadPayload(const FArweaveUploadRequest &Request) {
  return !Request.Instruction.PayloadJson.IsEmpty()
             ? Request.Instruction.PayloadJson
             : Request.SignedPayload;
}

inline TMap<FString, FString>
ArweaveUploadHeaders(const FArweaveUploadInstruction &Instruction) {
  TMap<FString, FString> Headers;
  Headers.Add(TEXT("Content-Type"),
              Instruction.ContentType.IsEmpty() ? TEXT("application/json")
                                                : Instruction.ContentType);
  return (!Instruction.AuiAuthHeader.IsEmpty()
              ? (Headers.Add(TEXT("Authorization"),
                             Instruction.AuiAuthHeader),
                 void())
              : void(),
          !Instruction.TagsJson.IsEmpty()
              ? (Headers.Add(TEXT("X-Forboc-Tags"), Instruction.TagsJson),
                 void())
              : void(),
          Headers);
}

inline TMap<FString, FString>
ArweaveDownloadHeaders(const FArweaveDownloadInstruction &Instruction) {
  TMap<FString, FString> Headers;
  return (!Instruction.AdiAuthHeader.IsEmpty()
              ? (Headers.Add(TEXT("Authorization"),
                             Instruction.AdiAuthHeader),
                 void())
              : void(),
          Headers);
}

inline rtk::FetchArgs ArweaveUploadFetchArgs(
    const FArweaveUploadRequest &Request) {
  rtk::FetchArgs Args;
  Args.method = TEXT("POST");
  Args.url = ArweaveUploadUrl(Request.Instruction);
  Args.body = ArweaveUploadPayload(Request);
  Args.headers = ArweaveUploadHeaders(Request.Instruction);
  Args.responseHandler = TEXT("text");
  Args.timeout = 60000;
  return Args;
}

inline rtk::FetchArgs ArweaveDownloadFetchArgs(
    const FArweaveDownloadRequest &Request) {
  rtk::FetchArgs Args;
  Args.method = TEXT("GET");
  Args.url = ArweaveDownloadUrl(Request.Instruction);
  Args.headers = ArweaveDownloadHeaders(Request.Instruction);
  Args.responseHandler = TEXT("text");
  Args.timeout = 60000;
  return Args;
}

inline int32 ResponseStatus(
    const rtk::QueryReturnValue<FString> &QueryResult) {
  return QueryResult.meta.hasValue && QueryResult.meta.value.response.hasValue
             ? QueryResult.meta.value.response.value.status
             : QueryResult.error.hasValue
                   ? (QueryResult.error.value.statusCode > 0
                          ? QueryResult.error.value.statusCode
                          : QueryResult.error.value.originalStatus)
                   : 0;
}

inline FString ResponseContent(
    const rtk::QueryReturnValue<FString> &QueryResult) {
  return QueryResult.data.hasValue
             ? QueryResult.data.value
             : QueryResult.error.hasValue ? QueryResult.error.value.data
                                          : FString();
}

inline FString ResponseHeader(
    const rtk::QueryReturnValue<FString> &QueryResult,
    const FString &Name) {
  const TMap<FString, FString> *Headers =
      QueryResult.meta.hasValue && QueryResult.meta.value.response.hasValue
          ? &QueryResult.meta.value.response.value.headers
          : nullptr;
  return Headers && Headers->Find(Name)
             ? *Headers->Find(Name)
             : Headers && Headers->Find(Name.ToLower())
                   ? *Headers->Find(Name.ToLower())
                   : FString();
}

inline FString QueryErrorText(
    const rtk::QueryReturnValue<FString> &QueryResult) {
  return QueryResult.error.hasValue
             ? (!QueryResult.error.value.error.IsEmpty()
                    ? QueryResult.error.value.error
                    : !QueryResult.error.value.status.IsEmpty()
                          ? QueryResult.error.value.status
                          : TEXT("unknown_error"))
             : FString();
}

inline FString UploadTxIdFromContent(const FString &Content) {
  TSharedPtr<FJsonObject> ResponseObject;
  return JsonInterop::ParseJsonObject(Content, ResponseObject) &&
                 ResponseObject.IsValid() &&
                 ResponseObject->HasTypedField<EJson::String>(TEXT("id"))
             ? ResponseObject->GetStringField(TEXT("id"))
             : FString();
}

inline FArweaveUploadResult UploadResultFromQuery(
    const rtk::QueryReturnValue<FString> &QueryResult,
    const FArweaveUploadInstruction &Instruction) {
  const int32 Status = ResponseStatus(QueryResult);
  const FString Content = ResponseContent(QueryResult);
  const bool bStatusOk = !QueryResult.error.hasValue && Status >= 200 &&
                         Status < 300;
  FArweaveUploadResult Result;
  Result.ResponseJson = Content;
  Result.StatusCode = Status;
  Result.Status = FString::FromInt(Status);
  Result.TxId = ResponseHeader(QueryResult, TEXT("x-id"));
  Result.TxId = !Result.TxId.IsEmpty() ? Result.TxId
                                       : UploadTxIdFromContent(Content);
  Result.bSuccess = bStatusOk && !Result.TxId.IsEmpty();
  Result.Error =
      Result.bSuccess
          ? FString()
          : bStatusOk ? FString(TEXT("upload_missing_tx_id"))
                      : Status > 0
                            ? FString::Printf(TEXT("upload_failed_status_%d"),
                                              Status)
                            : FString(TEXT("upload_request_failed:")) +
                                  QueryErrorText(QueryResult);
  Result.ArweaveUrl =
      !Instruction.GatewayUrl.IsEmpty() && !Result.TxId.IsEmpty()
          ? Instruction.GatewayUrl + TEXT("/") + Result.TxId
          : FString();
  return Result;
}

inline FArweaveDownloadResult DownloadResultFromQuery(
    const rtk::QueryReturnValue<FString> &QueryResult,
    const FArweaveDownloadInstruction &Instruction) {
  const int32 Status = ResponseStatus(QueryResult);
  const FString Payload = ResponseContent(QueryResult);
  TSharedPtr<FJsonObject> BodyObject;
  const bool bStatusOk = !QueryResult.error.hasValue && Status >= 200 &&
                         Status < 300;
  const bool bValidJson =
      bStatusOk && JsonInterop::ParseJsonObject(Payload, BodyObject);
  FArweaveDownloadResult Result;
  Result.TxId = !Instruction.ExpectedTxId.IsEmpty() ? Instruction.ExpectedTxId
                                                    : Instruction.TxId;
  Result.Payload = Payload;
  Result.BodyJson = Result.Payload;
  Result.ResponseJson = Result.Payload;
  Result.StatusCode = Status;
  Result.Status = FString::FromInt(Status);
  Result.bSuccess = bValidJson;
  Result.Error = Result.bSuccess
                     ? FString()
                     : Status > 0
                           ? (bStatusOk ? FString(TEXT("download_invalid_json"))
                                        : FString::Printf(
                                              TEXT("download_failed_status_%d"),
                                              Status))
                           : QueryErrorText(QueryResult);
  return Result;
}

} // namespace Detail

} // namespace APISlice
