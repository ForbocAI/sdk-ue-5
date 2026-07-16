#pragma once

#include "CoreMinimal.h"

namespace ForbocAI {
namespace CLI {
namespace Soul {

struct FCLISoulLimits {
  int32 EmptyItemCount;
  int32 SingleArgumentCount;
  int32 DoubleArgumentCount;
  int32 FirstArgumentIndex;
  int32 SecondArgumentIndex;
  int32 DefaultListLimit;
};

struct FCLISoulSyntax {
  FString ListSeparator;
};

struct FCLISoulMessages {
  FString ExportUsage;
  FString Exported;
  FString ImportUsage;
  FString Imported;
  FString ListEmpty;
  FString ChatUsage;
  FString ChatUser;
  FString ChatNpc;
  FString VerifyUsage;
  FString VerificationValid;
  FString VerificationInvalid;
};

struct FCLISoulState {
  FCLISoulLimits Limits;
  FCLISoulSyntax Syntax;
  FCLISoulMessages Messages;
};

} // namespace Soul
} // namespace CLI
} // namespace ForbocAI
