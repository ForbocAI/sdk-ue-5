#pragma once
#include "MicroGame/Features/Components/AuthoredValues/AuthoredValuesTypes.h"

#include "MicroGame/Features/Components/Harness/Maze/Definition/MazeDefinitionTypes.h"
#include "MicroGame/Features/Data/DataAdapters.h"

namespace MicroGame::Maze {

/**
 * User Story: As Maze command composition, I need all operation templates decoded from one authored object.
 * @fn inline FMazeCommands ReadMazeCommands( const TSharedRef<FJsonObject> &Source)
 */
inline FMazeCommands ReadMazeCommands(
    const TSharedRef<FJsonObject> &Source) {
  return {
      DataAdapters::ReadStringField(Source, TEXT("generate")),
      DataAdapters::ReadStringField(Source, TEXT("create")),
      DataAdapters::ReadStringField(Source, TEXT("identityUpdate")),
      DataAdapters::ReadStringField(Source, TEXT("recall")),
      DataAdapters::ReadStringField(Source, TEXT("decide")),
      DataAdapters::ReadStringField(Source, TEXT("memoryStore")),
      DataAdapters::ReadStringField(Source, TEXT("typedMemoryStore")),
  };
}

/** User Story: As Maze formatting, I need all replacement tokens decoded from one authored object. @fn inline FMazeTokens ReadMazeTokens(const TSharedRef<FJsonObject> &Source) */
inline FMazeTokens ReadMazeTokens(const TSharedRef<FJsonObject> &Source) {
  return {
      DataAdapters::ReadStringField(Source, TEXT("turn")),
      DataAdapters::ReadStringField(Source, TEXT("cell")),
      DataAdapters::ReadStringField(Source, TEXT("query")),
      DataAdapters::ReadStringField(Source, TEXT("legal")),
      DataAdapters::ReadStringField(Source, TEXT("visited")),
      DataAdapters::ReadStringField(Source, TEXT("avoid")),
      DataAdapters::ReadStringField(Source, TEXT("narrowed")),
      DataAdapters::ReadStringField(Source, TEXT("thought")),
      DataAdapters::ReadStringField(Source, TEXT("reason")),
      DataAdapters::ReadStringField(Source, TEXT("optimal")),
      DataAdapters::ReadStringField(Source, TEXT("choices")),
      DataAdapters::ReadStringField(Source, TEXT("direction")),
      DataAdapters::ReadStringField(Source, TEXT("neighbor")),
      DataAdapters::ReadStringField(Source, TEXT("from")),
      DataAdapters::ReadStringField(Source, TEXT("to")),
      DataAdapters::ReadStringField(Source, TEXT("note")),
      DataAdapters::ReadStringField(Source, TEXT("dialogue")),
      DataAdapters::ReadStringField(Source, TEXT("turns")),
      DataAdapters::ReadStringField(Source, TEXT("deadEnds")),
      DataAdapters::ReadStringField(Source, TEXT("edges")),
      DataAdapters::ReadStringField(Source, TEXT("options")),
      DataAdapters::ReadStringField(Source, TEXT("moves")),
      DataAdapters::ReadStringField(Source, TEXT("id")),
      DataAdapters::ReadStringField(Source, TEXT("line")),
      DataAdapters::ReadStringField(Source, TEXT("attribute")),
      DataAdapters::ReadStringField(Source, TEXT("context")),
      DataAdapters::ReadStringField(Source, TEXT("persona")),
      DataAdapters::ReadStringField(Source, TEXT("memory")),
      DataAdapters::ReadStringField(Source, TEXT("type")),
      DataAdapters::ReadStringField(Source, TEXT("role")),
      DataAdapters::ReadStringField(Source, TEXT("prefix")),
      DataAdapters::ReadStringField(Source, TEXT("index")),
      DataAdapters::ReadStringField(Source, TEXT("depth")),
      DataAdapters::ReadStringField(Source, TEXT("name")),
      DataAdapters::ReadStringField(Source, TEXT("seed")),
      DataAdapters::ReadStringField(Source, TEXT("solved")),
      DataAdapters::ReadStringField(Source, TEXT("output")),
  };
}

/** User Story: As deterministic Maze generation, I need numeric constants decoded from the authored data boundary. @fn inline FMazeNumbers ReadMazeNumbers(const TSharedRef<FJsonObject> &Source) */
inline FMazeNumbers ReadMazeNumbers(const TSharedRef<FJsonObject> &Source) {
  return {
      DataAdapters::ReadNumberField(Source, TEXT("zero")),
      DataAdapters::ReadNumberField(Source, TEXT("one")),
      DataAdapters::ReadNumberField(Source, TEXT("mainPathEndpointCount")),
      static_cast<uint32>(
          DataAdapters::ReadNumberField(Source, TEXT("prngIncrement"))),
      DataAdapters::ReadNumberField(Source, TEXT("prngFirstShift")),
      DataAdapters::ReadNumberField(Source, TEXT("prngSecondShift")),
      static_cast<uint32>(
          DataAdapters::ReadNumberField(Source, TEXT("prngMultiplier"))),
      DataAdapters::ReadNumberField(Source, TEXT("prngThirdShift")),
      DataAdapters::ReadDoubleField(Source, TEXT("uint32Divisor")),
  };
}

/** User Story: As the shared normal and Ghost Maze mechanic, I need one immutable authored definition for topology, commands, scoring, and transcript formats. @fn inline const FMazeConfig &MazeConfig() */
inline const FMazeConfig &MazeConfig() {
  static const FMazeConfig Data = []() {
    const DataAdapters::FSettingsSource Source =
        DataAdapters::SettingsSource(TEXT("harness/maze.json"));
    const TSharedRef<FJsonObject> Root = Source.Root;
    FMazeConfig Value;
    Value.Group = DataAdapters::ReadStringField(Root, TEXT("group"));
    Value.Header = DataAdapters::ReadStringField(Root, TEXT("header"));
    Value.Footer = DataAdapters::ReadStringField(Root, TEXT("footer"));
    Value.TurnLimit = DataAdapters::ReadNumberField(Root, TEXT("turnLimit"));
    Value.DeadEndCount = DataAdapters::ReadNumberField(Root, TEXT("deadEndCount"));
    Value.DeadEndDepth = DataAdapters::ReadNumberField(Root, TEXT("deadEndDepth"));
    Value.MinRealChoice = DataAdapters::ReadNumberField(Root, TEXT("minRealChoice"));
    Value.MainPathMin = DataAdapters::ReadNumberField(Root, TEXT("mainPathMin"));
    Value.MainPathMax = DataAdapters::ReadNumberField(Root, TEXT("mainPathMax"));
    Value.NameAttribute = DataAdapters::ReadStringField(Root, TEXT("nameAttribute"));
    Value.RoleAttribute = DataAdapters::ReadStringField(Root, TEXT("roleAttribute"));
    Value.PersonaAttributes = DataAdapters::ReadStringArrayField(Root, TEXT("personaAttributes"));
    Value.SceneAttribute = DataAdapters::ReadStringField(Root, TEXT("sceneAttribute"));
    Value.SceneMemoryType = DataAdapters::ReadStringField(Root, TEXT("sceneMemoryType"));
    Value.GoalMemoryType = DataAdapters::ReadStringField(Root, TEXT("goalMemoryType"));
    Value.Directions = DataAdapters::ReadStringArrayField(Root, TEXT("directions"));
    Value.Opposites = DataAdapters::ReadStringMap(
        DataAdapters::ReadObjectField(Source, TEXT("opposites")));
    Value.StartId = DataAdapters::ReadStringField(Root, TEXT("startId"));
    Value.ExitId = DataAdapters::ReadStringField(Root, TEXT("exitId"));
    Value.CellPrefix = DataAdapters::ReadStringField(Root, TEXT("cellPrefix"));
    Value.DeadEndPrefix = DataAdapters::ReadStringField(Root, TEXT("deadEndPrefix"));
    Value.CellIdFormat = DataAdapters::ReadStringField(Root, TEXT("cellIdFormat"));
    Value.DeadEndIdFormat = DataAdapters::ReadStringField(Root, TEXT("deadEndIdFormat"));
    Value.SceneText = DataAdapters::ReadStringField(Root, TEXT("sceneText"));
    Value.TurnPrompt = DataAdapters::ReadStringField(Root, TEXT("turnPrompt"));
    Value.Commands = ReadMazeCommands(DataAdapters::ReadObjectField(Source, TEXT("commands")));
    Value.ContextQuote = DataAdapters::ReadStringField(Root, TEXT("contextQuote"));
    Value.EmptyContext = DataAdapters::ReadStringField(Root, TEXT("emptyContext"));
    Value.ContextSeparator = DataAdapters::ReadStringField(Root, TEXT("contextSeparator"));
    Value.PersonaSeparator = DataAdapters::ReadStringField(Root, TEXT("personaSeparator"));
    Value.DisplayNameFormat = DataAdapters::ReadStringField(Root, TEXT("displayNameFormat"));
    Value.ValueField = DataAdapters::ReadStringField(Root, TEXT("valueField"));
    Value.IdPattern = DataAdapters::ReadStringField(Root, TEXT("idPattern"));
    Value.OptionSeparator = DataAdapters::ReadStringField(Root, TEXT("optionSeparator"));
    Value.DisplayOptionSeparator = DataAdapters::ReadStringField(Root, TEXT("displayOptionSeparator"));
    Value.DeadEndNote = DataAdapters::ReadStringField(Root, TEXT("deadEndNote"));
#define FORBOCAI_READ_MAZE_STRING(Field, Key)                             \
  Value.Field = DataAdapters::ReadStringField(Root, TEXT(Key));
    FORBOCAI_READ_MAZE_STRING(QueryFormat, FORBOCAI_SDKCLI_AUTHORED_STRINGV0324D6CA02E2)
    FORBOCAI_READ_MAZE_STRING(WardFormat, FORBOCAI_SDKCLI_AUTHORED_STRINGVF8A24CCFFD53)
    FORBOCAI_READ_MAZE_STRING(ReasonFormat, FORBOCAI_SDKCLI_AUTHORED_STRINGV6B05278B3273)
    FORBOCAI_READ_MAZE_STRING(ThoughtFormat, FORBOCAI_SDKCLI_AUTHORED_STRINGVB6264B8D70F5)
    FORBOCAI_READ_MAZE_STRING(DecisionQualityFormat, FORBOCAI_SDKCLI_AUTHORED_STRINGV37F4724BBA23)
    FORBOCAI_READ_MAZE_STRING(NoRealChoiceNote, FORBOCAI_SDKCLI_AUTHORED_STRINGV076C83DC97E0)
    FORBOCAI_READ_MAZE_STRING(MovedFormat, FORBOCAI_SDKCLI_AUTHORED_STRINGVAFFF567CBF95)
    FORBOCAI_READ_MAZE_STRING(BlockedFormat, FORBOCAI_SDKCLI_AUTHORED_STRINGV48D3BCD9BE9F)
    FORBOCAI_READ_MAZE_STRING(UnparsedFormat, FORBOCAI_SDKCLI_AUTHORED_STRINGV8BED521FB982)
    FORBOCAI_READ_MAZE_STRING(SolvedFormat, FORBOCAI_SDKCLI_AUTHORED_STRINGV27395894CDD4)
    FORBOCAI_READ_MAZE_STRING(FailedFormat, FORBOCAI_SDKCLI_AUTHORED_STRINGVEB8159D75BBA)
    FORBOCAI_READ_MAZE_STRING(CommandFailed, FORBOCAI_SDKCLI_AUTHORED_STRINGVCB652FBD772D)
    FORBOCAI_READ_MAZE_STRING(GeneratedValueMissing, FORBOCAI_SDKCLI_AUTHORED_STRINGVDA9CD1076CC7)
    FORBOCAI_READ_MAZE_STRING(CreatedIdMissing, FORBOCAI_SDKCLI_AUTHORED_STRINGV6607664A8A70)
    FORBOCAI_READ_MAZE_STRING(LogWriteFailed, FORBOCAI_SDKCLI_AUTHORED_STRINGVC655F7EDDC97)
    FORBOCAI_READ_MAZE_STRING(LayoutHeader, FORBOCAI_SDKCLI_AUTHORED_STRINGVFE3975DAEAF2)
    FORBOCAI_READ_MAZE_STRING(LayoutEntry, FORBOCAI_SDKCLI_AUTHORED_STRINGV0AF2C5E2ABF6)
    FORBOCAI_READ_MAZE_STRING(EdgeFormat, FORBOCAI_SDKCLI_AUTHORED_STRINGV02A8B8A36FC0)
    FORBOCAI_READ_MAZE_STRING(RunHeader, FORBOCAI_SDKCLI_AUTHORED_STRINGVEDB0FCED1802)
    FORBOCAI_READ_MAZE_STRING(ExplorerFormat, FORBOCAI_SDKCLI_AUTHORED_STRINGV04709074AEAB)
    FORBOCAI_READ_MAZE_STRING(RunSummaryFormat, FORBOCAI_SDKCLI_AUTHORED_STRINGV8FAE086AEF5A)
    FORBOCAI_READ_MAZE_STRING(BlankLine, FORBOCAI_SDKCLI_AUTHORED_STRINGV0C9A8BE33F6D)
    FORBOCAI_READ_MAZE_STRING(DebugHeader, FORBOCAI_SDKCLI_AUTHORED_STRINGVDC455DABBCAE)
    FORBOCAI_READ_MAZE_STRING(DebugFooter, FORBOCAI_SDKCLI_AUTHORED_STRINGV1FDE37251C58)
    FORBOCAI_READ_MAZE_STRING(DialogueFallback, FORBOCAI_SDKCLI_AUTHORED_STRINGV0DB7D618E1E4)
    FORBOCAI_READ_MAZE_STRING(LogFileEnvKey, FORBOCAI_SDKCLI_AUTHORED_STRINGVE6BC09D8CF14)
    FORBOCAI_READ_MAZE_STRING(DefaultLogPath, FORBOCAI_SDKCLI_AUTHORED_STRINGVD8F3E341D457)
    FORBOCAI_READ_MAZE_STRING(OutputLineSeparator, FORBOCAI_SDKCLI_AUTHORED_STRINGVA75D7344BB13)
    FORBOCAI_READ_MAZE_STRING(OutputEncoding, FORBOCAI_SDKCLI_AUTHORED_STRINGVA4EC3898284C)
#undef FORBOCAI_READ_MAZE_STRING
    Value.Tokens = ReadMazeTokens(DataAdapters::ReadObjectField(Source, TEXT("tokens")));
    Value.Numbers = ReadMazeNumbers(DataAdapters::ReadObjectField(Source, TEXT("numbers")));
    return Value;
  }();
  return Data;
}

} // namespace MicroGame::Maze
