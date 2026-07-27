#include "Systems/Dependencies/Process/DependenciesProcessAdapters.h"
#include "Components/AuthoredValues/AuthoredValuesTypes.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformFile.h"
#include "HAL/PlatformFileManager.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformTime.h"
#include "Misc/Paths.h"

namespace {

/** User Story: As a features dependencies process consumer, I need to invoke find matching directory through a stable signature so the features dependencies process workflow remains explicit and composable. @fn FString findMatchingDirectory(const TArray<FString> &Directories, const FString &Root, const FString &Prefix, int32 Index) */
FString findMatchingDirectory(const TArray<FString> &Directories,
                              const FString &Root, const FString &Prefix,
                              int32 Index) {
  return Index >= Directories.Num()
             ? FString()
             : Directories[Index].StartsWith(Prefix)
                   ? Root / Directories[Index]
                   : findMatchingDirectory(Directories, Root, Prefix,
                                           Index + FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4);
}

/** User Story: As a features dependencies process consumer, I need to invoke poll process through a stable signature so the features dependencies process workflow remains explicit and composable. @fn int32 pollProcess(FProcHandle &Process, void *ReadPipe, FString &Output, float TimeoutSeconds, double StartTime) */
int32 pollProcess(FProcHandle &Process, void *ReadPipe, FString &Output,
                  float TimeoutSeconds, double StartTime) {
  return !FPlatformProcess::IsProcRunning(Process)
             ? FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA
         : FPlatformTime::Seconds() - StartTime > TimeoutSeconds
             ? (FPlatformProcess::TerminateProc(Process), -FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4)
             : (Output += FPlatformProcess::ReadPipe(ReadPipe),
                FPlatformProcess::Sleep(FORBOCAI_SDK_AUTHORED_NUMBERV449E2C0F44AC),
                pollProcess(Process, ReadPipe, Output, TimeoutSeconds,
                            StartTime));
}

} // namespace

namespace DependenciesProcessAdapters {

/** User Story: As a features dependencies process consumer, I need to invoke find subdir with prefix through a stable signature so the features dependencies process workflow remains explicit and composable. @fn FString findSubdirWithPrefix(const FString &Directory, const FString &Prefix) */
FString findSubdirWithPrefix(const FString &Directory,
                             const FString &Prefix) {
  TArray<FString> Directories;
  IFileManager::Get().FindFiles(Directories, *(Directory / TEXT(FORBOCAI_SDK_AUTHORED_STRINGV6BD05BEAAE27)), false,
                                true);
  return findMatchingDirectory(Directories, Directory, Prefix, FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA);
}

/** User Story: As a features dependencies process consumer, I need to invoke run process through a stable signature so the features dependencies process workflow remains explicit and composable. @fn int32 runProcess(const FString &Executable, const FString &Arguments, const FString &WorkingDirectory, float TimeoutSeconds) */
int32 runProcess(const FString &Executable, const FString &Arguments,
                 const FString &WorkingDirectory, float TimeoutSeconds) {
  FString Output;
  int32 ReturnCode = -FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4;
  void *ReadPipe = nullptr;
  void *WritePipe = nullptr;
  FPlatformProcess::CreatePipe(ReadPipe, WritePipe);

  FProcHandle Process = FPlatformProcess::CreateProc(
      *Executable, *Arguments, false, true, true, nullptr, FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA,
      *(WorkingDirectory.IsEmpty() ? FPaths::ProjectDir() : WorkingDirectory),
      WritePipe, ReadPipe);

  return !Process.IsValid()
             ? (FPlatformProcess::ClosePipe(ReadPipe, WritePipe), -FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4)
             : [&]() {
                 const int32 PollResult =
                     pollProcess(Process, ReadPipe, Output, TimeoutSeconds,
                                 FPlatformTime::Seconds());
                 Output += FPlatformProcess::ReadPipe(ReadPipe);
                 PollResult == FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA
                     ? (FPlatformProcess::GetProcReturnCode(Process,
                                                            &ReturnCode),
                        void())
                     : void();
                 FPlatformProcess::ClosePipe(ReadPipe, WritePipe);
                 FPlatformProcess::CloseProc(Process);
                 return PollResult == FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA ? ReturnCode : -FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4;
               }();
}

/** User Story: As a features dependencies process consumer, I need to invoke extract zip through a stable signature so the features dependencies process workflow remains explicit and composable. @fn bool extractZip(const FString &ArchivePath, const FString &DestinationDirectory) */
bool extractZip(const FString &ArchivePath,
                const FString &DestinationDirectory) {
  FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(
      *DestinationDirectory);
#if PLATFORM_MAC || PLATFORM_LINUX
  return runProcess(
             TEXT(FORBOCAI_SDK_AUTHORED_STRINGVC6048CE5BF86),
             FString::Printf(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV46984212410F), *ArchivePath,
                             *DestinationDirectory)) == FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA;
#elif PLATFORM_WINDOWS
  return runProcess(
             TEXT(FORBOCAI_SDK_AUTHORED_STRINGV092D6FEFA3D8),
             FString::Printf(
                 TEXT(FORBOCAI_SDK_AUTHORED_STRINGV1B08E7B58AD6
                      FORBOCAI_SDK_AUTHORED_STRINGV619CE2E5592D),
                 *ArchivePath, *DestinationDirectory)) == FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA;
#else
  return false;
#endif
}

} // namespace DependenciesProcessAdapters
