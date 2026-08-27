/**
 * Live API Tests for Handler Loop
 * User Story: As protocol orchestration, I need tests that run against the live API so that integration regressions
 * are caught.
 */

#include "Core/rtk.hpp"
#include "Components/AuthoredValues/AuthoredValuesTypes.h"
#include "CoreMinimal.h"
#include "Systems/API/APIApi.h"
#include "Systems/API/Serialization/APISerializationAdapters.h"
#include "Systems/Config/ConfigThunks.h"
#include "HAL/PlatformProcess.h"
#include "Misc/AutomationTest.h"
#include "Store.h"
#include "Tests/Integration/Protocol/HandlersLive/HandlersLiveTestAdapters.h"

using namespace rtk;

struct FProcessLiveTestState {
  bool bCompleted = false;
  bool bSuccess = false;
  FString Error;
  FNPCProcessTape Tape;
  int32 Step = FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA;
  TSharedPtr<rtk::EnhancedStore<FRuntimeState>> Store;
};

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(
    FProcessLiveStepWait, TSharedPtr<FProcessLiveTestState>, State, int32, PollCount);
/**
 * User Story: As a developer, I need Update to fulfill its role in the module.
 * @fn bool FProcessLiveStepWait::Update()
 */
bool FProcessLiveStepWait::Update() {
  const int32 MaxPolls = FORBOCAI_SDK_AUTHORED_NUMBERV07C0796E1646;  // ~15s at 50ms

  if (State->Step == FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA) {
      State->Step = FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4;
      State->Store = MakeShared<rtk::EnhancedStore<FRuntimeState>>(createRuntimeStore());
      Ops::hydrateRuntimeConfig(*State->Store);
      
      FNPCProcessRequest Req;
      Req.Tape = FNPCProcessTape();
      Req.Tape.Observation = TEXT(FORBOCAI_SDK_AUTHORED_STRINGV1AD8029617BF);
      
      auto Dispatch = [this](const rtk::AnyAction &A) { return State->Store->dispatch(A); };
      auto GetState = [this]() -> const FRuntimeState & { return State->Store->getState(); };
      
      APISlice::Endpoints::postNpcProcess(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV4EC4BCA7807D), Req)(Dispatch, GetState)
          .then([this](const FNPCProcessResponse &R) {
              if (R.Instruction.Type == ENPCInstructionType::IdentifyActor) {
                  State->Step = FORBOCAI_SDK_AUTHORED_NUMBERV6AC392A47561;
                  State->Tape = R.Tape;
              } else {
                  State->bCompleted = true;
                  State->bSuccess = false;
                  State->Error = ProtocolHandlersLiveTestAdapters::ExpectedInstructionError(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV9648236DCEE6), R.Instruction.Type);
              }
          })
          .catch_([this](std::string E) {
              State->bCompleted = true;
              State->bSuccess = false;
              State->Error = FString(UTF8_TO_TCHAR(E.c_str()));
          })
          .execute();
      return false;
  }
  
  if (State->Step == FORBOCAI_SDK_AUTHORED_NUMBERV6AC392A47561) {
      State->Step = FORBOCAI_SDK_AUTHORED_NUMBERV32732DCF7787;
      FNPCProcessRequest Req;
      Req.Tape = State->Tape;
      
      TSharedPtr<FJsonObject> ActorRes = MakeShared<FJsonObject>();
      ActorRes->SetStringField(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV1A8A149929AF), TEXT(FORBOCAI_SDK_AUTHORED_STRINGV7E97DFAAA192));
      TSharedPtr<FJsonObject> ActorObj = MakeShared<FJsonObject>();
      ActorObj->SetStringField(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVDBA420FAB4C4), TEXT(FORBOCAI_SDK_AUTHORED_STRINGV4EC4BCA7807D));
      ActorObj->SetObjectField(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV13EF85B46077), MakeShared<FJsonObject>());
      ActorRes->SetObjectField(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVE279DD109F1C), ActorObj);
      Req.PreviousResult = ProtocolHandlersLiveTestAdapters::LastResultJson(ActorRes);
      Req.bHasPreviousResult = true;
      
      auto Dispatch = [this](const rtk::AnyAction &A) { return State->Store->dispatch(A); };
      auto GetState = [this]() -> const FRuntimeState & { return State->Store->getState(); };
      
      APISlice::Endpoints::postNpcProcess(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV4EC4BCA7807D), Req)(Dispatch, GetState)
          .then([this](const FNPCProcessResponse &R) {
              if (R.Instruction.Type == ENPCInstructionType::QueryVector) {
                  State->Step = FORBOCAI_SDK_AUTHORED_NUMBERV17F0DE0DDF4A;
                  State->Tape = R.Tape;
              } else {
                  State->bCompleted = true;
                  State->bSuccess = false;
                  State->Error = ProtocolHandlersLiveTestAdapters::ExpectedInstructionError(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVA1B9D4047349), R.Instruction.Type);
              }
          })
          .catch_([this](std::string E) {
              State->bCompleted = true;
              State->bSuccess = false;
              State->Error = FString(UTF8_TO_TCHAR(E.c_str()));
          })
          .execute();
      return false;
  }
  
  if (State->Step == FORBOCAI_SDK_AUTHORED_NUMBERV17F0DE0DDF4A) {
      State->Step = FORBOCAI_SDK_AUTHORED_NUMBERV2B61CCD40B6E;
      FNPCProcessRequest Req;
      Req.Tape = State->Tape;
      
      TSharedPtr<FJsonObject> QueryRes = MakeShared<FJsonObject>();
      QueryRes->SetStringField(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV1A8A149929AF), TEXT(FORBOCAI_SDK_AUTHORED_STRINGV8A54476A166E));
      QueryRes->SetArrayField(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV3A0AFD601C3E), TArray<TSharedPtr<FJsonValue>>());
      Req.PreviousResult = ProtocolHandlersLiveTestAdapters::LastResultJson(QueryRes);
      Req.bHasPreviousResult = true;
      
      auto Dispatch = [this](const rtk::AnyAction &A) { return State->Store->dispatch(A); };
      auto GetState = [this]() -> const FRuntimeState & { return State->Store->getState(); };
      
      APISlice::Endpoints::postNpcProcess(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV4EC4BCA7807D), Req)(Dispatch, GetState)
          .then([this](const FNPCProcessResponse &R) {
              if (R.Instruction.Type == ENPCInstructionType::Decision) {
                  State->Step = FORBOCAI_SDK_AUTHORED_NUMBERVFB04E74C9EE8;
                  State->Tape = R.Tape;
              } else {
                  State->bCompleted = true;
                  State->bSuccess = false;
                  State->Error = ProtocolHandlersLiveTestAdapters::ExpectedInstructionError(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV6B5B6E4359EF), R.Instruction.Type);
              }
          })
          .catch_([this](std::string E) {
              State->bCompleted = true;
              State->bSuccess = false;
              State->Error = FString(UTF8_TO_TCHAR(E.c_str()));
          })
          .execute();
      return false;
  }
  
  if (State->Step == FORBOCAI_SDK_AUTHORED_NUMBERVFB04E74C9EE8) {
      State->Step = FORBOCAI_SDK_AUTHORED_NUMBERV5AE13912B613;
      FNPCProcessRequest Req;
      Req.Tape = State->Tape;
      
      TSharedPtr<FJsonObject> DecRes = MakeShared<FJsonObject>();
      DecRes->SetStringField(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV1A8A149929AF), TEXT(FORBOCAI_SDK_AUTHORED_STRINGV6B5B6E4359EF));
      TSharedPtr<FJsonObject> IntentObj = MakeShared<FJsonObject>();
      IntentObj->SetStringField(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVACBF9830D250), TEXT(FORBOCAI_SDK_AUTHORED_STRINGV804CBB7F5896));
      IntentObj->SetStringField(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVD48EE2807F09), TEXT(FORBOCAI_SDK_AUTHORED_STRINGV6F3BFC9554E3));
      DecRes->SetObjectField(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV88C75E1D04B7), IntentObj);
      Req.PreviousResult = ProtocolHandlersLiveTestAdapters::LastResultJson(DecRes);
      Req.bHasPreviousResult = true;
      
      auto Dispatch = [this](const rtk::AnyAction &A) { return State->Store->dispatch(A); };
      auto GetState = [this]() -> const FRuntimeState & { return State->Store->getState(); };
      
      APISlice::Endpoints::postNpcProcess(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV4EC4BCA7807D), Req)(Dispatch, GetState)
          .then([this](const FNPCProcessResponse &R) {
              if (R.Instruction.Type == ENPCInstructionType::Reasoning) {
                  State->Step = FORBOCAI_SDK_AUTHORED_NUMBERVAC6FC173B700;
                  State->Tape = R.Tape;
                  
                  if (!State->Tape.bDecisionCompleted) {
                      State->bCompleted = true;
                      State->bSuccess = false;
                      State->Error = TEXT(FORBOCAI_SDK_AUTHORED_STRINGV1EE2E3B1CDF0);
                  }
              } else {
                  State->bCompleted = true;
                  State->bSuccess = false;
                  State->Error = ProtocolHandlersLiveTestAdapters::ExpectedInstructionError(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV128754029BBF), R.Instruction.Type);
              }
          })
          .catch_([this](std::string E) {
              State->bCompleted = true;
              State->bSuccess = false;
              State->Error = FString(UTF8_TO_TCHAR(E.c_str()));
          })
          .execute();
      return false;
  }

  if (State->Step == FORBOCAI_SDK_AUTHORED_NUMBERVAC6FC173B700) {
      State->Step = FORBOCAI_SDK_AUTHORED_NUMBERV339CB6A0C3A6;
      FNPCProcessRequest Req;
      Req.Tape = State->Tape;
      
      TSharedPtr<FJsonObject> RsgRes = MakeShared<FJsonObject>();
      RsgRes->SetStringField(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV1A8A149929AF), TEXT(FORBOCAI_SDK_AUTHORED_STRINGV128754029BBF));
      TSharedPtr<FJsonObject> OutObj = MakeShared<FJsonObject>();
      OutObj->SetStringField(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV5D053553F968), TEXT(FORBOCAI_SDK_AUTHORED_STRINGVAAA7A4C4D393));
      OutObj->SetStringField(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV5EA0BB4F9359), TEXT(FORBOCAI_SDK_AUTHORED_STRINGV1DB372C7D79C));
      RsgRes->SetObjectField(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV2BDC92E440EF), OutObj);
      Req.PreviousResult = ProtocolHandlersLiveTestAdapters::LastResultJson(RsgRes);
      Req.bHasPreviousResult = true;
      
      auto Dispatch = [this](const rtk::AnyAction &A) { return State->Store->dispatch(A); };
      auto GetState = [this]() -> const FRuntimeState & { return State->Store->getState(); };
      
      APISlice::Endpoints::postNpcProcess(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV4EC4BCA7807D), Req)(Dispatch, GetState)
          .then([this](const FNPCProcessResponse &R) {
              if (R.Instruction.Type == ENPCInstructionType::Finalize) {
                  State->Step = FORBOCAI_SDK_AUTHORED_NUMBERV14FE7CBC615F;
                  State->Tape = R.Tape;
                  
                  if (!State->Tape.bReasoningCompleted) {
                      State->Error = TEXT(FORBOCAI_SDK_AUTHORED_STRINGVD3F2A4083BF1);
                      State->bSuccess = false;
                  } else if (State->Tape.ReasoningOutput.ResponseText.IsEmpty()) {
                      State->Error = TEXT(FORBOCAI_SDK_AUTHORED_STRINGV0A870C8E98DD);
                      State->bSuccess = false;
                  } else {
                      State->bSuccess = true;
                  }
                  State->bCompleted = true;
              } else {
                  State->bCompleted = true;
                  State->bSuccess = false;
                  State->Error = ProtocolHandlersLiveTestAdapters::ExpectedInstructionError(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV1EFFA2D43442), R.Instruction.Type);
              }
          })
          .catch_([this](std::string E) {
              State->bCompleted = true;
              State->bSuccess = false;
              State->Error = FString(UTF8_TO_TCHAR(E.c_str()));
          })
          .execute();
      return false;
  }

  if (State->bCompleted)
    return true;
  if (++PollCount >= MaxPolls) {
    State->bCompleted = true;
    State->bSuccess = false;
    State->Error = TEXT(FORBOCAI_SDK_AUTHORED_STRINGV97A50E774DE5);
    return true;
  }
  FPlatformProcess::Sleep(FORBOCAI_SDK_AUTHORED_NUMBERV4B582E8E76C5);
  return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FProtocolHandlersLiveTest,
    FORBOCAI_SDK_AUTHORED_STRINGVB78C9ED577EB,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 * @fn bool FProtocolHandlersLiveTest::RunTest(const FString &Parameters)
 */
bool FProtocolHandlersLiveTest::RunTest(const FString &Parameters) {
  const FString ApiKey =
      FPlatformMisc::GetEnvironmentVariable(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV5F0C966E7042));
  if (ApiKey.IsEmpty()) {
    AddError(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVB5A7F99F39DA));
    return true;
  }

  auto State = MakeShared<FProcessLiveTestState>();
  ADD_LATENT_AUTOMATION_COMMAND(FProcessLiveStepWait(State, FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA));

  ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand(
      [this, State]() {
        TestTrue(FORBOCAI_SDK_AUTHORED_STRINGVC9EEF0644BA0, State->bCompleted);
        if (!State->bCompleted)
          return;
        TestTrue(FString::Printf(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV5017B6B5505A), *State->Error), State->bSuccess);
      },
      FORBOCAI_SDK_AUTHORED_NUMBERVEC53E6A2E194));

  return true;
}
