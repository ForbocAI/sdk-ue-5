#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Core/RTK/Query/Request/Request.hpp"

namespace rtk {
namespace detail {
/** User Story: As a rtk query response consumer, I need to invoke fetch request meta through a stable signature so the rtk query response workflow remains explicit and composable. @fn inline FetchBaseQueryRequest fetchRequestMeta(const FetchArgs &Args, const FString &ResolvedUrl, const TMap<FString, FString> &Headers) */
inline FetchBaseQueryRequest fetchRequestMeta(const FetchArgs &Args,
                                              const FString &ResolvedUrl,
                                              const TMap<FString, FString> &Headers) {
  FetchBaseQueryRequest Request;
  return (Request.url = ResolvedUrl, Request.method = Args.method,
          Request.body = Args.body, Request.headers = Headers, Request);
}

/** User Story: As a rtk query response consumer, I need to invoke fetch response meta through a stable signature so the rtk query response workflow remains explicit and composable. @fn inline FetchBaseQueryResponse fetchResponseMeta(FHttpResponsePtr Res, const FString &Content) */
inline FetchBaseQueryResponse fetchResponseMeta(FHttpResponsePtr Res,
                                                const FString &Content) {
  FetchBaseQueryResponse Response;
  return Res.IsValid()
             ? (Response.status = Res->GetResponseCode(),
                Response.data = Content,
                Response.headers = responseHeaders(Res), Response)
             : Response;
}

/** User Story: As a rtk query response consumer, I need to invoke fetch meta through a stable signature so the rtk query response workflow remains explicit and composable. @fn inline FetchBaseQueryMeta fetchMeta( const FetchBaseQueryRequest &Request, const func::Maybe<FetchBaseQueryResponse> &Response) */
inline FetchBaseQueryMeta fetchMeta(
    const FetchBaseQueryRequest &Request,
    const func::Maybe<FetchBaseQueryResponse> &Response) {
  FetchBaseQueryMeta Meta;
  Meta.request = Request;
  Meta.response = Response;
  return Meta;
}

/** User Story: As an RTK Query transport, I need one canonical network failure value shared by text, JSON, and binary response handlers. @fn inline FetchBaseQueryError fetchNetworkFailureError() */
inline FetchBaseQueryError fetchNetworkFailureError() {
  return FetchBaseQueryError::fetchError(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV5AFDDD9C0874));
}

/** User Story: As a rtk query response consumer, I need to invoke decode fetch content through a stable signature so the rtk query response workflow remains explicit and composable. @fn template <typename T> QueryReturnValue<T> decodeFetchContent( const FString &Content, int32 Code, const func::Maybe<FetchBaseQueryMeta> &Meta) */
template <typename T>
QueryReturnValue<T> decodeFetchContent(
    const FString &Content, int32 Code,
    const func::Maybe<FetchBaseQueryMeta> &Meta) {
  T ResultPayload;
  return Content.IsEmpty()
             ? QueryReturnValue<T>::success(ResultPayload, Meta)
             : JsonDeserializer<T>::Deserialize(Content, ResultPayload)
                   ? QueryReturnValue<T>::success(ResultPayload, Meta)
                   : QueryReturnValue<T>::failure(
                         FetchBaseQueryError::parsingError(
                             Code, Content, TEXT(FORBOCAI_SDK_AUTHORED_STRINGV3C1F951AB2E8)),
                         Meta);
}

/** User Story: As a rtk query response consumer, I need to invoke resolve fetch completion through a stable signature so the rtk query response workflow remains explicit and composable. @fn template <typename T> void resolveFetchCompletion( const std::function<void(QueryReturnValue<T>)> &Resolve, const FetchBaseQueryRequest &RequestMeta, FHttpResponsePtr Res, bool bWasSuccessful) */
template <typename T>
void resolveFetchCompletion(
    const std::function<void(QueryReturnValue<T>)> &Resolve,
    const FetchBaseQueryRequest &RequestMeta, FHttpResponsePtr Res,
    bool bWasSuccessful) {
  const int32 Code = Res.IsValid() ? Res->GetResponseCode() : FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA;
  const FString Content = Res.IsValid() ? Res->GetContentAsString() : TEXT("");
  const func::Maybe<FetchBaseQueryResponse> ResponseMeta =
      Res.IsValid() ? func::just(fetchResponseMeta(Res, Content))
                    : func::nothing<FetchBaseQueryResponse>();
  const func::Maybe<FetchBaseQueryMeta> Meta =
      func::just(fetchMeta(RequestMeta, ResponseMeta));

  Resolve((!bWasSuccessful || !Res.IsValid())
              ? QueryReturnValue<T>::failure(
                    fetchNetworkFailureError(), Meta)
              : (Code < FORBOCAI_SDK_AUTHORED_NUMBERVEF1B5401B507 || Code >= FORBOCAI_SDK_AUTHORED_NUMBERV07C0796E1646)
                    ? QueryReturnValue<T>::failure(
                          FetchBaseQueryError::httpError(Code, Content), Meta)
                    : decodeFetchContent<T>(Content, Code, Meta));
}

/** User Story: As an RTK Query binary response consumer, I need response bytes preserved exactly while retaining normal fetch metadata and failure semantics. @fn template <> inline void resolveFetchCompletion<TArray<uint8>>( const std::function<void(QueryReturnValue<TArray<uint8>>)> &Resolve, const FetchBaseQueryRequest &RequestMeta, FHttpResponsePtr Res, bool bWasSuccessful) */
template <>
inline void resolveFetchCompletion<TArray<uint8>>(
    const std::function<void(QueryReturnValue<TArray<uint8>>)> &Resolve,
    const FetchBaseQueryRequest &RequestMeta, FHttpResponsePtr Res,
    bool bWasSuccessful) {
  const int32 Code = Res.IsValid() ? Res->GetResponseCode() : FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA;
  const FString Diagnostic =
      Res.IsValid() ? Res->GetContentAsString() : FString();
  const func::Maybe<FetchBaseQueryResponse> ResponseMeta =
      Res.IsValid() ? func::just(fetchResponseMeta(Res, Diagnostic))
                    : func::nothing<FetchBaseQueryResponse>();
  const func::Maybe<FetchBaseQueryMeta> Meta =
      func::just(fetchMeta(RequestMeta, ResponseMeta));
  const QueryReturnValue<TArray<uint8>> Result =
      (!bWasSuccessful || !Res.IsValid())
          ? QueryReturnValue<TArray<uint8>>::failure(
                fetchNetworkFailureError(), Meta)
          : (Code < FORBOCAI_SDK_AUTHORED_NUMBERVEF1B5401B507 || Code >= FORBOCAI_SDK_AUTHORED_NUMBERV07C0796E1646)
                ? QueryReturnValue<TArray<uint8>>::failure(
                      FetchBaseQueryError::httpError(Code, Diagnostic), Meta)
                : QueryReturnValue<TArray<uint8>>::success(Res->GetContent(),
                                                           Meta);
  /** User Story: As a rtk query response consumer, I need to invoke resolve through a stable signature so the rtk query response workflow remains explicit and composable. @fn Resolve(Result) */
  Resolve(Result);
}

template <typename T, typename Enable> struct JsonDeserializer {
  /** User Story: As a rtk query response consumer, I need to invoke deserialize through a stable signature so the rtk query response workflow remains explicit and composable. @fn static bool Deserialize(const FString &Content, T &OutValue) */
  static bool Deserialize(const FString &Content, T &OutValue) {
    return FJsonObjectConverter::JsonObjectStringToUStruct(Content, &OutValue,
                                                           FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA, FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA);
  }
};

template <> struct JsonDeserializer<FString, void> {
  /** User Story: As a rtk query response consumer, I need to invoke deserialize through a stable signature so the rtk query response workflow remains explicit and composable. @fn static bool Deserialize(const FString &Content, FString &OutValue) */
  static bool Deserialize(const FString &Content, FString &OutValue) {
    OutValue = Content;
    return true;
  }
};

template <> struct JsonDeserializer<rtk::FEmptyPayload, void> {
  /** User Story: As a rtk query response consumer, I need to invoke deserialize through a stable signature so the rtk query response workflow remains explicit and composable. @fn static bool Deserialize(const FString &Content, rtk::FEmptyPayload &OutValue) */
  static bool Deserialize(const FString &Content,
                          rtk::FEmptyPayload &OutValue) {
    return true;
  }
};

template <> struct JsonDeserializer<TArray<FString>, void> {
  /** User Story: As a rtk query response consumer, I need to invoke deserialize through a stable signature so the rtk query response workflow remains explicit and composable. @fn static bool Deserialize(const FString &Content, TArray<FString> &OutValue) */
  static bool Deserialize(const FString &Content, TArray<FString> &OutValue) {
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Content);
    TArray<TSharedPtr<FJsonValue>> JsonValues;
    return !FJsonSerializer::Deserialize(Reader, JsonValues)
               ? false
               : (OutValue.Empty(JsonValues.Num()),
                  deserializeStringArrayRecursive(JsonValues, FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA, OutValue));
  }
};

template <typename T> struct JsonDeserializer<TArray<T>, void> {
  /** User Story: As a rtk query response consumer, I need to invoke deserialize through a stable signature so the rtk query response workflow remains explicit and composable. @fn static bool Deserialize(const FString &Content, TArray<T> &OutValue) */
  static bool Deserialize(const FString &Content, TArray<T> &OutValue) {
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Content);
    TArray<TSharedPtr<FJsonValue>> JsonValues;
    return !FJsonSerializer::Deserialize(Reader, JsonValues)
               ? false
               : (OutValue.Empty(JsonValues.Num()),
                  deserializeStructArrayRecursive<T>(JsonValues, FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA, OutValue));
  }
};

} // namespace detail
} // namespace rtk
