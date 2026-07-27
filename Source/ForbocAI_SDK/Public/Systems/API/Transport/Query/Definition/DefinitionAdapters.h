#pragma once

#include "Systems/API/Transport/Query/Execution/ExecutionThunks.h"

namespace APISlice::Detail {

/**
 * Builds an injected RTK-style endpoint thunk from request metadata.
 * User Story: As API endpoint wiring, I need one endpoint factory so store integration, tags, and request builders stay consistent across endpoints.
 * @fn template <typename Arg, typename Result> inline ThunkAction<Result, FRuntimeState> MakeEndpoint( const FString &EndpointName, const Arg &ArgValue, std::function<func::AsyncResult<rtk::QueryReturnValue<Result>>( const Arg &, const rtk::ApiContext<FRuntimeState> &)> RequestBuilder, const TArray<FApiEndpointTag> &providesTags = TArray<FApiEndpointTag>(), const TArray<FApiEndpointTag> &invalidatesTags = TArray<FApiEndpointTag>(), rtk::DefinitionType Type = rtk::DefinitionType::query)
 */
template <typename Arg, typename Result>
inline ThunkAction<Result, FRuntimeState> MakeEndpoint(
    const FString &EndpointName, const Arg &ArgValue,
    std::function<func::AsyncResult<rtk::QueryReturnValue<Result>>(
        const Arg &, const rtk::ApiContext<FRuntimeState> &)> RequestBuilder,
    const TArray<FApiEndpointTag> &providesTags = TArray<FApiEndpointTag>(),
    const TArray<FApiEndpointTag> &invalidatesTags =
        TArray<FApiEndpointTag>(),
    rtk::DefinitionType Type = rtk::DefinitionType::query) {
  ApiEndpoint<Arg, Result, FRuntimeState> Endpoint;
  Endpoint.EndpointName = EndpointName;
  Endpoint.Type = Type;
  Endpoint.providesTags = providesTags;
  Endpoint.invalidatesTags = invalidatesTags;
  Endpoint.RequestBuilder = RequestBuilder;
  rtk::Api<FRuntimeState> &InjectedApi =
      rtk::injectEndpoints(api, Endpoint);
  return rtk::initiate(InjectedApi, Endpoint)(ArgValue);
}

/**
 * User Story: As read-only API calls, I need a GET factory so fetch endpoints can reuse the same store integration path.
 * @fn template <typename Result> inline ThunkAction<Result, FRuntimeState> MakeGet(const FString &EndpointName, const FString &Path, const TArray<FApiEndpointTag> &Tags = TArray<FApiEndpointTag>(), int32 Timeout = Transport::transportQueryData().Timeouts.Disabled)
 */
template <typename Result>
inline ThunkAction<Result, FRuntimeState>
MakeGet(const FString &EndpointName, const FString &Path,
        const TArray<FApiEndpointTag> &Tags = TArray<FApiEndpointTag>(),
        int32 Timeout = Transport::transportQueryData().Timeouts.Disabled) {
  const Transport::FTransportQueryData &Data =
      Transport::transportQueryData();
  return MakeEndpoint<rtk::FEmptyPayload, Result>(
      EndpointName, rtk::FEmptyPayload{},
      [Path, Data, Timeout](const rtk::FEmptyPayload &,
                           const rtk::ApiContext<FRuntimeState> &Context) {
        return ExecuteApiBaseQuery<Result>(Data.Methods.Get, Path, Context,
                                           FString(), Timeout);
      },
      Tags);
}

/**
 * User Story: As write-oriented API calls, I need a POST factory so typed request payloads can be serialized and dispatched uniformly.
 * @fn template <typename Request, typename Result> inline ThunkAction<Result, FRuntimeState> MakePost( const FString &EndpointName, const FString &Path, const Request &RequestValue, const TArray<FApiEndpointTag> &Invalidates = TArray<FApiEndpointTag>())
 */
template <typename Request, typename Result>
inline ThunkAction<Result, FRuntimeState> MakePost(
    const FString &EndpointName, const FString &Path,
    const Request &RequestValue,
    const TArray<FApiEndpointTag> &Invalidates = TArray<FApiEndpointTag>()) {
  const Transport::FTransportQueryData &Data =
      Transport::transportQueryData();
  return MakeEndpoint<Request, Result>(
      EndpointName, RequestValue,
      [Path, Data](const Request &Arg,
                  const rtk::ApiContext<FRuntimeState> &Context) {
        return ExecuteApiBaseQuery<Result>(Data.Methods.Post, Path, Context,
                                           ToJson(Arg));
      },
      TArray<FApiEndpointTag>(), Invalidates, rtk::DefinitionType::mutation);
}

/**
 * User Story: As destructive API calls, I need a DELETE factory so endpoint removal flows share the same dispatch and invalidation behavior.
 * @fn template <typename Result> inline ThunkAction<Result, FRuntimeState> MakeDelete( const FString &EndpointName, const FString &Path, const TArray<FApiEndpointTag> &Invalidates = TArray<FApiEndpointTag>())
 */
template <typename Result>
inline ThunkAction<Result, FRuntimeState> MakeDelete(
    const FString &EndpointName, const FString &Path,
    const TArray<FApiEndpointTag> &Invalidates = TArray<FApiEndpointTag>()) {
  const Transport::FTransportQueryData &Data =
      Transport::transportQueryData();
  return MakeEndpoint<rtk::FEmptyPayload, Result>(
      EndpointName, rtk::FEmptyPayload{},
      [Path, Data](const rtk::FEmptyPayload &,
                  const rtk::ApiContext<FRuntimeState> &Context) {
        return ExecuteApiBaseQuery<Result>(Data.Methods.Delete, Path, Context);
      },
      TArray<FApiEndpointTag>(), Invalidates, rtk::DefinitionType::mutation);
}

} // namespace APISlice::Detail
