#pragma once

#include "TestGame/Features/Systems/Quality/QualityAdapters.h"

namespace TestGame {

/** User Story: As a quality harness, I need authored command data translated into the canonical command contract once. @fn inline FCommandSpec toQualityCommandSpec(const FQualityCommandData &Command) */
inline FCommandSpec
toQualityCommandSpec(const FQualityCommandData &Command) {
  return {Command.Group, Command.Command, Command.ExpectedRoutes, {}};
}

/** User Story: As a quality harness, I need deployed model identity collected through the public status command. @fn inline FCommandSpec qualityMetadataCommand() */
inline FCommandSpec qualityMetadataCommand() {
  return toQualityCommandSpec(qualityData().MetadataCommand);
}

/** User Story: As a generic NPC evaluator, I need neutral runtime persona and memory setup performed through public CLI commands. @fn inline TArray<FCommandSpec> qualitySetupCommands() */
inline TArray<FCommandSpec> qualitySetupCommands() {
  return func::map_array<FQualityCommandData, FCommandSpec>(
      qualityData().SetupCommands, toQualityCommandSpec);
}

/** User Story: As a model evaluator, I need the complete neutral probe contract selected from authored data. @fn inline TArray<FQualityProbe> qualityProbeDefinitions() */
inline TArray<FQualityProbe> qualityProbeDefinitions() {
  return qualityData().Probes;
}

/** User Story: As a quality harness, I need every model probe routed through the same NPC CLI command surface as a game. @fn inline FCommandSpec qualityProbeCommand(const FQualityProbe &Probe) */
inline FCommandSpec qualityProbeCommand(const FQualityProbe &Probe) {
  FQualityCommandData Command = qualityData().ProbeCommand;
  Command.Command = Probe.Command;
  return toQualityCommandSpec(Command);
}

} // namespace TestGame
