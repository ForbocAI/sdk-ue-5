#include "CoreMinimal.h"
#include "Components/AuthoredValues/AuthoredValuesTypes.h"
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "Internationalization/Regex.h"

// Define a test that natively runs the BDDCoverage evaluation check
DEFINE_SPEC(FBddCoverageSpec, FORBOCAI_SDK_AUTHORED_STRINGVECFAB5F86A1B,
            EAutomationTestFlags::ProductFilter |
                EAutomationTestFlags_ApplicationContextMask)

namespace {
/** User Story: As a tests bdd consumer, I need to invoke find files through a stable signature so the tests bdd workflow remains explicit and composable. @fn TArray<FString> FindFiles(const FString& Directory, const FString& Extension) */
TArray<FString> FindFiles(const FString& Directory, const FString& Extension) {
    TArray<FString> Files;
    IFileManager::Get().FindFilesRecursive(Files, *Directory, *FString::Printf(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV46B39534B892), *Extension), true, false, false);
    return Files;
}

/**
 * User Story: As a developer, I need ReadFile to fulfill its role in the module.
 * @fn FString ReadFile(const FString& Path)
 */
FString ReadFile(const FString& Path) {
    FString Content;
    FFileHelper::LoadFileToString(Content, *Path);
    return Content;
}

/**
 * User Story: As a developer, I need ExtractRegexMatches to fulfill its role in the module.
 * @fn void ExtractRegexMatches(const FString& Content, const FString& PatternStr, int32 GroupIndex, TSet<FString>& OutMatches)
 */
void ExtractRegexMatches(const FString& Content, const FString& PatternStr, int32 GroupIndex, TSet<FString>& OutMatches) {
    FRegexPattern Pattern(PatternStr);
    FRegexMatcher Matcher(Pattern, Content);
    while (Matcher.FindNext()) {
        OutMatches.Add(Matcher.GetCaptureGroup(GroupIndex));
    }
}
} // namespace

/**
 * User Story: As a developer, I need Define to fulfill its role in the module.
 * @fn void FBddCoverageSpec::Define()
 */
