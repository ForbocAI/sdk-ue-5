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
  Response.status = Res->GetResponseCode();
  Response.data = Content;
  Response.headers = responseHeaders(Res);
  return Response;
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

/** User Story: As an RTK Query transport, I need HTTP failure classification expressed once so text and binary handlers share the same status boundary. @fn inline bool isFetchHttpError(int32 Code) */
inline bool isFetchHttpError(int32 Code) {
  return Code < FORBOCAI_SDK_AUTHORED_NUMBERVEF1B5401B507 ||
         Code >= FORBOCAI_SDK_AUTHORED_NUMBERV07C0796E1646;
}

/** User Story: As an RTK Query transport, I need network failure construction shared across payload types so absent responses retain identical metadata. @fn template <typename T> QueryReturnValue<T> fetchNetworkFailure(const func::Maybe<FetchBaseQueryMeta> &Meta) */
template <typename T>
QueryReturnValue<T>
fetchNetworkFailure(const func::Maybe<FetchBaseQueryMeta> &Meta) {
  return QueryReturnValue<T>::failure(fetchNetworkFailureError(), Meta);
}

/** User Story: As an RTK Query JSON consumer, I need engine out-parameter decoding lifted into Maybe before transport result classification. @fn template <typename T> func::Maybe<T> decodeFetchPayload(const FString &Content) */
template <typename T>
func::Maybe<T> decodeFetchPayload(const FString &Content) {
  T ResultPayload{};
  const bool bDecoded =
      JsonDeserializer<T>::Deserialize(Content, ResultPayload);
  return func::fromNullable(MoveTemp(ResultPayload), bDecoded);
}

/** User Story: As a rtk query response consumer, I need to invoke decode fetch content through a stable signature so the rtk query response workflow remains explicit and composable. @fn template <typename T> QueryReturnValue<T> decodeFetchContent( const FString &Content, int32 Code, const func::Maybe<FetchBaseQueryMeta> &Meta) */
template <typename T>
QueryReturnValue<T> decodeFetchContent(
    const FString &Content, int32 Code,
    const func::Maybe<FetchBaseQueryMeta> &Meta) {
  return func::multi_match<bool, QueryReturnValue<T>>(
      Content.IsEmpty(),
      {func::when<bool, QueryReturnValue<T>>(
          func::equals<bool>(true),
          [&Meta](const bool &) {
            return QueryReturnValue<T>::success(T{}, Meta);
          })},
      [&Content, Code, &Meta](const bool &) {
        return func::match(
            decodeFetchPayload<T>(Content),
            [&Meta](const T &Payload) {
              return QueryReturnValue<T>::success(Payload, Meta);
            },
            [&Content, Code, &Meta]() {
              return QueryReturnValue<T>::failure(
                  FetchBaseQueryError::parsingError(
                      Code, Content,
                      TEXT(FORBOCAI_SDK_AUTHORED_STRINGV3C1F951AB2E8)),
                  Meta);
            });
      });
}

/** User Story: As a rtk query response consumer, I need to invoke resolve fetch completion through a stable signature so the rtk query response workflow remains explicit and composable. @fn template <typename T> void resolveFetchCompletion( const std::function<void(QueryReturnValue<T>)> &Resolve, const FetchBaseQueryRequest &RequestMeta, FHttpResponsePtr Res, bool bWasSuccessful) */
template <typename T>
void resolveFetchCompletion(
    const std::function<void(QueryReturnValue<T>)> &Resolve,
    const FetchBaseQueryRequest &RequestMeta, FHttpResponsePtr Res,
    bool bWasSuccessful) {
  const func::Maybe<FHttpResponsePtr> Response =
      func::fromNullable(Res, static_cast<bool>(Res.Get()));
  const func::Maybe<FetchBaseQueryResponse> ResponseMeta =
      func::fmap(Response, [](const FHttpResponsePtr &Present) {
        const FString Content = Present->GetContentAsString();
        return fetchResponseMeta(Present, Content);
      });
  const func::Maybe<FetchBaseQueryMeta> Meta =
      func::just(fetchMeta(RequestMeta, ResponseMeta));
  const QueryReturnValue<T> Result = func::match(
      Response,
      [bWasSuccessful, &Meta](const FHttpResponsePtr &Present) {
        const int32 Code = Present->GetResponseCode();
        const FString Content = Present->GetContentAsString();
        return func::multi_match<bool, QueryReturnValue<T>>(
            bWasSuccessful,
            {func::when<bool, QueryReturnValue<T>>(
                func::equals<bool>(false),
                [&Meta](const bool &) { return fetchNetworkFailure<T>(Meta); })},
            [&Content, Code, &Meta](const bool &) {
              return func::multi_match<int32, QueryReturnValue<T>>(
                  Code,
                  {func::when<int32, QueryReturnValue<T>>(
                      [](const int32 &Status) {
                        return isFetchHttpError(Status);
                      },
                      [&Content, &Meta](const int32 &Status) {
                        return QueryReturnValue<T>::failure(
                            FetchBaseQueryError::httpError(Status, Content),
                            Meta);
                      })},
                  [&Content, &Meta](const int32 &Status) {
                    return decodeFetchContent<T>(Content, Status, Meta);
                  });
            });
      },
      [&Meta]() { return fetchNetworkFailure<T>(Meta); });
  Resolve(Result);
}

