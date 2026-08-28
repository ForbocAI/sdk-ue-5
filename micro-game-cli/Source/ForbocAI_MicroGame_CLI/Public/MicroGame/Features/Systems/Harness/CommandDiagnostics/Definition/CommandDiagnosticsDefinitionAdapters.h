#pragma once

#include "MicroGame/Features/Components/Harness/CommandDiagnostics/CommandDiagnosticsTypes.h"
#include "MicroGame/Features/Data/DataAdapters.h"

namespace MicroGame::CommandDiagnostics {

/** User Story: As all micro-game debug modes, I need one immutable JSON-owned command-diagnostics vocabulary. @fn inline const FCommandDiagnosticsData &CommandDiagnosticsData() */
inline const FCommandDiagnosticsData &CommandDiagnosticsData() {
  static const FCommandDiagnosticsData Data = []() {
    const DataAdapters::FSettingsSource Source =
        DataAdapters::SettingsSource(TEXT("harness/debug.json"));
    const TSharedRef<FJsonObject> Root = Source.Root;
    return FCommandDiagnosticsData{
        DataAdapters::ReadStringField(Root, TEXT("header")),
        DataAdapters::ReadStringField(Root, TEXT("footer")),
        DataAdapters::ReadStringField(Root, TEXT("environmentPrefix")),
        DataAdapters::ReadStringField(Root, TEXT("apiUrlEnvironmentKey")),
        DataAdapters::ReadStringField(Root, TEXT("apiKeyEnvironmentKey")),
        DataAdapters::ReadStringField(Root, TEXT("protocolEnvironmentKey")),
        DataAdapters::ReadStringField(Root, TEXT("promptEnvironmentKey")),
        DataAdapters::ReadStringField(Root, TEXT("enabledValue")),
        DataAdapters::ReadStringField(Root, TEXT("unsetValue")),
        DataAdapters::ReadStringField(Root, TEXT("configuredValue")),
        DataAdapters::ReadStringField(Root, TEXT("missingValue")),
        DataAdapters::ReadStringField(Root, TEXT("apiUrlLabel")),
        DataAdapters::ReadStringField(Root, TEXT("authLabel")),
        DataAdapters::ReadStringField(Root, TEXT("commandPrefix")),
        DataAdapters::ReadStringField(Root, TEXT("groupPrefix")),
        DataAdapters::ReadStringField(Root, TEXT("routesPrefix")),
        DataAdapters::ReadStringField(Root, TEXT("assertionsPrefix")),
        DataAdapters::ReadStringField(Root, TEXT("statusPrefix")),
        DataAdapters::ReadStringField(Root, TEXT("durationPrefix")),
        DataAdapters::ReadStringField(Root, TEXT("outputPrefix")),
        DataAdapters::ReadStringField(Root, TEXT("routePrefix")),
        DataAdapters::ReadStringField(Root, TEXT("none")),
        DataAdapters::ReadStringField(Root, TEXT("emptyOutput")),
        DataAdapters::ReadStringField(Root, TEXT("lineSeparator")),
        DataAdapters::ReadStringField(Root, TEXT("listSeparator")),
        DataAdapters::ReadStringField(Root, TEXT("assertionSeparator")),
        DataAdapters::ReadStringField(Root, TEXT("blankLine")),
    };
  }();
  return Data;
}

} // namespace MicroGame::CommandDiagnostics