void FBddCoverageSpec::Define() {
  Describe(FORBOCAI_SDK_AUTHORED_STRINGV8534B17C786A, [this]() {
    It(FORBOCAI_SDK_AUTHORED_STRINGV055BD5FD1AA9, [this]() {
      // Find the absolute root for the SDK Plugin directory
      FString PluginDir = FPaths::Combine(FPaths::ProjectDir(), TEXT(FORBOCAI_SDK_AUTHORED_STRINGVE6EEB85CB7D8));
      if (!IFileManager::Get().DirectoryExists(*PluginDir)) { 
        // alternate mapping depending on UE directory structuring
        PluginDir = FPaths::Combine(FPaths::ProjectDir(), TEXT(FORBOCAI_SDK_AUTHORED_STRINGVA9E395F4F191));
      }
      
      FString SourceDir = FPaths::Combine(PluginDir, TEXT(FORBOCAI_SDK_AUTHORED_STRINGVE76AFDDEE51C));
      
      TSet<FString> CoveredTags;
      
      // Parse @covers tags from test files
      FString TestsDir = FPaths::Combine(SourceDir, TEXT(FORBOCAI_SDK_AUTHORED_STRINGV1A5DCAFCCBF0));
      TArray<FString> TestFiles = FindFiles(TestsDir, TEXT(FORBOCAI_SDK_AUTHORED_STRINGV4A32740A7EA9));
      TestFiles.Append(FindFiles(TestsDir, TEXT(FORBOCAI_SDK_AUTHORED_STRINGVB7B1381AD51B)));
      
      FRegexPattern CoversPattern(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV888D591EA543));
      for (const FString& File : TestFiles) {
          FString Content = ReadFile(File);
          FRegexMatcher Matcher(CoversPattern, Content);
          while (Matcher.FindNext()) {
              CoveredTags.Add(FString::Printf(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV59D95A028C65), *Matcher.GetCaptureGroup(FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4), *Matcher.GetCaptureGroup(FORBOCAI_SDK_AUTHORED_NUMBERV6AC392A47561)));
          }
      }
      
      auto CheckCoverage = [this, &CoveredTags](const FString& Category, const TSet<FString>& Expected) {
          int32 MissingCount = FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA;
          for (const FString& Item : Expected) {
              FString Tag = FString::Printf(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV59D95A028C65), *Category, *Item);
              if (!CoveredTags.Contains(Tag)) {
                  AddError(FString::Printf(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVA0325C3FDBC7), *Tag));
                  MissingCount++;
              }
          }
          if (Expected.Num() > FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA) {
              TestTrue(FString::Printf(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV83A7AEE844F6), *Category, MissingCount), MissingCount == FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA);
          }
      };

      // 1. Expected API
      TSet<FString> ExpectedApi;
      TArray<FString> ApiEndpointFiles = FindFiles(FPaths::Combine(SourceDir, TEXT(FORBOCAI_SDK_AUTHORED_STRINGV9682CF430076)), TEXT(FORBOCAI_SDK_AUTHORED_STRINGVB7B1381AD51B));
      for (const FString& File : ApiEndpointFiles) {
          ExtractRegexMatches(
              ReadFile(File),
              TEXT(FORBOCAI_SDK_AUTHORED_STRINGV9002FE0B9CB4),
              FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4, ExpectedApi);
      }
      CheckCoverage(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVF52209C81EE7), ExpectedApi);
      
      // 2. Expected Core Thunks
      TSet<FString> ExpectedThunks;
      TArray<FString> PublicFiles = FindFiles(FPaths::Combine(SourceDir, TEXT(FORBOCAI_SDK_AUTHORED_STRINGVB107DA7975A1)), TEXT(FORBOCAI_SDK_AUTHORED_STRINGVB7B1381AD51B));
      for (const FString& File : PublicFiles) {
          ExtractRegexMatches(ReadFile(File), TEXT(FORBOCAI_SDK_AUTHORED_STRINGV0C0E8ACD5680), FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4, ExpectedThunks);
      }
      CheckCoverage(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV50DC8D98F115), ExpectedThunks);
      
      // 3. Expected CLI Ops
      TSet<FString> ExpectedCliOps;
      const TArray<FString> CliThunkFiles = FindFiles(
          FPaths::Combine(SourceDir, TEXT(FORBOCAI_SDK_AUTHORED_STRINGV277067D48906)),
          TEXT(FORBOCAI_SDK_AUTHORED_STRINGV5AEA4D95615E));
      for (const FString &File : CliThunkFiles) {
          const FString NsContent = ReadFile(File);
          const int32 OpsNamespace = NsContent.Find(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV7F1BFF94B326));
          if (OpsNamespace == INDEX_NONE) {
              continue;
          }
          FRegexPattern FnPattern(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV596B111208E6));
          FRegexMatcher FnMatcher(FnPattern, NsContent.Mid(OpsNamespace));
          while (FnMatcher.FindNext()) {
              FString Name = FnMatcher.GetCaptureGroup(FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4);
              ExpectedCliOps.Add(Name);
          }
      }
      CheckCoverage(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV1966298E2D18), ExpectedCliOps);
      
      // 4. Expected CLI Actions
      TSet<FString> ExpectedCli;
      const FString CliAdapters = ReadFile(FPaths::Combine(
          SourceDir, TEXT(FORBOCAI_SDK_AUTHORED_STRINGV49DFD4AC3C14)));
      const int32 Begin = CliAdapters.Find(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVA30AA6724A25));
      const int32 End = CliAdapters.Find(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVFEFFEE112DF6));
      if (Begin != INDEX_NONE && End > Begin) {
          ExtractRegexMatches(CliAdapters.Mid(Begin, End - Begin),
                              TEXT(FORBOCAI_SDK_AUTHORED_STRINGV508EF483DDBF), FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4,
                              ExpectedCli);
      }
      CheckCoverage(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVDEF1C0C3F798), ExpectedCli);
    });
  });
}
