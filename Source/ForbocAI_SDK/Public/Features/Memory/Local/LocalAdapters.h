#pragma once

#include "Core/SdkVectorizer.h"
#include "Features/Memory/MemorySlice.h"
#include "Misc/Paths.h"
#include "NativeStorage.h"

namespace rtk {
namespace detail {

/**
 * Returns the singleton handle backing the local sqlite memory database.
 * User Story: As node-memory thunks, I need a shared database handle so local
 * memory operations reuse one opened connection.
 */
inline Native::Sqlite::DB &NodeMemoryHandle() {
  static Native::Sqlite::DB Handle = nullptr;
  return Handle;
}

/**
 * Returns the project-local infrastructure root used by native runtimes.
 * User Story: As local runtime setup, I need one canonical infrastructure root
 * so vector assets resolve from one place.
 */
inline FString GetLocalInfrastructureDir() {
  return FPaths::ProjectDir() + TEXT("local_infrastructure/");
}

/**
 * Returns the default sqlite database path for node-backed memory.
 * User Story: As node-memory setup, I need a canonical database location so
 * local memory storage initializes predictably.
 */
inline FString DefaultNodeMemoryPath() {
  return GetLocalInfrastructureDir() + TEXT("vectors/forbocai_vectors.db");
}

/**
 * Returns the mutable storage holding the active node-memory database path.
 * User Story: As node-memory setup, I need one mutable path slot so config and
 * initialization code share the active database target.
 */
inline FString &NodeMemoryPathStorage() {
  static FString Path = DefaultNodeMemoryPath();
  return Path;
}

/**
 * Returns the current node-memory database handle.
 * User Story: As node-memory helpers, I need one place to read the active
 * database handle so storage and recall use the same connection.
 */
inline Native::Sqlite::DB EnsureNodeMemoryDatabase() {
  return NodeMemoryHandle();
}

/**
 * Builds a persisted memory item from a memory-store instruction.
 * User Story: As node-memory store thunks, I need instructions converted into
 * persisted memory items so local storage uses a complete record shape.
 */
inline FMemoryItem MakeMemoryItem(const FMemoryStoreInstruction &Instruction) {
  FMemoryItem Item;
  Item.Id = FGuid::NewGuid().ToString();
  Item.Text = Instruction.Text;
  Item.Type = Instruction.Type.IsEmpty() ? TEXT("observation")
                                         : Instruction.Type;
  Item.Importance = Instruction.Importance;
  Item.Timestamp = FDateTime::UtcNow().ToUnixTimestamp();
  Item.Embedding = ForbocAI::SDK::Vectorizer::Embed(Instruction.Text);
  return Item;
}

} // namespace detail
} // namespace rtk
