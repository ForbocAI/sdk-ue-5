#pragma once

#include "Core/RTK/Query/Serialization/Serialization.hpp"

namespace rtk {
namespace detail {
inline bool isAbsoluteFetchUrl(const FString &Url) {
  return Url.StartsWith(TEXT("http://")) || Url.StartsWith(TEXT("https://"));
}

inline FString trimTrailingSlash(const FString &Value) {
  FString Copy = Value;
  Copy.RemoveFromEnd(TEXT("/"));
  return Copy;
}

inline FString trimLeadingSlash(const FString &Value) {
  FString Copy = Value;
  Copy.RemoveFromStart(TEXT("/"));
  return Copy;
}

inline FString buildFetchUrl(const FString &BaseUrl, const FString &Url) {
  return BaseUrl.IsEmpty() || isAbsoluteFetchUrl(Url)
             ? Url
             : trimTrailingSlash(BaseUrl) + TEXT("/") + trimLeadingSlash(Url);
}

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

inline bool isWriteMethod(const FString &Method) {
  return Method == TEXT("POST") || Method == TEXT("PUT") ||
         Method == TEXT("PATCH");
}

inline bool applyBody(IHttpRequest &Request, const FetchArgs &Args) {
  return isWriteMethod(Args.method)
             ? (!Args.headers.Contains(TEXT("Content-Type"))
                    ? (Request.SetHeader(TEXT("Content-Type"),
                                         TEXT("application/json")),
                       void())
                    : void(),
                Request.SetContentAsString(Args.body), true)
             : true;
}

inline bool applyTimeout(IHttpRequest &Request, const FetchArgs &Args,
                         const FetchBaseQueryArgs &Options) {
  const int32 Timeout = Args.timeout > 0 ? Args.timeout : Options.timeout;
  return Timeout > 0
             ? (Request.SetTimeout(static_cast<float>(Timeout) / 1000.0f),
                true)
             : true;
}

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

inline bool applyHeaders(IHttpRequest &Request,
                         const TMap<FString, FString> &Headers) {
  TArray<FString> Keys;
  return (Headers.GetKeys(Keys),
          applyHeaderKeysRecursive(Request, Headers, Keys, 0), true);
}

inline bool hasHeader(const TMap<FString, FString> &Headers,
                      const FString &Name) {
  return Headers.Contains(Name) || Headers.Contains(Name.ToLower());
}

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

inline TMap<FString, FString>
mergeHeaders(const TMap<FString, FString> &BaseHeaders,
             const TMap<FString, FString> &ArgHeaders) {
  TMap<FString, FString> Merged = BaseHeaders;
  TArray<FString> Keys;
  return (ArgHeaders.GetKeys(Keys),
          mergeHeaderKeysRecursive(ArgHeaders, Keys, 0, Merged), Merged);
}

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

inline TMap<FString, FString> responseHeaders(FHttpResponsePtr Res) {
  TMap<FString, FString> Headers;
  return Res.IsValid()
             ? (addHeaderLineRecursive(Res->GetAllHeaders(), 0, Headers),
                Headers)
             : Headers;
}

} // namespace detail
} // namespace rtk
