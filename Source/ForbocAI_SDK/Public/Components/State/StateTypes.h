#pragma once

#include "Components/Bridge/BridgeTypes.h"
#include "Components/CLI/CLITypes.h"
#include "Components/CLI/Presentation/PresentationTypes.h"
#include "Components/Config/ConfigTypes.h"
#include "Components/Dependencies/DependenciesTypes.h"
#include "Components/Directive/DirectiveTypes.h"
#include "Components/Ghost/State/GhostStateTypes.h"
#include "Components/Memory/MemoryTypes.h"
#include "Components/NPC/NPCTypes.h"
#include "Components/Soul/SoulTypes.h"
#include "Components/Vector/VectorTypes.h"

/**
 * User Story: As an SDK workflow, I need one typed runtime state contract so
 * feature thunks and selectors can compose without importing the root store.
 */
struct FRuntimeState {
  NPCSlice::FNPCSliceState NPCs;
  MemorySlice::FMemorySliceState Memory;
  DirectiveSlice::FDirectiveSliceState Directives;
  BridgeSlice::FBridgeSliceState Bridge;
  SoulSlice::FSoulSliceState Soul;
  GhostSlice::FGhostSliceState Ghost;
  FVectorState Vector;
  FDependenciesState Dependencies;
  ForbocAI::CLI::FCLIState CLI;
  ForbocAI::CLI::Presentation::FCLIPresentationState CLIPresentation;
  ConfigSlice::FConfigState Config;
  TMap<FString, FString> Extra;
};
