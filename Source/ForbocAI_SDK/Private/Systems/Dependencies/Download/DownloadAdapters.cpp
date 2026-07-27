#include "Systems/Dependencies/Download/DownloadAdapters.h"

#include "Core/rtk.hpp"
#include "Systems/Dependencies/Manifest/ManifestAdapters.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace {

struct FBinaryDownloadState {
  FString Destination;
  std::function<void(FString)> Resolve;
  std::function<void(std::string)> Reject;
};

/** User Story: As dependency transport, I need authored errors converted once so async rejection preserves the manifest message. @fn std::string toDownloadError(const FString &Error) */
std::string toDownloadError(const FString &Error) {
  return std::string(TCHAR_TO_UTF8(*Error));
}

/** User Story: As dependency transport, I need redirect classification read from the manifest so HTTP policy is not compiled into the SDK. @fn bool isRedirectCode(int32 Code) */
bool isRedirectCode(int32 Code) {
  return func::contains_value<int32>(
      Dependencies::Manifest::manifest().Http.RedirectStatusCodes, Code);
}

/**
 * User Story: As dependency transport, I need response headers resolved case-insensitively from RTK Query metadata.
 * @fn FString downloadResponseHeaderAdapter( const rtk::FetchBaseQueryResponse &Response, const FString &Name)
 */
FString downloadResponseHeaderAdapter(
    const rtk::FetchBaseQueryResponse &Response, const FString &Name) {
  const FString *Exact = Response.headers.Find(Name);
  const FString *Normalized = Response.headers.Find(Name.ToLower());
  return Exact ? *Exact : Normalized ? *Normalized : FString();
}

/** User Story: As dependency transport, I need redirect traversal bounded and reusable so downloads follow valid release URLs without looping forever. @fn void continueBinaryDownload(const FString &RequestUrl, const TSharedPtr<FBinaryDownloadState> &State, int32 RedirectCount) */
void continueBinaryDownload(const FString &RequestUrl,
                            const TSharedPtr<FBinaryDownloadState> &State,
                            int32 RedirectCount);

/** User Story: As dependency installation, I need destination creation and file persistence checked together so successful downloads always exist on disk. @fn void resolveBinaryDownload(const TSharedPtr<FBinaryDownloadState> &State, const TArray<uint8> &Payload) */
void resolveBinaryDownload(const TSharedPtr<FBinaryDownloadState> &State,
                           const TArray<uint8> &Payload) {
  const Dependencies::Manifest::FHttp &Http =
      Dependencies::Manifest::manifest().Http;
  const FString Directory = FPaths::GetPath(State->Destination);
  IFileManager &Files = IFileManager::Get();
  const bool bDirectoryReady =
      Directory.IsEmpty() || Files.DirectoryExists(*Directory) ||
      Files.MakeDirectory(*Directory, true);
  bDirectoryReady &&
          FFileHelper::SaveArrayToFile(Payload, *State->Destination)
      ? State->Resolve(State->Destination)
      : State->Reject(toDownloadError(Http.SaveFailed));
}

/**
 * User Story: As dependency transport, I need every RTK Query result validated before persistence so redirects and failures cannot be mistaken for binaries.
 * @fn void handleBinaryDownloadResponse( const rtk::QueryReturnValue<TArray<uint8>> &Result, const TSharedPtr<FBinaryDownloadState> &State, int32 RedirectCount)
 */
void handleBinaryDownloadResponse(
    const rtk::QueryReturnValue<TArray<uint8>> &Result,
    const TSharedPtr<FBinaryDownloadState> &State, int32 RedirectCount) {
  const Dependencies::Manifest::FHttp &Http =
      Dependencies::Manifest::manifest().Http;
  const func::Maybe<rtk::FetchBaseQueryResponse> Response =
      Result.meta.hasValue ? Result.meta.value.response
                           : func::nothing<rtk::FetchBaseQueryResponse>();
  const int32 Status = Response.hasValue
                           ? Response.value.status
                           : rtk::FetchBaseQueryResponse().status;
  const FString Location =
      Response.hasValue
          ? downloadResponseHeaderAdapter(Response.value, Http.LocationHeader)
          : FString();
  const bool bRedirect = isRedirectCode(Status) && !Location.IsEmpty();
  const bool bSuccessful =
      Status >= Http.SuccessStatusMinimum &&
      Status < Http.SuccessStatusMaximumExclusive;
  func::is_nothing(Response)
      ? State->Reject(toDownloadError(Http.NetworkFailed))
      : bRedirect
            ? (RedirectCount >= Http.MaxRedirects
                   ? State->Reject(toDownloadError(Http.RedirectLimitFailed))
                   : continueBinaryDownload(
                         Location, State, RedirectCount + Http.RedirectStep))
            : !bSuccessful || Result.error.hasValue
                  ? State->Reject(toDownloadError(Http.HttpErrorPrefix) +
                                  std::to_string(Status))
                  : Result.data.hasValue
                        ? resolveBinaryDownload(State, Result.data.value)
                        : State->Reject(toDownloadError(Http.NetworkFailed));
}

/** User Story: As dependency transport, I need each binary request executed through RTK Query so every SDK network effect shares one transport boundary. @fn void continueBinaryDownload(const FString &RequestUrl, const TSharedPtr<FBinaryDownloadState> &State, int32 RedirectCount) */
void continueBinaryDownload(const FString &RequestUrl,
                            const TSharedPtr<FBinaryDownloadState> &State,
                            int32 RedirectCount) {
  const Dependencies::Manifest::FManifest &Settings =
      Dependencies::Manifest::manifest();
  rtk::FetchBaseQueryArgs Options;
  Options.responseHandler = FString();
  rtk::FetchArgs Args;
  Args.url = RequestUrl;
  Args.method = Settings.Http.Method;
  Args.timeout = FMath::RoundToInt(
      FTimespan::FromSeconds(Settings.DownloadTimeoutSeconds)
          .GetTotalMilliseconds());
  rtk::fetchBaseQuery<TArray<uint8>>(Options)(
      Args, rtk::BaseQueryApi(), rtk::FEmptyPayload{})
      .then([State, RedirectCount](
                const rtk::QueryReturnValue<TArray<uint8>> &Result) {
        handleBinaryDownloadResponse(Result, State, RedirectCount);
      })
      .catch_([State](const std::string &) {
        State->Reject(toDownloadError(
            Dependencies::Manifest::manifest().Http.NetworkFailed));
      })
      .execute();
}

} // namespace

namespace Dependencies::DownloadAdapters {

/** User Story: As dependency setup, I need binary assets downloaded through RTK Query so installers own no transport policy. @fn func::AsyncResult<FString> downloadBinary(const FString &Url, const FString &Destination) */
func::AsyncResult<FString> downloadBinary(const FString &Url,
                                          const FString &Destination) {
  return func::createAsyncResult<FString>(
      [Url, Destination](std::function<void(FString)> Resolve,
                         std::function<void(std::string)> Reject) {
        const TSharedPtr<FBinaryDownloadState> State =
            MakeShared<FBinaryDownloadState>(
                FBinaryDownloadState{Destination, Resolve, Reject});
        continueBinaryDownload(
            Url, State,
            Dependencies::Manifest::manifest().Http.InitialRedirectCount);
      });
}

} // namespace Dependencies::DownloadAdapters
