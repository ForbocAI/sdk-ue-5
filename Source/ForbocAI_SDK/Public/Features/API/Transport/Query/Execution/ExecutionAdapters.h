#pragma once

#include "Features/API/Transport/Codec/TransportCodecAdapters.h"
#include "Features/API/Transport/Configuration/ConfigurationAdapters.h"

namespace APISlice::Detail {

/** User Story: As a api transport query execution consumer, I need to invoke api base query options through a stable signature so the api transport query execution workflow remains explicit and composable. @fn inline rtk::FetchBaseQueryArgs ApiBaseQueryOptions() */
inline rtk::FetchBaseQueryArgs ApiBaseQueryOptions() {
  rtk::FetchBaseQueryArgs Options;
  const FString ApiKey = SDKConfig::GetApiKey();
  const Transport::FTransportQueryData &Data =
      Transport::transportQueryData();
  !ApiKey.IsEmpty()
      ? (Options.headers.Add(Data.Headers.Authorization,
                             Data.Headers.BearerPrefix + ApiKey),
         void())
      : void();
  return Options;
}

/**
 * User Story: As a api transport query execution consumer, I need to invoke api fetch args through a stable signature so the api transport query execution workflow remains explicit and composable.
 * @fn inline rtk::FetchArgs ApiFetchArgs( const FString &Method, const FString &Url, const FString &Body = FString(), int32 Timeout = Transport::transportQueryData().Timeouts.Disabled)
 */
inline rtk::FetchArgs ApiFetchArgs(
    const FString &Method, const FString &Url,
    const FString &Body = FString(),
    int32 Timeout = Transport::transportQueryData().Timeouts.Disabled) {
  rtk::FetchArgs Args;
  Args.method = Method;
  Args.url = Url;
  Args.body = Body;
  Args.timeout = Timeout;
  return Args;
}

/**
 * User Story: As a api transport query execution consumer, I need to invoke execute api base query through a stable signature so the api transport query execution workflow remains explicit and composable.
 * @fn template <typename Result> inline func::AsyncResult<rtk::QueryReturnValue<Result>> ExecuteApiBaseQuery( const FString &Method, const FString &Url, const FString &Body = FString(), int32 Timeout = Transport::transportQueryData().Timeouts.Disabled)
 */
template <typename Result>
inline func::AsyncResult<rtk::QueryReturnValue<Result>> ExecuteApiBaseQuery(
    const FString &Method, const FString &Url,
    const FString &Body = FString(),
    int32 Timeout = Transport::transportQueryData().Timeouts.Disabled) {
  return rtk::fetchBaseQuery<Result>(ApiBaseQueryOptions())(
      ApiFetchArgs(Method, Url, Body, Timeout), rtk::BaseQueryApi(),
      rtk::FEmptyPayload{});
}

/**
 * Decodes a raw-string RTK Query return value into a typed value.
 * User Story: As endpoint codecs, I need shared decode handling so response parsing and transport failures are mapped consistently.
 * @fn template <typename Result> inline func::AsyncResult<rtk::QueryReturnValue<Result>> DecodeQueryReturnValue( func::AsyncResult<rtk::QueryReturnValue<FString>> RawResult, std::function<bool(const FString &, Result &)> Decoder)
 */
template <typename Result>
inline func::AsyncResult<rtk::QueryReturnValue<Result>>
DecodeQueryReturnValue(
    func::AsyncResult<rtk::QueryReturnValue<FString>> RawResult,
    std::function<bool(const FString &, Result &)> Decoder) {
  const Transport::FTransportQueryData &Data =
      Transport::transportQueryData();
  return func::AsyncResult<rtk::QueryReturnValue<Result>>::create(
      [RawResult, Decoder, Data](
          std::function<void(rtk::QueryReturnValue<Result>)> Resolve,
          std::function<void(std::string)> Reject) {
        RawResult
            .then([Decoder, Resolve, Data](
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
                                         : Data.Timeouts.Disabled,
                                     QueryResult.data.value,
                                     Data.Errors.JsonDeserializationFailed),
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

} // namespace APISlice::Detail