/** User Story: As an RTK Query binary response consumer, I need response bytes preserved exactly while retaining normal fetch metadata and failure semantics. @fn template <> inline void resolveFetchCompletion<TArray<uint8>>( const std::function<void(QueryReturnValue<TArray<uint8>>)> &Resolve, const FetchBaseQueryRequest &RequestMeta, FHttpResponsePtr Res, bool bWasSuccessful) */
template <>
inline void resolveFetchCompletion<TArray<uint8>>(
    const std::function<void(QueryReturnValue<TArray<uint8>>)> &Resolve,
    const FetchBaseQueryRequest &RequestMeta, FHttpResponsePtr Res,
    bool bWasSuccessful) {
  const func::Maybe<FHttpResponsePtr> Response =
      func::fromNullable(Res, static_cast<bool>(Res.Get()));
  const func::Maybe<FetchBaseQueryResponse> ResponseMeta =
      func::fmap(Response, [](const FHttpResponsePtr &Present) {
        const FString Diagnostic = Present->GetContentAsString();
        return fetchResponseMeta(Present, Diagnostic);
      });
  const func::Maybe<FetchBaseQueryMeta> Meta =
      func::just(fetchMeta(RequestMeta, ResponseMeta));
  const QueryReturnValue<TArray<uint8>> Result =
      func::match(
          Response,
          [bWasSuccessful,
           &Meta](const FHttpResponsePtr &Present) {
            const int32 Code = Present->GetResponseCode();
            const FString Diagnostic = Present->GetContentAsString();
            return func::multi_match<bool,
                                     QueryReturnValue<TArray<uint8>>>(
                bWasSuccessful,
                {func::when<bool, QueryReturnValue<TArray<uint8>>>(
                    func::equals<bool>(false),
                    [&Meta](const bool &) {
                      return fetchNetworkFailure<TArray<uint8>>(Meta);
                    })},
                [&Code, &Diagnostic, &Meta,
                 &Present](const bool &) {
                  return func::multi_match<
                      int32, QueryReturnValue<TArray<uint8>>>(
                      Code,
                      {func::when<int32,
                                  QueryReturnValue<TArray<uint8>>>(
                          [](const int32 &Status) {
                            return isFetchHttpError(Status);
                          },
                          [&Diagnostic,
                           &Meta](const int32 &Status) {
                            return QueryReturnValue<TArray<uint8>>::failure(
                                FetchBaseQueryError::httpError(Status,
                                                               Diagnostic),
                                Meta);
                          })},
                      [&Present,
                       &Meta](const int32 &) {
                        return QueryReturnValue<TArray<uint8>>::success(
                            Present->GetContent(), Meta);
                      });
                });
          },
          [&Meta]() {
            return fetchNetworkFailure<TArray<uint8>>(Meta);
          });
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
    const bool bParsed = FJsonSerializer::Deserialize(Reader, JsonValues);
    return func::match(
        func::fromNullable(MoveTemp(JsonValues), bParsed),
        [&OutValue](const TArray<TSharedPtr<FJsonValue>> &Values) {
          return assignDeserializedValue(deserializeStringArray(Values),
                                         OutValue);
        },
        []() { return false; });
  }
};

template <typename T> struct JsonDeserializer<TArray<T>, void> {
  /** User Story: As a rtk query response consumer, I need to invoke deserialize through a stable signature so the rtk query response workflow remains explicit and composable. @fn static bool Deserialize(const FString &Content, TArray<T> &OutValue) */
  static bool Deserialize(const FString &Content, TArray<T> &OutValue) {
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Content);
    TArray<TSharedPtr<FJsonValue>> JsonValues;
    const bool bParsed = FJsonSerializer::Deserialize(Reader, JsonValues);
    return func::match(
        func::fromNullable(MoveTemp(JsonValues), bParsed),
        [&OutValue](const TArray<TSharedPtr<FJsonValue>> &Values) {
          return assignDeserializedValue(deserializeStructArray<T>(Values),
                                         OutValue);
        },
        []() { return false; });
  }
};

} // namespace detail
} // namespace rtk
