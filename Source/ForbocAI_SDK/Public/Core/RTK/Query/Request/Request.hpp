#pragma once

#include "Core/RTK/Query/Serialization/Serialization.hpp"

namespace rtk {
namespace detail {
/** User Story: As a rtk query request consumer, I need to invoke is absolute fetch url through a stable signature so the rtk query request workflow remains explicit and composable. @fn inline bool isAbsoluteFetchUrl(const FString &Url) */
inline bool isAbsoluteFetchUrl(const FString &Url) {
  return Url.StartsWith(TEXT("http://")) || Url.StartsWith(TEXT("https://"));
}

/** User Story: As a rtk query request consumer, I need to invoke trim trailing slash through a stable signature so the rtk query request workflow remains explicit and composable. @fn inline FString trimTrailingSlash(const FString &Value) */
inline FString trimTrailingSlash(const FString &Value) {
  FString Copy = Value;
  Copy.RemoveFromEnd(TEXT("/"));
  return Copy;
}

/** User Story: As a rtk query request consumer, I need to invoke trim leading slash through a stable signature so the rtk query request workflow remains explicit and composable. @fn inline FString trimLeadingSlash(const FString &Value) */
inline FString trimLeadingSlash(const FString &Value) {
  FString Copy = Value;
  Copy.RemoveFromStart(TEXT("/"));
  return Copy;
}

/** User Story: As a rtk query request consumer, I need to invoke build fetch url through a stable signature so the rtk query request workflow remains explicit and composable. @fn inline FString buildFetchUrl(const FString &BaseUrl, const FString &Url) */
inline FString buildFetchUrl(const FString &BaseUrl, const FString &Url) {
  return BaseUrl.IsEmpty() || isAbsoluteFetchUrl(Url)
             ? Url
             : trimTrailingSlash(BaseUrl) + TEXT("/") + trimLeadingSlash(Url);
}

/** User Story: As a rtk query request consumer, I need to invoke append param keys recursive through a stable signature so the rtk query request workflow remains explicit and composable. @fn inline void appendParamKeysRecursive(const TMap<FString, FString> &Params, const TArray<FString> &Keys, int32 Index, FString &Query) */
inline void appendParamKeysRecursive(const TMap<FString, FString> &Params,
                                     const TArray<FString> &Keys, int32 Index,
                                     FString &Query) {
  Index >= Keys.Num()
      ? void()
      : (Params.Find(Keys[Index])
             ? (Query += (Query.IsEmpty() ? TEXT("?") : TEXT("&")) +
                        FGenericPlatformHttp::UrlEncode(Keys[Index]) +
                        TEXT("=") +
                        FGenericPlatformHttp::UrlEncode(*Params.Find(Keys[Index])),
                void())
             : void(),
         appendParamKeysRecursive(Params, Keys, Index + 1, Query));
}

/** User Story: As a rtk query request consumer, I need to invoke append fetch params through a stable signature so the rtk query request workflow remains explicit and composable. @fn inline FString appendFetchParams(const FString &Url, const TMap<FString, FString> &Params) */
inline FString appendFetchParams(const FString &Url,
                                 const TMap<FString, FString> &Params) {
  TArray<FString> Keys;
  FString Query;
  return Params.Num() == 0
             ? Url
             : (Params.GetKeys(Keys),
                appendParamKeysRecursive(Params, Keys, 0, Query),
                Url + Query);
}

/** User Story: As a rtk query request consumer, I need to invoke is write method through a stable signature so the rtk query request workflow remains explicit and composable. @fn inline bool isWriteMethod(const FString &Method) */
inline bool isWriteMethod(const FString &Method) {
  return Method == TEXT("POST") || Method == TEXT("PUT") ||
         Method == TEXT("PATCH");
}

/** User Story: As an RTK Query request, I need case-normalized header detection so defaults never overwrite caller policy. @fn inline bool hasHeader(const TMap<FString, FString> &Headers, const FString &Name) */
inline bool hasHeader(const TMap<FString, FString> &Headers,
                      const FString &Name) {
  return Headers.Contains(Name) || Headers.Contains(Name.ToLower());
}

/** User Story: As an RTK Query text mutation, I need text encoded through the platform HTTP request while retaining the official body property. @fn inline bool applyFetchTextBody(IHttpRequest &Request, const TMap<FString, FString> &Headers, const FString &Body) */
inline bool applyFetchTextBody(IHttpRequest &Request,
                               const TMap<FString, FString> &Headers,
                               const FString &Body) {
  return (!hasHeader(Headers, TEXT("Content-Type"))
              ? (Request.SetHeader(TEXT("Content-Type"),
                                   TEXT("application/json")),
                 void())
              : void(),
          Request.SetContentAsString(Body), true);
}

/** User Story: As an RTK Query binary mutation, I need bytes passed without text conversion so signatures and digests remain exact. @fn inline bool applyFetchBinaryBody(IHttpRequest &Request, const TArray<uint8> &Body) */
inline bool applyFetchBinaryBody(IHttpRequest &Request,
                                 const TArray<uint8> &Body) {
  Request.SetContent(Body);
  return true;
}

/** User Story: As an RTK Query request, I need the canonical body union interpreted once at the HTTP boundary. @fn inline bool applyBody(IHttpRequest &Request, const FetchArgs &Args) */
inline bool applyBody(IHttpRequest &Request, const FetchArgs &Args) {
  return !isWriteMethod(Args.method)
             ? true
             : func::match(
                   Args.body.bytes,
                   [&Request](const TArray<uint8> &Body) {
                     return applyFetchBinaryBody(Request, Body);
                   },
                   [&Request, &Args]() {
                     return func::match(
                         Args.body.text,
                         [&Request, &Args](const FString &Body) {
                           return applyFetchTextBody(Request, Args.headers,
                                                     Body);
                         },
                         []() { return true; });
                   });
}

/** User Story: As a rtk query request consumer, I need to invoke apply timeout through a stable signature so the rtk query request workflow remains explicit and composable. @fn inline bool applyTimeout(IHttpRequest &Request, const FetchArgs &Args, const FetchBaseQueryArgs &Options) */
inline bool applyTimeout(IHttpRequest &Request, const FetchArgs &Args,
                         const FetchBaseQueryArgs &Options) {
  const int32 Timeout = Args.timeout > 0 ? Args.timeout : Options.timeout;
  const float TimeoutSeconds = static_cast<float>(Timeout) / 1000.0f;
  return Timeout > 0
             ? (Request.SetTimeout(TimeoutSeconds),
                Request.SetActivityTimeout(TimeoutSeconds), true)
             : true;
}

/** User Story: As a rtk query request consumer, I need to invoke apply header keys recursive through a stable signature so the rtk query request workflow remains explicit and composable. @fn inline void applyHeaderKeysRecursive(IHttpRequest &Request, const TMap<FString, FString> &Headers, const TArray<FString> &Keys, int32 Index) */
inline void applyHeaderKeysRecursive(IHttpRequest &Request,
                                     const TMap<FString, FString> &Headers,
                                     const TArray<FString> &Keys,
                                     int32 Index) {
  Index >= Keys.Num()
      ? void()
      : (Headers.Find(Keys[Index])
             ? (Request.SetHeader(Keys[Index], *Headers.Find(Keys[Index])),
                void())
             : void(),
         applyHeaderKeysRecursive(Request, Headers, Keys, Index + 1));
}

/** User Story: As a rtk query request consumer, I need to invoke apply headers through a stable signature so the rtk query request workflow remains explicit and composable. @fn inline bool applyHeaders(IHttpRequest &Request, const TMap<FString, FString> &Headers) */
inline bool applyHeaders(IHttpRequest &Request,
                         const TMap<FString, FString> &Headers) {
  TArray<FString> Keys;
  return (Headers.GetKeys(Keys),
          applyHeaderKeysRecursive(Request, Headers, Keys, 0), true);
}

/** User Story: As a rtk query request consumer, I need to invoke apply accept header through a stable signature so the rtk query request workflow remains explicit and composable. @fn inline bool applyAcceptHeader(IHttpRequest &Request, const TMap<FString, FString> &Headers, const ResponseHandler &Handler) */
inline bool applyAcceptHeader(IHttpRequest &Request,
                              const TMap<FString, FString> &Headers,
                              const ResponseHandler &Handler) {
  return hasHeader(Headers, TEXT("Accept"))
             ? true
             : Handler == TEXT("json")
                   ? (Request.SetHeader(TEXT("Accept"),
                                        TEXT("application/json")),
                      true)
                   : Handler == TEXT("text")
                         ? (Request.SetHeader(
                                TEXT("Accept"),
                                TEXT("text/plain, text/html, */*")),
                            true)
                         : true;
}

/** User Story: As a rtk query request consumer, I need to invoke merge header keys recursive through a stable signature so the rtk query request workflow remains explicit and composable. @fn inline void mergeHeaderKeysRecursive(const TMap<FString, FString> &Source, const TArray<FString> &Keys, int32 Index, TMap<FString, FString> &Target) */
inline void mergeHeaderKeysRecursive(const TMap<FString, FString> &Source,
                                     const TArray<FString> &Keys, int32 Index,
                                     TMap<FString, FString> &Target) {
  Index >= Keys.Num()
      ? void()
      : (Source.Find(Keys[Index])
             ? (Target.Add(Keys[Index], *Source.Find(Keys[Index])), void())
             : void(),
         mergeHeaderKeysRecursive(Source, Keys, Index + 1, Target));
}

/** User Story: As a rtk query request consumer, I need to invoke merge headers through a stable signature so the rtk query request workflow remains explicit and composable. @fn inline TMap<FString, FString> mergeHeaders(const TMap<FString, FString> &BaseHeaders, const TMap<FString, FString> &ArgHeaders) */
inline TMap<FString, FString>
mergeHeaders(const TMap<FString, FString> &BaseHeaders,
             const TMap<FString, FString> &ArgHeaders) {
  TMap<FString, FString> Merged = BaseHeaders;
  TArray<FString> Keys;
  return (ArgHeaders.GetKeys(Keys),
          mergeHeaderKeysRecursive(ArgHeaders, Keys, 0, Merged), Merged);
}

/** User Story: As a rtk query request consumer, I need to invoke add header line recursive through a stable signature so the rtk query request workflow remains explicit and composable. @fn inline void addHeaderLineRecursive(const TArray<FString> &HeaderLines, int32 Index, TMap<FString, FString> &OutHeaders) */
inline void addHeaderLineRecursive(const TArray<FString> &HeaderLines,
                                   int32 Index,
                                   TMap<FString, FString> &OutHeaders) {
  Index >= HeaderLines.Num()
      ? void()
      : [&]() {
          FString Key;
          FString Value;
          HeaderLines[Index].Split(TEXT(":"), &Key, &Value)
              ? (Key.TrimStartAndEndInline(), Value.TrimStartAndEndInline(),
                 OutHeaders.Add(Key, Value),
                 OutHeaders.Add(Key.ToLower(), Value), void())
              : void();
          addHeaderLineRecursive(HeaderLines, Index + 1, OutHeaders);
        }();
}

/** User Story: As a rtk query request consumer, I need to invoke response headers through a stable signature so the rtk query request workflow remains explicit and composable. @fn inline TMap<FString, FString> responseHeaders(FHttpResponsePtr Res) */
inline TMap<FString, FString> responseHeaders(FHttpResponsePtr Res) {
  TMap<FString, FString> Headers;
  return Res.IsValid()
             ? (addHeaderLineRecursive(Res->GetAllHeaders(), 0, Headers),
                Headers)
             : Headers;
}

} // namespace detail
} // namespace rtk
