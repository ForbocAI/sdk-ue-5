#pragma once

#include "ForbocAI_SDK/Public/Systems/Data/DataAdapters.h"
#include "Components/Testing/API/Process/ProcessTypes.h"

namespace Testing::API::Process {

/** User Story: As an NPC process codec test, I need one immutable fixture loaded from the mirrored TS data so both SDKs validate identical success and rejection cases. @fn inline const FProcessCodecFixture &ProcessCodecFixture() */
inline const FProcessCodecFixture &ProcessCodecFixture() {
  static const FProcessCodecFixture Fixture = []() {
    const DataAdapters::FSettingsSource Source =
        DataAdapters::SettingsSource(TEXT("ForbocAI_SDK"),
                                     TEXT("Data/tests/api/npc.json"));
    const TSharedRef<FJsonObject> Labels =
        DataAdapters::ReadObjectField(Source, TEXT("labels"));
    return FProcessCodecFixture{
        DataAdapters::SerializeObject(DataAdapters::ReadObjectField(
            Source, TEXT("queryVectorResponse"))),
        DataAdapters::SerializeObject(DataAdapters::ReadObjectField(
            Source, TEXT("queryVectorExpected"))),
        DataAdapters::SerializeObject(
            DataAdapters::ReadObjectField(Source, TEXT("finalizeResponse"))),
        DataAdapters::SerializeObject(
            DataAdapters::ReadObjectField(Source, TEXT("finalizeExpected"))),
        DataAdapters::SerializeObject(DataAdapters::ReadObjectField(
            Source, TEXT("malformedActionResponse"))),
        DataAdapters::SerializeObject(DataAdapters::ReadObjectField(
            Source, TEXT("malformedMemoryResponse"))),
        DataAdapters::SerializeObject(DataAdapters::ReadObjectField(
            Source, TEXT("malformedActorResponse"))),
        DataAdapters::SerializeObject(DataAdapters::ReadObjectField(
            Source, TEXT("unknownInstructionResponse"))),
        {
            DataAdapters::ReadStringField(Labels, TEXT("queryVector")),
            DataAdapters::ReadStringField(Labels, TEXT("finalize")),
            DataAdapters::ReadStringField(Labels, TEXT("rejectsAction")),
            DataAdapters::ReadStringField(Labels, TEXT("rejectsMemory")),
            DataAdapters::ReadStringField(Labels, TEXT("rejectsActor")),
            DataAdapters::ReadStringField(Labels,
                                          TEXT("rejectsInstruction")),
        }};
  }();
  return Fixture;
}

} // namespace Testing::API::Process
