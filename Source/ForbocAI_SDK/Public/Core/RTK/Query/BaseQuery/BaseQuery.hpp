#pragma once

#include "Core/RTK/Query/StructuralSharing/StructuralSharing.hpp"

namespace rtk {
template <typename Result>
BaseQueryFn<FetchArgs, Result, FetchBaseQueryError, FEmptyPayload,
            FetchBaseQueryMeta>
fetchBaseQuery(const FetchBaseQueryArgs &Options = FetchBaseQueryArgs()) {
  return [Options](const FetchArgs &Args, const BaseQueryApi &Api,
                   const FEmptyPayload &ExtraOptions)
             -> func::AsyncResult<QueryReturnValue<Result>> {
    (void)Api;
    (void)ExtraOptions;
    return func::createAsyncResult<QueryReturnValue<Result>>(
        [Options, Args](
            std::function<void(QueryReturnValue<Result>)> Resolve,
            std::function<void(std::string)> Reject) {
          const FString ResolvedUrl = detail::buildFetchUrl(Options.baseUrl,
                                                            Args.url);
          const FString ResolvedUrlWithParams =
              detail::appendFetchParams(ResolvedUrl, Args.params);
          const TMap<FString, FString> Headers =
              detail::mergeHeaders(Options.headers, Args.headers);
          const FetchBaseQueryRequest RequestMeta =
              detail::fetchRequestMeta(Args, ResolvedUrlWithParams, Headers);
          const ResponseHandler Handler =
              !Args.responseHandler.IsEmpty() ? Args.responseHandler
                                              : Options.responseHandler;
          TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request =
              FHttpModule::Get().CreateRequest();
          Request->SetURL(ResolvedUrlWithParams);
          Request->SetVerb(Args.method);
          (void)Reject;
          (void)detail::applyTimeout(*Request, Args, Options);
          (void)detail::applyHeaders(*Request, Headers);
          (void)detail::applyAcceptHeader(*Request, Headers, Handler);
          (void)detail::applyBody(*Request, Args);

          Request->OnProcessRequestComplete().BindLambda(
              [Resolve, RequestMeta](FHttpRequestPtr Req, FHttpResponsePtr Res,
                                     bool bWasSuccessful) {
                (void)Req;
                detail::resolveFetchCompletion<Result>(
                    Resolve, RequestMeta, Res, bWasSuccessful);
              });

          Request->ProcessRequest();
        });
  };
}

namespace detail {

template <typename Args, typename Result, typename Error,
          typename DefinitionExtraOptions, typename Meta>
void retryBaseQueryAttempt(
    const BaseQueryFn<Args, Result, Error, DefinitionExtraOptions, Meta>
        &BaseQuery,
    const Args &ArgsValue, const BaseQueryApi &Api,
    const DefinitionExtraOptions &ExtraOptions, int32 Attempt,
    int32 MaxRetries,
    std::function<void(QueryReturnValue<Result, Error, Meta>)> Resolve,
    std::function<void(std::string)> Reject) {
  BaseQuery(ArgsValue, Api, ExtraOptions)
      .then([BaseQuery, ArgsValue, Api, ExtraOptions, Attempt, MaxRetries,
             Resolve, Reject](QueryReturnValue<Result, Error, Meta> Value) {
        Value.error.hasValue && Attempt < MaxRetries
            ? retryBaseQueryAttempt<Args, Result, Error,
                                    DefinitionExtraOptions, Meta>(
                  BaseQuery, ArgsValue, Api, ExtraOptions, Attempt + 1,
                  MaxRetries, Resolve, Reject)
            : Resolve(Value);
      })
      .catch_([BaseQuery, ArgsValue, Api, ExtraOptions, Attempt, MaxRetries,
               Resolve, Reject](std::string ErrorText) {
        Attempt < MaxRetries
            ? retryBaseQueryAttempt<Args, Result, Error,
                                    DefinitionExtraOptions, Meta>(
                  BaseQuery, ArgsValue, Api, ExtraOptions, Attempt + 1,
                  MaxRetries, Resolve, Reject)
            : Reject(ErrorText);
      })
      .execute();
}

} // namespace detail

template <typename Args = FetchArgs, typename Result = FString,
          typename Error = FetchBaseQueryError,
          typename DefinitionExtraOptions = FEmptyPayload,
          typename Meta = FetchBaseQueryMeta>
BaseQueryFn<Args, Result, Error, DefinitionExtraOptions, Meta>
retry(const BaseQueryFn<Args, Result, Error, DefinitionExtraOptions, Meta>
          &BaseQuery,
      const RetryOptions &Options = RetryOptions()) {
  return [BaseQuery, Options](const Args &ArgsValue, const BaseQueryApi &Api,
                              const DefinitionExtraOptions &ExtraOptions)
             -> func::AsyncResult<QueryReturnValue<Result, Error, Meta>> {
    return func::createAsyncResult<QueryReturnValue<Result, Error, Meta>>(
        [BaseQuery, Options, ArgsValue, Api, ExtraOptions](
            std::function<void(QueryReturnValue<Result, Error, Meta>)> Resolve,
            std::function<void(std::string)> Reject) {
          detail::retryBaseQueryAttempt<Args, Result, Error,
                                        DefinitionExtraOptions, Meta>(
              BaseQuery, ArgsValue, Api, ExtraOptions, 0,
              FMath::Max(0, Options.maxRetries), Resolve, Reject);
        });
  };
}

template <typename Error = FetchBaseQueryError>
BaseQueryFn<FEmptyPayload, FEmptyPayload, Error, FEmptyPayload,
            FetchBaseQueryMeta>
fakeBaseQuery(const Error &ErrorValue = Error()) {
  return [ErrorValue](const FEmptyPayload &, const BaseQueryApi &,
                      const FEmptyPayload &)
             -> func::AsyncResult<
                 QueryReturnValue<FEmptyPayload, Error, FetchBaseQueryMeta>> {
    return func::createAsyncResult<
        QueryReturnValue<FEmptyPayload, Error, FetchBaseQueryMeta>>(
        [ErrorValue](
            std::function<void(QueryReturnValue<FEmptyPayload, Error,
                                                FetchBaseQueryMeta>)> Resolve,
            std::function<void(std::string)> Reject) {
          (void)Reject;
          Resolve(QueryReturnValue<FEmptyPayload, Error,
                                   FetchBaseQueryMeta>::failure(ErrorValue));
        });
  };
}
} // namespace rtk
