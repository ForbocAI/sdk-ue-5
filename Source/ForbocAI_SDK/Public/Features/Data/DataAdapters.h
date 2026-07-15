#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace SDKData {

inline FString DataPath(const FString &RelativePath) {
  const TSharedPtr<IPlugin> Plugin =
      IPluginManager::Get().FindPlugin(TEXT("ForbocAI_SDK"));
  check(Plugin.IsValid());
  return FPaths::Combine(Plugin->GetContentDir(), TEXT("Data"), RelativePath);
}

inline TSharedRef<FJsonObject> LoadObject(const FString &RelativePath) {
  FString Json;
  check(FFileHelper::LoadFileToString(Json, *DataPath(RelativePath)));
  TSharedPtr<FJsonObject> Object;
  const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
  check(FJsonSerializer::Deserialize(Reader, Object));
  check(Object.IsValid());
  return Object.ToSharedRef();
}

} // namespace SDKData
