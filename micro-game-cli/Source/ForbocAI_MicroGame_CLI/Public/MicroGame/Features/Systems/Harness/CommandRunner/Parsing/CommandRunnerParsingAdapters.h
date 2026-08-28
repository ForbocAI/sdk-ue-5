#pragma once

#include "Core/fp.hpp"
#include "MicroGame/Features/Systems/Harness/CommandRunner/Configuration/ConfigurationAdapters.h"

namespace MicroGame::CommandRunner::Parsing {

namespace detail {

/** User Story: As a CLI transport adapter, I need public CLI tokens collected recursively while harness-only text markers remain outside SDK values. @fn inline TArray<FString> ExtractCliTokensRecursive(const TArray<FString> &Tokens, int32 Index, TArray<FString> Acc) */
inline TArray<FString>
ExtractCliTokensRecursive(const TArray<FString> &Tokens, int32 Index,
                          TArray<FString> Acc) {
  const FCommandRunnerData &Data = CommandRunnerData();
  return Index >= Tokens.Num()
             ? Acc
             : ExtractCliTokensRecursive(
                   Tokens, Index + Data.limits.nextIndex,
                   Tokens[Index] == Data.syntax.textOption
                       ? MoveTemp(Acc)
                       : func::append_value<FString>(MoveTemp(Acc),
                                                     Tokens[Index]));
}

/** User Story: As a command parser, I need authored quote characters recognized consistently across SDK execution and evidence projection. @fn inline bool IsQuoteCharacter(const TCHAR Character) */
inline bool IsQuoteCharacter(const TCHAR Character) {
  const FCommandRunnerQuotes &Quotes = CommandRunnerData().quotes;
  const int32 FirstCharacterIndex =
      CommandRunnerData().limits.firstTokenIndex;
  return Character == Quotes.doubleQuote[FirstCharacterIndex] ||
         Character == Quotes.singleQuote[FirstCharacterIndex];
}

/** User Story: As a command parser, I need quoted input tokenized recursively so spaces inside prompts remain intact. @fn inline TArray<FString> TokenizeRecursive(const FString &Command, int32 Index, TCHAR QuoteCharacter, FString Current, TArray<FString> Tokens) */
inline TArray<FString> TokenizeRecursive(const FString &Command, int32 Index,
                                         TCHAR QuoteCharacter,
                                         FString Current,
                                         TArray<FString> Tokens) {
  const FCommandRunnerData &Data = CommandRunnerData();
  return Index >= Command.Len()
             ? (Current.IsEmpty()
                    ? Tokens
                    : func::append_value<FString>(MoveTemp(Tokens), Current))
         : QuoteCharacter != TCHAR() && Command[Index] == QuoteCharacter
             ? TokenizeRecursive(Command, Index + Data.limits.nextIndex,
                                 TCHAR(), MoveTemp(Current), MoveTemp(Tokens))
         : QuoteCharacter == TCHAR() && IsQuoteCharacter(Command[Index])
             ? TokenizeRecursive(Command, Index + Data.limits.nextIndex,
                                 Command[Index], MoveTemp(Current),
                                 MoveTemp(Tokens))
         : QuoteCharacter == TCHAR() && FChar::IsWhitespace(Command[Index])
             ? TokenizeRecursive(
                   Command, Index + Data.limits.nextIndex, TCHAR(), FString(),
                   Current.IsEmpty()
                       ? MoveTemp(Tokens)
                       : func::append_value<FString>(MoveTemp(Tokens), Current))
             : TokenizeRecursive(
                   Command, Index + Data.limits.nextIndex, QuoteCharacter,
                   Current + FString::Chr(Command[Index]), MoveTemp(Tokens));
}

/** User Story: As a command evidence reader, I need an option value selected recursively from tokenized SDK CLI input. @fn inline func::Maybe<FString> FindCommandOptionValue(const TArray<FString> &Tokens, const FString &Option, int32 Index) */
inline func::Maybe<FString>
FindCommandOptionValue(const TArray<FString> &Tokens, const FString &Option,
                       int32 Index) {
  const int32 NextIndex = CommandRunnerData().limits.nextIndex;
  return Index >= Tokens.Num()
             ? func::nothing<FString>()
         : Tokens[Index] == Option && Index + NextIndex < Tokens.Num()
             ? func::just(Tokens[Index + NextIndex])
             : FindCommandOptionValue(Tokens, Option, Index + NextIndex);
}

} // namespace detail

/** User Story: As an SDK CLI runner, I need only the executable root removed before the SDK-owned CLI catalog resolves the complete invocation. @fn inline TArray<FString> ExtractCliTokens(const TArray<FString> &Tokens) */
inline TArray<FString> ExtractCliTokens(const TArray<FString> &Tokens) {
  const FCommandRunnerData &Data = CommandRunnerData();
  return Tokens.Num() <= Data.limits.domainTokenIndex
             ? TArray<FString>()
             : detail::ExtractCliTokensRecursive(
                   Tokens, Data.limits.domainTokenIndex, TArray<FString>());
}

/** User Story: As an SDK CLI runner, I need one tokenizer shared by execution and transcript evidence selectors. @fn inline TArray<FString> Tokenize(const FString &Command) */
inline TArray<FString> Tokenize(const FString &Command) {
  return detail::TokenizeRecursive(
      Command, CommandRunnerData().limits.firstTokenIndex, TCHAR(), FString(),
      TArray<FString>());
}

/** User Story: As a transcript selector, I need quoted command options decoded through the same parser used by SDK CLI execution. @fn inline func::Maybe<FString> ReadCommandOption(const FString &Command, const FString &Option) */
inline func::Maybe<FString> ReadCommandOption(const FString &Command,
                                              const FString &Option) {
  return detail::FindCommandOptionValue(
      Tokenize(Command), Option, CommandRunnerData().limits.firstTokenIndex);
}

} // namespace MicroGame::CommandRunner::Parsing
