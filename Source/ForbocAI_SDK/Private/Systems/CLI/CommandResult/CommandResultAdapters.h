#pragma once

#include "Components/CLI/CommandRouting/CommandRoutingTypes.h"

namespace CLIOps::CommandResult {

using Result = CLIOps::CommandRouting::Result;

/** User Story: As CLI presentation, I need semantic success text lifted into the shared command result. @fn inline Result Success(const FString &Message) */
inline Result Success(const FString &Message) {
  return Result::Success(TCHAR_TO_UTF8(*Message));
}

/** User Story: As CLI validation, I need semantic failure text lifted into the shared command result. @fn inline Result Failure(const FString &Message) */
inline Result Failure(const FString &Message) {
  return Result::Failure(TCHAR_TO_UTF8(*Message));
}

} // namespace CLIOps::CommandResult
