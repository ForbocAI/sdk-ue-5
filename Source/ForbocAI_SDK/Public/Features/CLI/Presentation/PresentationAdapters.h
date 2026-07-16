#pragma once

#include "CoreMinimal.h"
#include "Misc/OutputDevice.h"

namespace ForbocAI {
namespace CLI {
namespace Presentation {

/** User Story: As a features cli presentation consumer, I need to invoke format cli message through a stable signature so the features cli presentation workflow remains explicit and composable. @fn template <typename... Values> inline FString formatCliMessage(const FString &Template, Values &&...Arguments) */
template <typename... Values>
inline FString formatCliMessage(const FString &Template,
                                Values &&...Arguments) {
  return FString::Format(*Template,
                         {Forward<Values>(Arguments)...});
}

/** User Story: As a features cli presentation consumer, I need to invoke log cli message through a stable signature so the features cli presentation workflow remains explicit and composable. @fn inline void logCliMessage(const FString &Message) */
inline void logCliMessage(const FString &Message) {
  GLog != nullptr ? GLog->Log(*Message) : void();
}

/** User Story: As a features cli presentation consumer, I need to invoke log cli message when through a stable signature so the features cli presentation workflow remains explicit and composable. @fn inline void logCliMessageWhen(bool Predicate, const FString &Message) */
inline void logCliMessageWhen(bool Predicate, const FString &Message) {
  Predicate ? logCliMessage(Message) : void();
}

} // namespace Presentation
} // namespace CLI
} // namespace ForbocAI
