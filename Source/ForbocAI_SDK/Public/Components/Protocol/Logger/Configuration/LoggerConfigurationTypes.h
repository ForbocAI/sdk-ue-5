#pragma once

#include "CoreMinimal.h"

namespace LoggerConfiguration {

struct FRuntimeTypes {
  FString Object;
  FString String;
  FString Undefined;
};

struct FLevelData {
  FString Log;
  FString Info;
  FString Warn;
  FString Error;
};

struct FOptionsData {
  bool Enabled{};
  FString Grouping;
  FString Level;
  FString Prefix;
  FString DebugEnvironment;
  FString DebugEnabledValue;
  FString NodeEnvironment;
  FString ProductionEnvironment;
  FString ExpandedGrouping;
  FString CollapsedGrouping;
  FString FlatGrouping;
};

struct FTextData {
  FString Empty;
  FString CategoryPrefix;
  FString Action;
  FString Payload;
  FString OpaquePayload;
  FString Delta;
  FString PreviousState;
  FString NextState;
  FString KeyValueSeparator;
  FString FieldSeparator;
};

struct FBooleanData {
  FString True;
  FString False;
};

struct FDeltaData {
  FString Open;
  FString Transition;
  FString Close;
  FString Separator;
  FString None;
};

struct FLabelData {
  FString Npcs;
  FString Memory;
  FString Directives;
  FString Bridge;
  FString Soul;
  FString Ghost;
  FString Vector;
  FString Dependencies;
  FString Extra;
};

struct FFieldData {
  FString NpcIds;
  FString NpcActive;
  FString MemoryIds;
  FString MemoryStore;
  FString MemoryRecall;
  FString MemoryRecalled;
  FString MemoryError;
  FString DirectiveIds;
  FString DirectiveActive;
  FString BridgeStatus;
  FString BridgePresets;
  FString BridgeRulesets;
  FString BridgePresetIds;
  FString BridgeValidated;
  FString BridgeError;
  FString SoulExport;
  FString SoulImport;
  FString SoulHasExport;
  FString SoulHasImport;
  FString SoulAvailable;
  FString SoulError;
  FString GhostSession;
  FString GhostStatus;
  FString GhostProgress;
  FString GhostHasResults;
  FString GhostHistory;
  FString GhostLoading;
  FString GhostError;
  FString VectorStatus;
  FString VectorReady;
  FString VectorError;
  FString DependenciesStatus;
  FString DependenciesReport;
  FString DependenciesResult;
  FString DependenciesError;
  FString ExtraEntries;
};

struct FLoggerData {
  TArray<FString> ActionPrefixes;
  FRuntimeTypes RuntimeTypes;
  FLevelData Levels;
  FOptionsData Options;
  FTextData Text;
  FBooleanData Boolean;
  FDeltaData Delta;
  FLabelData Labels;
  FFieldData Fields;
};

} // namespace LoggerConfiguration
