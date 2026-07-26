#pragma once

#include "TestGame/Features/Data/DataAdapters.h"
#include "TestGame/Features/Systems/Contract/ContractTypes.h"

namespace TestGame::Contract {

/** User Story: As a contract parser, I need wire-field names loaded once without owning API transport. @fn inline const FContractData &ContractData() */
inline const FContractData &ContractData() {
  static const FContractData Data = []() {
    const DataAdapters::FSettingsSource Source =
        DataAdapters::SettingsSource(TEXT("systems/contract.json"));
    const TSharedRef<FJsonObject> Schema =
        DataAdapters::ReadObjectField(Source, TEXT("schema"));
    FContractData Value;
#define FORBOCAI_READ_CONTRACT_SCHEMA_FIELD(Name)                        \
  Value.Schema.Name = DataAdapters::ReadStringField(Schema, TEXT(#Name));
    FORBOCAI_CONTRACT_SCHEMA_FIELDS(FORBOCAI_READ_CONTRACT_SCHEMA_FIELD)
#undef FORBOCAI_READ_CONTRACT_SCHEMA_FIELD
    return Value;
  }();
  return Data;
}

} // namespace TestGame::Contract
