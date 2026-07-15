#pragma once

#include "Features/API/Transport/Codec/TransportCodecAdapters.h"

namespace APISlice::Detail {

inline rtk::FetchBaseQueryArgs ApiBaseQueryOptions() {
  rtk::FetchBaseQueryArgs Options;
  const FString ApiKey = SDKConfig::GetApiKey();
  !ApiKey.IsEmpty()
      ? (Options.headers.Add(TEXT("Authorization"),
                             FString(TEXT("Bearer ")) + ApiKey),
         void())
      : void();
  return Options;
}

inline rtk::FetchArgs ApiFetchArgs(const FString &Method, const FString &Url,
                                   const FString &Body = TEXT("")) {
  rtk::FetchArgs Args;
  Args.method = Method;
  Args.url = Url;
  Args.body = Body;
  return Args;
}

template <typename Result>
inline func::AsyncResult<rtk::QueryReturnValue<Result>>
ExecuteApiBaseQuery(const FString &Method, const FString &Url,
                    const FString &Body = TEXT("")) {
  return rtk::fetchBaseQuery<Result>(ApiBaseQueryOptions())(
      ApiFetchArgs(Method, Url, Body), rtk::BaseQueryApi(),
      rtk::FEmptyPayload{});
}

/**
 * Builds an injected RTK-style endpoint thunk from request metadata.
 * User Story: As API endpoint wiring, I need one endpoint factory so store
 * integration, tags, and request builders stay consistent across endpoints.
 */
template <typename Arg, typename Result>
inline ThunkAction<Result, FRuntimeState> MakeEndpoint(
    const FString &EndpointName, const Arg &ArgValue,
    std::function<func::AsyncResult<rtk::QueryReturnValue<Result>>(
        const Arg &)>
        RequestBuilder,
    const TArray<FApiEndpointTag> &providesTags = TArray<FApiEndpointTag>(),
    const TArray<FApiEndpointTag> &invalidatesTags =
        TArray<FApiEndpointTag>()) {
  ApiEndpoint<Arg, Result> Endpoint;
  Endpoint.EndpointName = EndpointName;
  Endpoint.providesTags = providesTags;
  Endpoint.invalidatesTags = invalidatesTags;
  Endpoint.RequestBuilder = RequestBuilder;
  return injectEndpoints(api, Endpoint)(ArgValue);
}

/**
 * Builds a GET endpoint thunk.
 * User Story: As read-only API calls, I need a GET factory so fetch endpoints
 * can reuse the same store integration path.
 */
template <typename Result>
inline ThunkAction<Result, FRuntimeState>
MakeGet(const FString &EndpointName, const FString &Url,
        const TArray<FApiEndpointTag> &Tags = TArray<FApiEndpointTag>()) {
  return MakeEndpoint<rtk::FEmptyPayload, Result>(
      EndpointName, rtk::FEmptyPayload{},
      [Url](const rtk::FEmptyPayload &) {
        return ExecuteApiBaseQuery<Result>(TEXT("GET"), Url);
      },
      Tags);
}

/**
 * Builds a POST endpoint thunk using struct-to-JSON encoding.
 * User Story: As write-oriented API calls, I need a POST factory so typed
 * request payloads can be serialized and dispatched uniformly.
 */
template <typename Request, typename Result>
inline ThunkAction<Result, FRuntimeState> MakePost(
    const FString &EndpointName, const FString &Url,
    const Request &RequestValue,
    const TArray<FApiEndpointTag> &Invalidates = TArray<FApiEndpointTag>()) {
  return MakeEndpoint<Request, Result>(
      EndpointName, RequestValue,
      [Url](const Request &Arg) {
        return ExecuteApiBaseQuery<Result>(TEXT("POST"), Url, ToJson(Arg));
      },
      TArray<FApiEndpointTag>(), Invalidates);
}

/**
 * Builds a DELETE endpoint thunk.
 * User Story: As destructive API calls, I need a DELETE factory so endpoint
 * removal flows share the same dispatch and invalidation behavior.
 */
template <typename Result>
inline ThunkAction<Result, FRuntimeState> MakeDelete(
    const FString &EndpointName, const FString &Url,
    const TArray<FApiEndpointTag> &Invalidates = TArray<FApiEndpointTag>()) {
  return MakeEndpoint<rtk::FEmptyPayload, Result>(
      EndpointName, rtk::FEmptyPayload{},
      [Url](const rtk::FEmptyPayload &) {
        return ExecuteApiBaseQuery<Result>(TEXT("DELETE"), Url);
      },
      TArray<FApiEndpointTag>(), Invalidates);
}

/**
 * Decodes a raw-string RTK Query return value into a typed RTK Query return value.
 * User Story: As endpoint codecs, I need shared decode handling so response
 * parsing and transport failures are mapped consistently.
 */
template <typename Result>
inline func::AsyncResult<rtk::QueryReturnValue<Result>>
DecodeQueryReturnValue(
    func::AsyncResult<rtk::QueryReturnValue<FString>> RawResult,
    std::function<bool(const FString &, Result &)> Decoder) {
  return func::AsyncResult<rtk::QueryReturnValue<Result>>::create(
      [RawResult,
       Decoder](std::function<void(rtk::QueryReturnValue<Result>)> Resolve,
                std::function<void(std::string)> Reject) {
        RawResult
            .then([Decoder, Resolve](
                      const rtk::QueryReturnValue<FString> &QueryResult) {
              QueryResult.error.hasValue
                  ? (Resolve(rtk::QueryReturnValue<Result>::failure(
                         QueryResult.error.value, QueryResult.meta)),
                     void())
                  : [&]() {
                      Result Parsed;
                      Decoder(QueryResult.data.value, Parsed)
                          ? (Resolve(rtk::QueryReturnValue<Result>::success(
                                 Parsed, QueryResult.meta)),
                             void())
                          : (Resolve(rtk::QueryReturnValue<Result>::failure(
                                 rtk::FetchBaseQueryError::parsingError(
                                     QueryResult.meta.hasValue &&
                                             QueryResult.meta.value.response
                                                 .hasValue
                                         ? QueryResult.meta.value.response.value
                                               .status
                                         : 0,
                                     QueryResult.data.value,
                                     TEXT("JSON deserialization failed")),
                                 QueryResult.meta)),
                             void());
                    }();
            })
            .catch_([Resolve](std::string Error) {
              Resolve(rtk::QueryReturnValue<Result>::failure(
                  rtk::FetchBaseQueryError::fetchError(
                      FString(UTF8_TO_TCHAR(Error.c_str())))));
            })
            .execute();
      });
}

/**
 * Builds a POST endpoint thunk that uses custom encode and decode functions.
 * User Story: As custom API codecs, I need one helper for hand-rolled payloads
 * so endpoints can bypass generic struct serialization when needed.
 */
template <typename Request, typename Result>
inline ThunkAction<Result, FRuntimeState> MakePostWithCodec(
    const FString &EndpointName, const FString &Url,
    const Request &RequestValue,
    std::function<FString(const Request &)> Encoder,
    std::function<bool(const FString &, Result &)> Decoder,
    const TArray<FApiEndpointTag> &Invalidates = TArray<FApiEndpointTag>()) {
  return MakeEndpoint<Request, Result>(
      EndpointName, RequestValue,
      [Url, Encoder, Decoder](const Request &Arg) {
        return DecodeQueryReturnValue<Result>(
            ExecuteApiBaseQuery<FString>(TEXT("POST"), Url, Encoder(Arg)),
            Decoder);
      },
      TArray<FApiEndpointTag>(), Invalidates);
}

/**
 * Builds a GET endpoint thunk that decodes a custom response shape.
 * User Story: As custom API codecs, I need GET helpers with pluggable decoders
 * so complex JSON responses can still use the common endpoint pipeline.
 */
template <typename Result>
inline ThunkAction<Result, FRuntimeState> MakeGetWithCodec(
    const FString &EndpointName, const FString &Url,
    std::function<bool(const FString &, Result &)> Decoder,
    const TArray<FApiEndpointTag> &Tags = TArray<FApiEndpointTag>()) {
  return MakeEndpoint<rtk::FEmptyPayload, Result>(
      EndpointName, rtk::FEmptyPayload{},
      [Url, Decoder](const rtk::FEmptyPayload &) {
        return DecodeQueryReturnValue<Result>(
            ExecuteApiBaseQuery<FString>(TEXT("GET"), Url), Decoder);
      },
      Tags);
}

/**
 * Builds a POST endpoint thunk from raw JSON text plus a custom decoder.
 * User Story: As raw-payload endpoints, I need a helper that accepts JSON text
 * directly so already-shaped payloads can be posted without double encoding.
 */
template <typename Result>
inline ThunkAction<Result, FRuntimeState>
MakePostRawWithCodec(const FString &EndpointName, const FString &Url,
                     const FString &PayloadJson,
                     std::function<bool(const FString &, Result &)> Decoder) {
  return MakeEndpoint<FString, Result>(
      EndpointName, PayloadJson, [Url, Decoder](const FString &Arg) {
        return DecodeQueryReturnValue<Result>(
            ExecuteApiBaseQuery<FString>(TEXT("POST"), Url, Arg), Decoder);
      });
}

} // namespace APISlice::Detail
