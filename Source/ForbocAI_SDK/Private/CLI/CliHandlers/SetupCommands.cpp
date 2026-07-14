#include "CLI/CliHandlers.h"
#include "Features/Async/AsyncAdapters.h"
#include "Features/Config/ConfigAdapters.h"
#include "Features/Memory/Local/Storage/StorageAdapters.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace CLIOps {
namespace Handlers {

namespace {

/**
 * Version constants (single source of truth for ThirdParty setup)
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
static const TCHAR *SQLITE_VERSION = TEXT("3460100");
static const TCHAR *SQLITE_VEC_VERSION = TEXT("v0.1.6");

/**
 * Finds the first subdirectory matching a prefix inside a directory.
 * User Story: As dependency setup, I need directory discovery so extracted
 * archives can be relocated without hard-coding versioned folder names.
 */
FString FindSubdirWithPrefix(const FString &Dir, const FString &Prefix) {
  TArray<FString> Dirs;
  IFileManager::Get().FindFiles(Dirs, *(Dir / TEXT("*")), false, true);

  struct FindHelper {
    static FString apply(const TArray<FString> &Arr, const FString &Dir,
                         const FString &Prefix, int32 Idx) {
      return Idx >= Arr.Num()
                 ? TEXT("")
                 : Arr[Idx].StartsWith(Prefix)
                       ? Dir / Arr[Idx]
                       : apply(Arr, Dir, Prefix, Idx + 1);
    }
  };

  return FindHelper::apply(Dirs, Dir, Prefix, 0);
}

/**
 * Runs an external process synchronously.
 * User Story: As setup automation, I need process execution wrapped so build
 * and extraction commands can run with shared logging and timeout handling.
 */
int32 RunProcess(const FString &Executable, const FString &Args,
                 const FString &WorkingDir = TEXT(""),
                 float TimeoutSeconds = 300.0f) {
  FString StdOut;
  FString StdErr;
  int32 ReturnCode = -1;

  const FString EffectiveDir =
      WorkingDir.IsEmpty() ? FPaths::ProjectDir() : WorkingDir;

  UE_LOG(LogTemp, Display, TEXT("  [exec] %s %s"), *Executable, *Args);

  void *ReadPipe = nullptr;
  void *WritePipe = nullptr;
  FPlatformProcess::CreatePipe(ReadPipe, WritePipe);

  FProcHandle Proc = FPlatformProcess::CreateProc(
      *Executable, *Args, false, true, true, nullptr, 0, *EffectiveDir,
      WritePipe, ReadPipe);

  return !Proc.IsValid()
    ? [&]() -> int32 {
        UE_LOG(LogTemp, Warning, TEXT("  [FAIL] Could not launch %s"),
               *Executable);
        FPlatformProcess::ClosePipe(ReadPipe, WritePipe);
        return -1;
      }()
    : [&]() -> int32 {
        /**
         * Wait with timeout — recursive helper replaces while loop
         * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
         */
        struct WaitHelper {
          static int32 poll(FProcHandle &Proc, void *ReadPipe, void *WritePipe,
                            FString &StdOut, float TimeoutSeconds,
                            double StartTime) {
            return !FPlatformProcess::IsProcRunning(Proc)
              ? 0
              : [&]() -> int32 {
                  StdOut += FPlatformProcess::ReadPipe(ReadPipe);
                  return (FPlatformTime::Seconds() - StartTime > TimeoutSeconds)
                    ? [&]() -> int32 {
                        FPlatformProcess::TerminateProc(Proc);
                        UE_LOG(LogTemp, Warning, TEXT("  [FAIL] Process timed out after %.0fs"),
                               TimeoutSeconds);
                        FPlatformProcess::ClosePipe(ReadPipe, WritePipe);
                        return -1;
                      }()
                    : [&]() -> int32 {
                        FPlatformProcess::Sleep(0.1f);
                        return poll(Proc, ReadPipe, WritePipe, StdOut,
                                    TimeoutSeconds, StartTime);
                      }();
                }();
          }
        };

        const double StartTime = FPlatformTime::Seconds();
        const int32 PollResult = WaitHelper::poll(
            Proc, ReadPipe, WritePipe, StdOut, TimeoutSeconds, StartTime);

        return PollResult == -1
          ? PollResult
          : [&]() -> int32 {
              StdOut += FPlatformProcess::ReadPipe(ReadPipe);
              FPlatformProcess::GetProcReturnCode(Proc, &ReturnCode);
              FPlatformProcess::ClosePipe(ReadPipe, WritePipe);

              (ReturnCode != 0 && !StdOut.IsEmpty())
                ? [&]() { UE_LOG(LogTemp, Warning, TEXT("  %s"), *StdOut); }()
                : (void)0;

              return ReturnCode;
            }();
      }();
}

/**
 * Extracts a zip file to a destination directory.
 * User Story: As dependency setup, I need zip extraction so downloaded archives
 * can be unpacked into the ThirdParty layout automatically.
 */
bool ExtractZip(const FString &ZipPath, const FString &DestDir) {
  IPlatformFile &PF = FPlatformFileManager::Get().GetPlatformFile();
  PF.CreateDirectoryTree(*DestDir);

#if PLATFORM_MAC || PLATFORM_LINUX
  return RunProcess(TEXT("/usr/bin/unzip"), FString::Printf(
      TEXT("-qo \"%s\" -d \"%s\""), *ZipPath, *DestDir)) == 0;
#elif PLATFORM_WINDOWS
  /**
   * PowerShell Expand-Archive
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  return RunProcess(TEXT("powershell.exe"), FString::Printf(
      TEXT("-NoProfile -Command \"Expand-Archive -Force -Path '%s' -DestinationPath '%s'\""),
      *ZipPath, *DestDir)) == 0;
#else
  UE_LOG(LogTemp, Warning, TEXT("  [FAIL] Zip extraction not supported on this platform"));
  return false;
#endif
}

/**
 * Extracts a tar.gz file to a destination directory.
 * User Story: As dependency setup, I need tarball extraction so downloaded
 * source archives can be unpacked into the expected build directories.
 */
bool ExtractTarGz(const FString &TarPath, const FString &DestDir) {
  IPlatformFile &PF = FPlatformFileManager::Get().GetPlatformFile();
  PF.CreateDirectoryTree(*DestDir);

#if PLATFORM_MAC || PLATFORM_LINUX
  return RunProcess(TEXT("/usr/bin/tar"), FString::Printf(
      TEXT("xzf \"%s\" -C \"%s\""), *TarPath, *DestDir)) == 0;
#elif PLATFORM_WINDOWS
  return RunProcess(TEXT("tar.exe"), FString::Printf(
      TEXT("xzf \"%s\" -C \"%s\""), *TarPath, *DestDir)) == 0;
#else
  return false;
#endif
}

/**
 * Reports ThirdParty dependency presence without modifying anything.
 * User Story: As setup diagnostics, I need a read-only verification pass so I
 * can inspect native dependency readiness before changing the workspace.
 */
Result VerifyThirdParty() {
  const FString PluginDir =
      FPaths::ProjectPluginsDir() / TEXT("ForbocAI_SDK");
  const FString ThirdPartyDir = PluginDir / TEXT("ThirdParty");

  const FString SqliteInclude = ThirdPartyDir / TEXT("sqlite-vss/include");
  const FString SqliteSrc = ThirdPartyDir / TEXT("sqlite-vss/src");

  IPlatformFile &PF = FPlatformFileManager::Get().GetPlatformFile();

  const bool bSqliteHeaders = PF.DirectoryExists(*SqliteInclude) &&
                              PF.FileExists(*(SqliteInclude / TEXT("sqlite3.h")));
  const bool bSqliteAmalgamation =
      PF.DirectoryExists(*SqliteSrc) &&
      PF.FileExists(*(SqliteSrc / TEXT("sqlite3.c")));
  const bool bVec0 = PF.FileExists(*(SqliteSrc / TEXT("vec0.c")));

  const FString LocalInfra = FPaths::ProjectDir() / TEXT("local_infrastructure");
  const FString VectorsDir = LocalInfra / TEXT("vectors");
  const bool bVectorDb = PF.DirectoryExists(*VectorsDir);

  UE_LOG(LogTemp, Display, TEXT(""));
  UE_LOG(LogTemp, Display, TEXT("=== ForbocAI ThirdParty Dependency Check ==="));
  UE_LOG(LogTemp, Display, TEXT(""));
  UE_LOG(LogTemp, Display, TEXT("  [%s] sqlite3 headers       (%s)"),
         bSqliteHeaders ? TEXT("OK") : TEXT("--"), *SqliteInclude);
  UE_LOG(LogTemp, Display, TEXT("  [%s] sqlite3 amalgamation  (%s)"),
         bSqliteAmalgamation ? TEXT("OK") : TEXT("--"), *SqliteSrc);
  UE_LOG(LogTemp, Display, TEXT("  [%s] sqlite-vec (vec0.c)   (%s)"),
         bVec0 ? TEXT("OK") : TEXT("--"),
         *(SqliteSrc / TEXT("vec0.c")));
  UE_LOG(LogTemp, Display, TEXT(""));
  UE_LOG(LogTemp, Display, TEXT("  Compile defines:"));
  UE_LOG(LogTemp, Display, TEXT("    WITH_FORBOC_SQLITE_VEC = %d"),
         WITH_FORBOC_SQLITE_VEC);
  UE_LOG(LogTemp, Display, TEXT(""));
  UE_LOG(LogTemp, Display, TEXT("  Runtime assets:"));
  UE_LOG(LogTemp, Display, TEXT("  [%s] Vector DB             (%s)"),
         bVectorDb ? TEXT("OK") : TEXT("--"), *VectorsDir);
  UE_LOG(LogTemp, Display, TEXT(""));

  (!bSqliteHeaders || !bSqliteAmalgamation || !bVec0)
    ? [&]() {
        UE_LOG(LogTemp, Display, TEXT(
            "  To set up: setup --sqlite-only"));
      }()
    : (void)0;

  const bool bAllBuild = bSqliteHeaders && bSqliteAmalgamation && bVec0;
  return bAllBuild ? Result::Success("All build-time dependencies present")
                   : Result::Failure("Some build-time dependencies missing");
}

/**
 * Installs or verifies the ThirdParty dependency bundle.
 * User Story: As setup flows, I need one dependency installer so sqlite-vss
 * assets can be downloaded and arranged predictably.
 */
Result SetupThirdPartyDeps(rtk::EnhancedStore<FRuntimeState> &Store,
                           const TArray<FString> &Args) {
  const FString PluginDir =
      FPaths::ProjectPluginsDir() / TEXT("ForbocAI_SDK");
  const FString ThirdPartyDir = PluginDir / TEXT("ThirdParty");
  const FString TmpDir =
      FPaths::ProjectDir() / TEXT("local_infrastructure/tmp");

  IPlatformFile &PF = FPlatformFileManager::Get().GetPlatformFile();

  (void)Args;

  UE_LOG(LogTemp, Display, TEXT(""));
  UE_LOG(LogTemp, Display, TEXT("=== ForbocAI ThirdParty Setup ==="));

  const FString SqliteIncDir = ThirdPartyDir / TEXT("sqlite-vss/include");
  const FString SqliteSrcDir = ThirdPartyDir / TEXT("sqlite-vss/src");

  PF.CreateDirectoryTree(*SqliteIncDir);
  PF.CreateDirectoryTree(*SqliteSrcDir);
  PF.CreateDirectoryTree(*TmpDir);

  int32 DownloadCount = 0;
  int32 FailCount = 0;

  /**
   * DownloadOne lambda — if (FileExists) replaced with ternary
   */
  auto DownloadOne = [&Store, &DownloadCount,
                      &FailCount](const FString &Url, const FString &Dest) -> bool {
    return FPlatformFileManager::Get().GetPlatformFile().FileExists(*Dest)
      ? [&]() -> bool {
          UE_LOG(LogTemp, Display, TEXT("  [skip] %s (exists)"),
                 *FPaths::GetCleanFilename(Dest));
          return true;
        }()
      : [&]() -> bool {
          UE_LOG(LogTemp, Display, TEXT("  [download] %s"),
                 *FPaths::GetCleanFilename(Dest));
          try {
            AsyncAdapters::waitForResult(Native::File::DownloadBinary(Url, Dest),
                                       120.0);
            ++DownloadCount;
            return true;
          } catch (const std::exception &E) {
            (void)E;
            UE_LOG(LogTemp, Warning, TEXT("  [FAIL] %s: %s"),
                   *FPaths::GetCleanFilename(Dest),
                   UTF8_TO_TCHAR(E.what()));
            ++FailCount;
            return false;
          }
        }();
  };

  /**
   * sqlite3 amalgamation + sqlite-vec (replaces vendor_sqlite in script)
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  UE_LOG(LogTemp, Display, TEXT(""));
  UE_LOG(LogTemp, Display, TEXT("  --- sqlite3 + sqlite-vec ---"));

        const FString Sqlite3hDest = SqliteIncDir / TEXT("sqlite3.h");
        const FString Sqlite3extDest = SqliteIncDir / TEXT("sqlite3ext.h");
        const FString Sqlite3cDest = SqliteSrcDir / TEXT("sqlite3.c");

        /**
         * Download + extract sqlite3 amalgamation if any file missing
         * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
         */
        (!PF.FileExists(*Sqlite3hDest) || !PF.FileExists(*Sqlite3cDest))
          ? [&]() {
              const FString SqliteZipUrl = FString::Printf(
                  TEXT("https://www.sqlite.org/2024/sqlite-amalgamation-%s.zip"),
                  SQLITE_VERSION);
              const FString SqliteZip = TmpDir / TEXT("sqlite-amalgamation.zip");
              const FString SqliteExtractDir = TmpDir / TEXT("sqlite-extract");

              DownloadOne(SqliteZipUrl, SqliteZip)
                ? [&]() {
                    UE_LOG(LogTemp, Display, TEXT("  [extract] sqlite-amalgamation.zip"));
                    ExtractZip(SqliteZip, SqliteExtractDir)
                      ? [&]() {
                          const FString InnerDir = FindSubdirWithPrefix(
                              SqliteExtractDir, TEXT("sqlite-amalgamation-"));

                          !InnerDir.IsEmpty()
                            ? [&]() {
                                IFileManager &FM = IFileManager::Get();
                                FM.Copy(*Sqlite3hDest, *(InnerDir / TEXT("sqlite3.h")));
                                FM.Copy(*Sqlite3extDest, *(InnerDir / TEXT("sqlite3ext.h")));
                                FM.Copy(*Sqlite3cDest, *(InnerDir / TEXT("sqlite3.c")));
                                UE_LOG(LogTemp, Display, TEXT("  [OK] sqlite3 amalgamation installed"));
                              }()
                            : [&]() {
                                UE_LOG(LogTemp, Warning,
                                       TEXT("  [FAIL] Could not find sqlite-amalgamation-* in archive"));
                                ++FailCount;
                              }();
                        }()
                      : [&]() {
                          ++FailCount;
                        }();
                  }()
                : (void)0;
            }()
          : [&]() {
              UE_LOG(LogTemp, Display, TEXT("  [skip] sqlite3 amalgamation (exists)"));
            }();

        /**
         * Download + extract sqlite-vec
         * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
         */
        const FString Vec0Dest = SqliteSrcDir / TEXT("vec0.c");
        !PF.FileExists(*Vec0Dest)
          ? [&]() {
              const FString VecZipUrl = FString::Printf(
                  TEXT("https://github.com/asg017/sqlite-vec/releases/download/%s/"
                       "sqlite-vec-%s-amalgamation.zip"),
                  SQLITE_VEC_VERSION,
                  *FString(SQLITE_VEC_VERSION).Replace(TEXT("v"), TEXT("")));
              const FString VecZip = TmpDir / TEXT("sqlite-vec-amalgamation.zip");
              const FString VecExtractDir = TmpDir / TEXT("sqlite-vec-extract");

              DownloadOne(VecZipUrl, VecZip)
                ? [&]() {
                    UE_LOG(LogTemp, Display, TEXT("  [extract] sqlite-vec-amalgamation.zip"));
                    ExtractZip(VecZip, VecExtractDir)
                      ? [&]() {
                          /**
                           * sqlite-vec amalgamation zip contains sqlite-vec.c at root
                           * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
                           */
                          IFileManager &FM = IFileManager::Get();
                          const FString VecSrc1 = VecExtractDir / TEXT("sqlite-vec.c");
                          const FString VecSrc2 = VecExtractDir / TEXT("vec0.c");
                          /**
                           * Check for nested dir too
                           * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
                           */
                          const FString VecSubDir = FindSubdirWithPrefix(
                              VecExtractDir, TEXT("sqlite-vec-"));
                          const FString VecSrc3 =
                              VecSubDir.IsEmpty() ? TEXT("")
                                                 : (VecSubDir / TEXT("sqlite-vec.c"));

                          PF.FileExists(*VecSrc1)
                            ? [&]() { FM.Copy(*Vec0Dest, *VecSrc1); }()
                            : PF.FileExists(*VecSrc2)
                            ? [&]() { FM.Copy(*Vec0Dest, *VecSrc2); }()
                            : (!VecSrc3.IsEmpty() && PF.FileExists(*VecSrc3))
                            ? [&]() { FM.Copy(*Vec0Dest, *VecSrc3); }()
                            : [&]() {
                                /**
                                 * Try src/ subdirectory
                                 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
                                 */
                                const FString VecSrc4 =
                                    VecSubDir.IsEmpty()
                                        ? TEXT("")
                                        : (VecSubDir / TEXT("src/sqlite-vec.c"));
                                (!VecSrc4.IsEmpty() && PF.FileExists(*VecSrc4))
                                  ? [&]() { FM.Copy(*Vec0Dest, *VecSrc4); }()
                                  : [&]() {
                                      UE_LOG(LogTemp, Warning,
                                             TEXT("  [FAIL] Could not find sqlite-vec.c or vec0.c in archive"));
                                      ++FailCount;
                                    }();
                              }();

                          PF.FileExists(*Vec0Dest)
                            ? [&]() {
                                UE_LOG(LogTemp, Display, TEXT("  [OK] sqlite-vec (vec0.c) installed"));
                              }()
                            : (void)0;

                          /**
                           * Copy header if present
                           * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
                           */
                          const FString VecHdrSrc =
                              VecSubDir.IsEmpty() ? (VecExtractDir / TEXT("sqlite-vec.h"))
                                                 : (VecSubDir / TEXT("sqlite-vec.h"));
                          PF.FileExists(*VecHdrSrc)
                            ? [&]() { FM.Copy(*(SqliteIncDir / TEXT("sqlite-vec.h")), *VecHdrSrc); }()
                            : (void)0;
                        }()
                      : [&]() {
                          ++FailCount;
                        }();
                  }()
                : (void)0;
            }()
          : [&]() {
              UE_LOG(LogTemp, Display, TEXT("  [skip] sqlite-vec vec0.c (exists)"));
            }();

  /**
   * Clean up temp dir
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  IFileManager::Get().DeleteDirectory(*TmpDir, true, true);

  UE_LOG(LogTemp, Display, TEXT(""));
  UE_LOG(LogTemp, Display, TEXT("  Downloaded: %d | Failed: %d"),
         DownloadCount, FailCount);
  UE_LOG(LogTemp, Display, TEXT(
      "  Rebuild the UE project to pick up WITH_FORBOC_SQLITE_VEC."));
  UE_LOG(LogTemp, Display, TEXT(""));

  return FailCount == 0
             ? Result::Success("ThirdParty setup completed")
             : Result::Failure("Some downloads failed — see logs above");
}

} // anonymous namespace

/**
 * Routes setup-related CLI commands to the appropriate setup helper.
 * User Story: As CLI users, I need setup commands dispatched through one
 * handler so verification, install, and build flows share parsing logic.
 */
HandlerResult HandleSetup(rtk::EnhancedStore<FRuntimeState> &Store,
                          const FString &CommandKey,
                          const TArray<FString> &Args) {
  using func::just;
  using func::nothing;

  return CommandKey == TEXT("setup")
    ? just(SetupThirdPartyDeps(Store, Args))
    : CommandKey == TEXT("setup_check")
    ? [&]() -> HandlerResult {
        (void)Store;
        return just(VerifyThirdParty());
      }()
    : nothing<Result>();
}

} // namespace Handlers
} // namespace CLIOps
