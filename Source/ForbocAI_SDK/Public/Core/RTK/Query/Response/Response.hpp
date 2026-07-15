#pragma once

#include "Core/RTK/Query/Request/Request.hpp"

namespace rtk {
namespace detail {
inline FetchBaseQueryRequest fetchRequestMeta(const FetchArgs &Args,
                                              const FString &ResolvedUrl,
                                              const TMap<FString, FString> &Headers) {
  FetchBaseQueryRequest Request;
  return (Request.url = ResolvedUrl, Request.method = Args.method,
          Request.body = Args.body, Request.headers = Headers, Request);
}

inline FetchBaseQueryResponse fetchResponseMeta(FHttpResponsePtr Res,
                                                const FString &Content) {
  FetchBaseQueryResponse Response;
  return Res.IsValid()
             ? (Response.status = Res->GetResponseCode(),
                Response.data = Content,
                Response.headers = responseHeaders(Res), Response)
             : Response;
}

inline FetchBaseQueryMeta fetchMeta(
    const FetchBaseQueryRequest &Request,
    const func::Maybe<FetchBaseQueryResponse> &Response) {
  FetchBaseQueryMeta Meta;
  Meta.request = Request;
  Meta.response = Response;
  return Meta;
}

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
                             Code, Content, TEXT("JSON deserialization failed")),
                         Meta);
}

template <typename T>
void resolveFetchCompletion(
    const std::function<void(QueryReturnValue<T>)> &Resolve,
    const FetchBaseQueryRequest &RequestMeta, FHttpResponsePtr Res,
    bool bWasSuccessful) {
  const int32 Code = Res.IsValid() ? Res->GetResponseCode() : 0;
  const FString Content = Res.IsValid() ? Res->GetContentAsString() : TEXT("");
  const func::Maybe<FetchBaseQueryResponse> ResponseMeta =
      Res.IsValid() ? func::just(fetchResponseMeta(Res, Content))
                    : func::nothing<FetchBaseQueryResponse>();
  const func::Maybe<FetchBaseQueryMeta> Meta =
      func::just(fetchMeta(RequestMeta, ResponseMeta));

  Resolve((!bWasSuccessful || !Res.IsValid())
              ? QueryReturnValue<T>::failure(
                    FetchBaseQueryError::fetchError(TEXT("Network failure")),
                    Meta)
              : (Code < 200 || Code >= 300)
                    ? QueryReturnValue<T>::failure(
                          FetchBaseQueryError::httpError(Code, Content), Meta)
                    : decodeFetchContent<T>(Content, Code, Meta));
}

template <typename T, typename Enable> struct JsonDeserializer {
  static bool Deserialize(const FString &Content, T &OutValue) {
    return FJsonObjectConverter::JsonObjectStringToUStruct(Content, &OutValue,
                                                           0, 0);
  }
};

template <> struct JsonDeserializer<FString, void> {
  static bool Deserialize(const FString &Content, FString &OutValue) {
    OutValue = Content;
    return true;
  }
};

template <> struct JsonDeserializer<rtk::FEmptyPayload, void> {
  static bool Deserialize(const FString &Content,
                          rtk::FEmptyPayload &OutValue) {
    return true;
  }
};

template <> struct JsonDeserializer<TArray<FString>, void> {
  static bool Deserialize(const FString &Content, TArray<FString> &OutValue) {
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Content);
    TArray<TSharedPtr<FJsonValue>> JsonValues;
    return !FJsonSerializer::Deserialize(Reader, JsonValues)
               ? false
               : (OutValue.Empty(JsonValues.Num()),
                  deserializeStringArrayRecursive(JsonValues, 0, OutValue));
  }
};

template <typename T> struct JsonDeserializer<TArray<T>, void> {
  static bool Deserialize(const FString &Content, TArray<T> &OutValue) {
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Content);
    TArray<TSharedPtr<FJsonValue>> JsonValues;
    return !FJsonSerializer::Deserialize(Reader, JsonValues)
               ? false
               : (OutValue.Empty(JsonValues.Num()),
                  deserializeStructArrayRecursive<T>(JsonValues, 0, OutValue));
  }
};

} // namespace detail
} // namespace rtk
