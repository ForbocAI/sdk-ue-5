#pragma once

#include "Features/Data/DataAdapters.h"
#include "Features/Protocol/Logger/Configuration/LoggerConfigurationTypes.h"

namespace LoggerConfiguration {

/**
 * User Story: As protocol logging, I need one immutable authored contract
 * shared with TS so action selection and log vocabulary cannot drift.
 * @fn inline const FLoggerData &loggerData()
 */
inline const FLoggerData &loggerData() {
  static const FLoggerData Data = []() {
    const DataAdapters::FSettingsSource Source =
        DataAdapters::SettingsSource(TEXT("ForbocAI_SDK"),
                                     TEXT("Data/protocol/logger.json"));
    const auto Options = DataAdapters::ReadObjectField(Source, TEXT("options"));
    const auto RuntimeTypes =
        DataAdapters::ReadObjectField(Source, TEXT("runtimeTypes"));
    const auto Levels = DataAdapters::ReadObjectField(Source, TEXT("levels"));
    const auto Text = DataAdapters::ReadObjectField(Source, TEXT("text"));
    const auto Boolean = DataAdapters::ReadObjectField(Source, TEXT("boolean"));
    const auto Delta = DataAdapters::ReadObjectField(Source, TEXT("delta"));
    const auto Labels = DataAdapters::ReadObjectField(Source, TEXT("labels"));
    const auto Fields = DataAdapters::ReadObjectField(Source, TEXT("fields"));
    const auto Npc = DataAdapters::ReadObjectField(Fields, TEXT("npc"));
    const auto Memory = DataAdapters::ReadObjectField(Fields, TEXT("memory"));
    const auto Directive =
        DataAdapters::ReadObjectField(Fields, TEXT("directive"));
    const auto Bridge = DataAdapters::ReadObjectField(Fields, TEXT("bridge"));
    const auto Soul = DataAdapters::ReadObjectField(Fields, TEXT("soul"));
    const auto Ghost = DataAdapters::ReadObjectField(Fields, TEXT("ghost"));
    const auto Vector = DataAdapters::ReadObjectField(Fields, TEXT("vector"));
    const auto Dependencies =
        DataAdapters::ReadObjectField(Fields, TEXT("dependencies"));
    const auto Extra = DataAdapters::ReadObjectField(Fields, TEXT("extra"));
    return FLoggerData{
        DataAdapters::ReadStringArrayField(Source.Root,
                                           TEXT("actionPrefixes")),
        {DataAdapters::ReadStringField(RuntimeTypes, TEXT("string")),
         DataAdapters::ReadStringField(RuntimeTypes, TEXT("undefined"))},
        {DataAdapters::ReadStringField(Levels, TEXT("log")),
         DataAdapters::ReadStringField(Levels, TEXT("info")),
         DataAdapters::ReadStringField(Levels, TEXT("warn")),
         DataAdapters::ReadStringField(Levels, TEXT("error"))},
        {DataAdapters::ReadBooleanField(Options, TEXT("enabled")),
         DataAdapters::ReadStringField(Options, TEXT("grouping")),
         DataAdapters::ReadStringField(Options, TEXT("level")),
         DataAdapters::ReadStringField(Options, TEXT("prefix")),
         DataAdapters::ReadStringField(Options, TEXT("debugEnvironment")),
         DataAdapters::ReadStringField(Options, TEXT("debugEnabledValue")),
         DataAdapters::ReadStringField(Options, TEXT("nodeEnvironment")),
         DataAdapters::ReadStringField(Options, TEXT("productionEnvironment")),
         DataAdapters::ReadStringField(Options, TEXT("expandedGrouping")),
         DataAdapters::ReadStringField(Options, TEXT("collapsedGrouping")),
         DataAdapters::ReadStringField(Options, TEXT("flatGrouping"))},
        {DataAdapters::ReadStringField(Text, TEXT("empty")),
         DataAdapters::ReadStringField(Text, TEXT("categoryPrefix")),
         DataAdapters::ReadStringField(Text, TEXT("action")),
         DataAdapters::ReadStringField(Text, TEXT("payload")),
         DataAdapters::ReadStringField(Text, TEXT("delta")),
         DataAdapters::ReadStringField(Text, TEXT("previousState")),
         DataAdapters::ReadStringField(Text, TEXT("nextState")),
         DataAdapters::ReadStringField(Text, TEXT("keyValueSeparator")),
         DataAdapters::ReadStringField(Text, TEXT("fieldSeparator"))},
        {DataAdapters::ReadStringField(Boolean, TEXT("true")),
         DataAdapters::ReadStringField(Boolean, TEXT("false"))},
        {DataAdapters::ReadStringField(Delta, TEXT("open")),
         DataAdapters::ReadStringField(Delta, TEXT("transition")),
         DataAdapters::ReadStringField(Delta, TEXT("close")),
         DataAdapters::ReadStringField(Delta, TEXT("separator")),
         DataAdapters::ReadStringField(Delta, TEXT("none"))},
        {DataAdapters::ReadStringField(Labels, TEXT("npcs")),
         DataAdapters::ReadStringField(Labels, TEXT("memory")),
         DataAdapters::ReadStringField(Labels, TEXT("directives")),
         DataAdapters::ReadStringField(Labels, TEXT("bridge")),
         DataAdapters::ReadStringField(Labels, TEXT("soul")),
         DataAdapters::ReadStringField(Labels, TEXT("ghost")),
         DataAdapters::ReadStringField(Labels, TEXT("vector")),
         DataAdapters::ReadStringField(Labels, TEXT("dependencies")),
         DataAdapters::ReadStringField(Labels, TEXT("extra"))},
        {DataAdapters::ReadStringField(Npc, TEXT("ids")),
         DataAdapters::ReadStringField(Npc, TEXT("active")),
         DataAdapters::ReadStringField(Memory, TEXT("ids")),
         DataAdapters::ReadStringField(Memory, TEXT("store")),
         DataAdapters::ReadStringField(Memory, TEXT("recall")),
         DataAdapters::ReadStringField(Memory, TEXT("recalled")),
         DataAdapters::ReadStringField(Memory, TEXT("error")),
         DataAdapters::ReadStringField(Directive, TEXT("ids")),
         DataAdapters::ReadStringField(Directive, TEXT("active")),
         DataAdapters::ReadStringField(Bridge, TEXT("status")),
         DataAdapters::ReadStringField(Bridge, TEXT("presets")),
         DataAdapters::ReadStringField(Bridge, TEXT("rulesets")),
         DataAdapters::ReadStringField(Bridge, TEXT("presetIds")),
         DataAdapters::ReadStringField(Bridge, TEXT("validated")),
         DataAdapters::ReadStringField(Bridge, TEXT("error")),
         DataAdapters::ReadStringField(Soul, TEXT("export")),
         DataAdapters::ReadStringField(Soul, TEXT("import")),
         DataAdapters::ReadStringField(Soul, TEXT("hasExport")),
         DataAdapters::ReadStringField(Soul, TEXT("hasImport")),
         DataAdapters::ReadStringField(Soul, TEXT("available")),
         DataAdapters::ReadStringField(Soul, TEXT("error")),
         DataAdapters::ReadStringField(Ghost, TEXT("session")),
         DataAdapters::ReadStringField(Ghost, TEXT("status")),
         DataAdapters::ReadStringField(Ghost, TEXT("progress")),
         DataAdapters::ReadStringField(Ghost, TEXT("hasResults")),
         DataAdapters::ReadStringField(Ghost, TEXT("history")),
         DataAdapters::ReadStringField(Ghost, TEXT("loading")),
         DataAdapters::ReadStringField(Ghost, TEXT("error")),
         DataAdapters::ReadStringField(Vector, TEXT("status")),
         DataAdapters::ReadStringField(Vector, TEXT("ready")),
         DataAdapters::ReadStringField(Vector, TEXT("error")),
         DataAdapters::ReadStringField(Dependencies, TEXT("status")),
         DataAdapters::ReadStringField(Dependencies, TEXT("report")),
         DataAdapters::ReadStringField(Dependencies, TEXT("result")),
         DataAdapters::ReadStringField(Dependencies, TEXT("error")),
         DataAdapters::ReadStringField(Extra, TEXT("entries"))}};
  }();
  return Data;
}

} // namespace LoggerConfiguration
