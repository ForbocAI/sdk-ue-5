#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Systems/Memory/Local/MemoryLocalThunks.h"
#include "Systems/Config/ConfigThunks.h"
#include "Systems/Protocol/ProtocolThunks.h"
#include "ProcessNPCTestTypes.h"

namespace ProcessNPCTestAdapters {

/** User Story: As a integration protocol npc consumer, I need to invoke complete through a stable signature so the integration protocol npc workflow remains explicit and composable. @fn inline void Complete(const TSharedPtr<FProcessNPCTestState> &State, bool bSuccess, const FAgentResponse &Response, const FString &Error) */
inline void Complete(const TSharedPtr<FProcessNPCTestState> &State,
                     bool bSuccess, const FAgentResponse &Response,
                     const FString &Error) {
  State->bCompleted = true;
  State->bSuccess = bSuccess;
  State->Response = Response;
  State->Error = Error;
}

/** User Story: As a integration protocol npc consumer, I need to invoke cleanup and complete through a stable signature so the integration protocol npc workflow remains explicit and composable. @fn inline void CleanupAndComplete( const TSharedPtr<FProcessNPCTestState> &State, bool bSuccess, const FAgentResponse &Response, const FString &Error) */
inline void CleanupAndComplete(
    const TSharedPtr<FProcessNPCTestState> &State, bool bSuccess,
    const FAgentResponse &Response, const FString &Error) {
  State->Store->dispatch(rtk::clearNodeMemoryThunk(State->DatabaseName))
      .then([State, bSuccess, Response, Error](rtk::FEmptyPayload) {
        Complete(State, bSuccess, Response, Error);
      })
      .catch_([State, Error](std::string CleanupError) {
        const FString Detail = UTF8_TO_TCHAR(CleanupError.c_str());
        Complete(State, false, FAgentResponse(),
                 FString::Printf(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV5BF11EC15CA2),
                                 *Error, *Detail));
      })
      .execute();
}

/** User Story: As a integration protocol npc consumer, I need to invoke start through a stable signature so the integration protocol npc workflow remains explicit and composable. @fn inline void Start(const TSharedPtr<FProcessNPCTestState> &State, const FProcessNPCTestParams &Params) */
inline void Start(const TSharedPtr<FProcessNPCTestState> &State,
                  const FProcessNPCTestParams &Params) {
  State->Store =
      MakeShared<rtk::EnhancedStore<FRuntimeState>>(createRuntimeStore());
  Ops::hydrateRuntimeConfig(*State->Store);
  State->DatabaseName = Params.NpcId;

  State->Store->dispatch(rtk::initNodeMemoryThunk(State->DatabaseName))
      .then([State, Params](
                const MemoryLocalTypes::FMemoryDatabasePaths &) {
        FProtocolProcessInput Input =
            ProtocolProcess::ProcessInput(Params.NpcId, Params.Input);
        Input.ContextJson = FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVF0320EEDEC6F) TEXT(FORBOCAI_SDK_AUTHORED_STRINGV3E3D7634AB68));
        Input.Persona = Params.Persona;
        State->Store
            ->dispatch(rtk::processNPC(
                Input,
                rtk::LocalProtocolHandlerContext(Params.NpcId)))
            .then([State](const FAgentResponse &Response) {
              CleanupAndComplete(State, true, Response, FString());
            })
            .catch_([State](std::string Error) {
              CleanupAndComplete(
                  State, false, FAgentResponse(),
                  FString(UTF8_TO_TCHAR(Error.c_str())));
            })
            .execute();
      })
      .catch_([State](std::string Error) {
        CleanupAndComplete(State, false, FAgentResponse(),
                           FString(UTF8_TO_TCHAR(Error.c_str())));
      })
      .execute();
}

} // namespace ProcessNPCTestAdapters
