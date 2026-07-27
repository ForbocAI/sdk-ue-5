#pragma once

#include "Systems/API/Endpoints/NPC/Process/Instruction/InstructionAdapters.h"
#include "Systems/API/Endpoints/NPC/Process/Tape/Decode/DecodeAdapters.h"

namespace APISlice::Detail {

/** User Story: As process endpoint callers, I need typed requests encoded through the authored request contract without duplicated field literals. @fn inline FString EncodeNpcProcessRequest(const FNPCProcessRequest &Request) */
inline FString EncodeNpcProcessRequest(const FNPCProcessRequest &Request) {
  const auto &Fields =
      NPCProcessConfiguration::processContractData().Request;
  const TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
  Root->SetObjectField(Fields.Tape, EncodeProcessTapeObject(Request.Tape));
  Request.bHasPreviousResult
      ? (JsonInterop::SetFieldFromJsonString(
             Root, Fields.Result, Request.PreviousResult, false),
         void())
      : void();
  return ToJsonString(Root);
}

/** User Story: As the SDK protocol loop, I need instruction and tape decoded together so no partial response can replace runtime state. @fn inline bool DecodeNpcProcessResponse(const FString &Json, FNPCProcessResponse &Response) */
inline bool DecodeNpcProcessResponse(const FString &Json,
                                     FNPCProcessResponse &Response) {
  const auto &Fields =
      NPCProcessConfiguration::processContractData().Response;
  TSharedPtr<FJsonObject> Root;
  const bool bValid =
      JsonInterop::ParseJsonObject(Json, Root) && Root.IsValid() &&
      Root->HasTypedField<EJson::Object>(Fields.Instruction) &&
      Root->HasTypedField<EJson::Object>(Fields.Tape);
  return !bValid
             ? false
             : [&]() {
                 FNPCProcessResponse Decoded;
                 const bool bDecoded = DecodeInstructionObject(
                                           Root->GetObjectField(
                                               Fields.Instruction),
                                           Decoded.Instruction) &&
                                       DecodeProcessTapeObject(
                                           Root->GetObjectField(Fields.Tape),
                                           Decoded.Tape);
                 return !bDecoded ? false : (Response = Decoded, true);
               }();
}

} // namespace APISlice::Detail
