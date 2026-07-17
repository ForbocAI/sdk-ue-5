#include "Features/Dependencies/Process/DependenciesProcessAdapters.h"
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
                                           Index + 1);
}

/** User Story: As a features dependencies process consumer, I need to invoke poll process through a stable signature so the features dependencies process workflow remains explicit and composable. @fn int32 pollProcess(FProcHandle &Process, void *ReadPipe, FString &Output, float TimeoutSeconds, double StartTime) */
int32 pollProcess(FProcHandle &Process, void *ReadPipe, FString &Output,
                  float TimeoutSeconds, double StartTime) {
  return !FPlatformProcess::IsProcRunning(Process)
             ? 0
         : FPlatformTime::Seconds() - StartTime > TimeoutSeconds
             ? (FPlatformProcess::TerminateProc(Process), -1)
             : (Output += FPlatformProcess::ReadPipe(ReadPipe),
                FPlatformProcess::Sleep(0.1f),
                pollProcess(Process, ReadPipe, Output, TimeoutSeconds,
                            StartTime));
}

} // namespace

namespace DependenciesProcessAdapters {

/** User Story: As a features dependencies process consumer, I need to invoke find subdir with prefix through a stable signature so the features dependencies process workflow remains explicit and composable. @fn FString findSubdirWithPrefix(const FString &Directory, const FString &Prefix) */
FString findSubdirWithPrefix(const FString &Directory,
                             const FString &Prefix) {
  TArray<FString> Directories;
  IFileManager::Get().FindFiles(Directories, *(Directory / TEXT("*")), false,
                                true);
  return findMatchingDirectory(Directories, Directory, Prefix, 0);
}

/** User Story: As a features dependencies process consumer, I need to invoke run process through a stable signature so the features dependencies process workflow remains explicit and composable. @fn int32 runProcess(const FString &Executable, const FString &Arguments, const FString &WorkingDirectory, float TimeoutSeconds) */
int32 runProcess(const FString &Executable, const FString &Arguments,
                 const FString &WorkingDirectory, float TimeoutSeconds) {
  FString Output;
  int32 ReturnCode = -1;
  void *ReadPipe = nullptr;
  void *WritePipe = nullptr;
  FPlatformProcess::CreatePipe(ReadPipe, WritePipe);

  FProcHandle Process = FPlatformProcess::CreateProc(
      *Executable, *Arguments, false, true, true, nullptr, 0,
      *(WorkingDirectory.IsEmpty() ? FPaths::ProjectDir() : WorkingDirectory),
      WritePipe, ReadPipe);

  return !Process.IsValid()
             ? (FPlatformProcess::ClosePipe(ReadPipe, WritePipe), -1)
             : [&]() {
                 const int32 PollResult =
                     pollProcess(Process, ReadPipe, Output, TimeoutSeconds,
                                 FPlatformTime::Seconds());
                 Output += FPlatformProcess::ReadPipe(ReadPipe);
                 PollResult == 0
                     ? (FPlatformProcess::GetProcReturnCode(Process,
                                                            &ReturnCode),
                        void())
                     : void();
                 FPlatformProcess::ClosePipe(ReadPipe, WritePipe);
                 FPlatformProcess::CloseProc(Process);
                 return PollResult == 0 ? ReturnCode : -1;
               }();
}

/** User Story: As a features dependencies process consumer, I need to invoke extract zip through a stable signature so the features dependencies process workflow remains explicit and composable. @fn bool extractZip(const FString &ArchivePath, const FString &DestinationDirectory) */
bool extractZip(const FString &ArchivePath,
                const FString &DestinationDirectory) {
  FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(
      *DestinationDirectory);
#if PLATFORM_MAC || PLATFORM_LINUX
  return runProcess(
             TEXT("/usr/bin/unzip"),
             FString::Printf(TEXT("-qo \"%s\" -d \"%s\""), *ArchivePath,
                             *DestinationDirectory)) == 0;
#elif PLATFORM_WINDOWS
  return runProcess(
             TEXT("powershell.exe"),
             FString::Printf(
                 TEXT("-NoProfile -Command \"Expand-Archive -Force -Path '%s' "
                      "-DestinationPath '%s'\""),
                 *ArchivePath, *DestinationDirectory)) == 0;
#else
  return false;
#endif
}

} // namespace DependenciesProcessAdapters
