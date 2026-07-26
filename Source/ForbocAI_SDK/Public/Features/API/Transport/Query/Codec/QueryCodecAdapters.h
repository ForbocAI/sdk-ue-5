#pragma once

#include "Features/API/Transport/Query/Definition/DefinitionAdapters.h"

namespace APISlice::Detail {

/**
 * User Story: As custom API codecs, I need one helper for hand-rolled payloads so endpoints can bypass generic struct serialization when needed.
 * @fn template <typename Request, typename Result> inline ThunkAction<Result, FRuntimeState> MakePostWithCodec( const FString &EndpointName, const FString &Path, const Request &RequestValue, std::function<FString(const Request &)> Encoder, std::function<bool(const FString &, Result &)> Decoder, const TArray<FApiEndpointTag> &Invalidates = TArray<FApiEndpointTag>())
 */
template <typename Request, typename Result>
inline ThunkAction<Result, FRuntimeState> MakePostWithCodec(
    const FString &EndpointName, const FString &Path,
    const Request &RequestValue,
    std::function<FString(const Request &)> Encoder,
    std::function<bool(const FString &, Result &)> Decoder,
    const TArray<FApiEndpointTag> &Invalidates = TArray<FApiEndpointTag>()) {
  const Transport::FTransportQueryData &Data =
      Transport::transportQueryData();
  return MakeEndpoint<Request, Result>(
      EndpointName, RequestValue,
      [Path, Encoder, Decoder, Data](
          const Request &Arg,
          const rtk::ApiContext<FRuntimeState> &Context) {
        return DecodeQueryReturnValue<Result>(
            ExecuteApiBaseQuery<FString>(Data.Methods.Post, Path, Context,
                                         Encoder(Arg)),
            Decoder);
      },
      TArray<FApiEndpointTag>(), Invalidates, rtk::DefinitionType::mutation);
}

/**
 * User Story: As read-oriented validation calls, I need custom wire codecs combined with RTK query caching and provides-tags behavior.
 * @fn template <typename Request, typename Result> inline ThunkAction<Result, FRuntimeState> MakePostQueryWithCodec( const FString &EndpointName, const FString &Path, const Request &RequestValue, std::function<FString(const Request &)> Encoder, std::function<bool(const FString &, Result &)> Decoder, const TArray<FApiEndpointTag> &Tags = TArray<FApiEndpointTag>())
 */
template <typename Request, typename Result>
inline ThunkAction<Result, FRuntimeState> MakePostQueryWithCodec(
    const FString &EndpointName, const FString &Path,
    const Request &RequestValue,
    std::function<FString(const Request &)> Encoder,
    std::function<bool(const FString &, Result &)> Decoder,
    const TArray<FApiEndpointTag> &Tags = TArray<FApiEndpointTag>()) {
  const Transport::FTransportQueryData &Data =
      Transport::transportQueryData();
  return MakeEndpoint<Request, Result>(
      EndpointName, RequestValue,
      [Path, Encoder, Decoder, Data](
          const Request &Arg,
          const rtk::ApiContext<FRuntimeState> &Context) {
        return DecodeQueryReturnValue<Result>(
            ExecuteApiBaseQuery<FString>(Data.Methods.Post, Path, Context,
                                         Encoder(Arg)),
            Decoder);
      },
      Tags, TArray<FApiEndpointTag>(), rtk::DefinitionType::query);
}

/**
 * User Story: As custom API codecs, I need GET helpers with pluggable decoders so complex JSON responses can still use the common endpoint pipeline.
 * @fn template <typename Result> inline ThunkAction<Result, FRuntimeState> MakeGetWithCodec( const FString &EndpointName, const FString &Path, std::function<bool(const FString &, Result &)> Decoder, const TArray<FApiEndpointTag> &Tags = TArray<FApiEndpointTag>())
 */
template <typename Result>
inline ThunkAction<Result, FRuntimeState> MakeGetWithCodec(
    const FString &EndpointName, const FString &Path,
    std::function<bool(const FString &, Result &)> Decoder,
    const TArray<FApiEndpointTag> &Tags = TArray<FApiEndpointTag>()) {
  const Transport::FTransportQueryData &Data =
      Transport::transportQueryData();
  return MakeEndpoint<rtk::FEmptyPayload, Result>(
      EndpointName, rtk::FEmptyPayload{},
      [Path, Decoder, Data](
          const rtk::FEmptyPayload &,
          const rtk::ApiContext<FRuntimeState> &Context) {
        return DecodeQueryReturnValue<Result>(
            ExecuteApiBaseQuery<FString>(Data.Methods.Get, Path, Context),
            Decoder);
      },
      Tags);
}

/**
 * User Story: As body-free API mutations, I need custom response decoding without inventing a request payload.
 * @fn template <typename Result> inline ThunkAction<Result, FRuntimeState> MakePostNoBodyWithCodec( const FString &EndpointName, const FString &Path, std::function<bool(const FString &, Result &)> Decoder, const TArray<FApiEndpointTag> &Invalidates = TArray<FApiEndpointTag>())
 */
template <typename Result>
inline ThunkAction<Result, FRuntimeState> MakePostNoBodyWithCodec(
    const FString &EndpointName, const FString &Path,
    std::function<bool(const FString &, Result &)> Decoder,
    const TArray<FApiEndpointTag> &Invalidates = TArray<FApiEndpointTag>()) {
  const Transport::FTransportQueryData &Data =
      Transport::transportQueryData();
  return MakeEndpoint<rtk::FEmptyPayload, Result>(
      EndpointName, rtk::FEmptyPayload{},
      [Path, Decoder, Data](
          const rtk::FEmptyPayload &,
          const rtk::ApiContext<FRuntimeState> &Context) {
        return DecodeQueryReturnValue<Result>(
            ExecuteApiBaseQuery<FString>(Data.Methods.Post, Path, Context),
            Decoder);
      },
      TArray<FApiEndpointTag>(), Invalidates,
      rtk::DefinitionType::mutation);
}

/**
 * User Story: As raw-payload endpoints, I need a helper that accepts JSON text directly so already-shaped payloads can be posted without double encoding.
 * @fn template <typename Result> inline ThunkAction<Result, FRuntimeState> MakePostRawWithCodec( const FString &EndpointName, const FString &Path, const FString &PayloadJson, std::function<bool(const FString &, Result &)> Decoder, const TArray<FApiEndpointTag> &Invalidates = TArray<FApiEndpointTag>())
 */
template <typename Result>
inline ThunkAction<Result, FRuntimeState> MakePostRawWithCodec(
    const FString &EndpointName, const FString &Path,
    const FString &PayloadJson,
    std::function<bool(const FString &, Result &)> Decoder,
    const TArray<FApiEndpointTag> &Invalidates = TArray<FApiEndpointTag>()) {
  const Transport::FTransportQueryData &Data =
      Transport::transportQueryData();
  return MakeEndpoint<FString, Result>(
      EndpointName, PayloadJson,
      [Path, Decoder, Data](
          const FString &Arg,
          const rtk::ApiContext<FRuntimeState> &Context) {
        return DecodeQueryReturnValue<Result>(
            ExecuteApiBaseQuery<FString>(Data.Methods.Post, Path, Context, Arg),
            Decoder);
      },
      TArray<FApiEndpointTag>(), Invalidates,
      rtk::DefinitionType::mutation);
}

} // namespace APISlice::Detail
