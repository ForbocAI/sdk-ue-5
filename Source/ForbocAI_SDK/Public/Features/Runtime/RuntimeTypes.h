#pragma once

#include "Features/Bridge/BridgeTypes.h"
#include "Features/CLI/CLITypes.h"
#include "Features/Dependencies/DependenciesTypes.h"
#include "Features/Directive/DirectiveTypes.h"
#include "Features/Ghost/State/StateTypes.h"
#include "Features/Memory/MemoryTypes.h"
#include "Features/NPC/NPCTypes.h"
#include "Features/Soul/SoulTypes.h"
#include "Features/Vector/VectorTypes.h"

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
  TMap<FString, FString> Extra;
};
