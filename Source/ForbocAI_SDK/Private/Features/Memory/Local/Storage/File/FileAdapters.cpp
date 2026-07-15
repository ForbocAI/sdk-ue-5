#include "Features/Memory/Local/Storage/File/StorageFileAdapters.h"

#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Misc/FileHelper.h"

namespace {

struct FBinaryDownloadState {
  FString Destination;
  std::function<void(FString)> Resolve;
  std::function<void(std::string)> Reject;
};

bool IsRedirectCode(const int32 Code) {
  return Code == 301 || Code == 302 || Code == 303 || Code == 307 ||
         Code == 308;
}

void ContinueBinaryDownload(const FString &RequestUrl,
                            const TSharedPtr<FBinaryDownloadState> &State);

void ResolveBinaryDownload(const TSharedPtr<FBinaryDownloadState> &State,
                           const TArray<uint8> &Payload) {
  FFileHelper::SaveArrayToFile(Payload, *State->Destination)
      ? State->Resolve(State->Destination)
      : State->Reject("Failed to save downloaded file to disk");
}

void HandleBinaryDownloadResponse(
    FHttpResponsePtr Response, const bool bWasSuccessful,
    const TSharedPtr<FBinaryDownloadState> &State) {
  (!bWasSuccessful || !Response.IsValid())
      ? State->Reject("Network failure downloading binary")
      : IsRedirectCode(Response->GetResponseCode()) &&
                !Response->GetHeader(TEXT("Location")).IsEmpty()
            ? ContinueBinaryDownload(Response->GetHeader(TEXT("Location")),
                                     State)
            : (Response->GetResponseCode() < 200 ||
               Response->GetResponseCode() >= 300)
                  ? State->Reject(std::string("HTTP error ") +
                                  std::to_string(Response->GetResponseCode()))
                  : ResolveBinaryDownload(State, Response->GetContent());
}

void ContinueBinaryDownload(const FString &RequestUrl,
                            const TSharedPtr<FBinaryDownloadState> &State) {
  TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request =
      FHttpModule::Get().CreateRequest();
  Request->SetURL(RequestUrl);
  Request->SetVerb(TEXT("GET"));
  Request->OnProcessRequestComplete().BindLambda(
      [State](FHttpRequestPtr RequestPtr, FHttpResponsePtr Response,
              bool bWasSuccessful) {
        HandleBinaryDownloadResponse(Response, bWasSuccessful, State);
      });
  Request->ProcessRequest();
}

} // namespace

namespace Native::File {

func::AsyncResult<FString> DownloadBinary(const FString &Url,
                                          const FString &DestPath) {
  return func::createAsyncResult<FString>(
      [Url, DestPath](std::function<void(FString)> Resolve,
                      std::function<void(std::string)> Reject) {
        TSharedPtr<FBinaryDownloadState> State =
            MakeShared<FBinaryDownloadState>();
        State->Destination = DestPath;
        State->Resolve = Resolve;
        State->Reject = Reject;
        ContinueBinaryDownload(Url, State);
      });
}

} // namespace Native::File
