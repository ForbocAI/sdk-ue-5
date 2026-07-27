#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Core/fp.hpp"
namespace Errors {

/** User Story: As a features errors consumer, I need to invoke is http error html through a stable signature so the features errors workflow remains explicit and composable. @fn inline bool isHttpErrorHtml(const FString &Message) */
inline bool isHttpErrorHtml(const FString &Message) {
  return Message.Contains(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV6369144AB26B), ESearchCase::IgnoreCase) ||
         Message.Contains(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV5F9551A2B49C), ESearchCase::IgnoreCase);
}

/** User Story: As a features errors consumer, I need to invoke strip html tags recursive through a stable signature so the features errors workflow remains explicit and composable. @fn inline FString stripHtmlTagsRecursive(const FString &Message, int32 Index, bool bInTag, bool bLastWasWhitespace, FString Out) */
inline FString stripHtmlTagsRecursive(const FString &Message, int32 Index,
                                      bool bInTag,
                                      bool bLastWasWhitespace, FString Out) {
  return Index >= Message.Len()
             ? Out.TrimStartAndEnd()
         : Message[Index] == TEXT('<')
             ? stripHtmlTagsRecursive(Message, Index + FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4, true,
                                      bLastWasWhitespace, MoveTemp(Out))
         : Message[Index] == TEXT('>')
             ? stripHtmlTagsRecursive(Message, Index + FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4, false,
                                      bLastWasWhitespace, MoveTemp(Out))
         : bInTag
             ? stripHtmlTagsRecursive(Message, Index + FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4, true,
                                      bLastWasWhitespace, MoveTemp(Out))
         : FChar::IsWhitespace(Message[Index])
             ? stripHtmlTagsRecursive(
                   Message, Index + FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4, false, true,
                   bLastWasWhitespace
                       ? MoveTemp(Out)
                       : (Out.AppendChar(TEXT(' ')), MoveTemp(Out)))
             : (Out.AppendChar(Message[Index]),
                stripHtmlTagsRecursive(Message, Index + FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4, false, false,
                                       MoveTemp(Out)));
}

/** User Story: As a features errors consumer, I need to invoke strip html tags through a stable signature so the features errors workflow remains explicit and composable. @fn inline FString stripHtmlTags(const FString &Message) */
inline FString stripHtmlTags(const FString &Message) {
  return stripHtmlTagsRecursive(Message, FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA, false, false, FString());
}

/** User Story: As a features errors consumer, I need to invoke read html title through a stable signature so the features errors workflow remains explicit and composable. @fn inline FString readHtmlTitle(const FString &Message) */
inline FString readHtmlTitle(const FString &Message) {
  const int32 TitleStart = Message.Find(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVBAC1B5A629A3), ESearchCase::IgnoreCase);
  return TitleStart == INDEX_NONE
             ? FString()
             : [&]() {
                 const int32 TitleOpenEnd =
                     Message.Find(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV2A3A705CFCA2), ESearchCase::CaseSensitive,
                                  ESearchDir::FromStart, TitleStart);
                 return TitleOpenEnd == INDEX_NONE
                            ? FString()
                            : [&]() {
                                const int32 TitleClose = Message.Find(
                                    TEXT(FORBOCAI_SDK_AUTHORED_STRINGVF896013AA044), ESearchCase::IgnoreCase,
                                    ESearchDir::FromStart, TitleOpenEnd + FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4);
                                return TitleClose == INDEX_NONE ||
                                               TitleClose <= TitleOpenEnd
                                           ? FString()
                                           : stripHtmlTags(Message.Mid(
                                                 TitleOpenEnd + FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4,
                                                 TitleClose - TitleOpenEnd -
                                                     FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4));
                              }();
               }();
}

/** User Story: As a features errors consumer, I need to invoke parse leading status code through a stable signature so the features errors workflow remains explicit and composable. @fn inline int32 parseLeadingStatusCode(const FString &Message) */
inline int32 parseLeadingStatusCode(const FString &Message) {
  return Message.Len() >= FORBOCAI_SDK_AUTHORED_NUMBERV32732DCF7787 && FChar::IsDigit(Message[FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA]) &&
                 FChar::IsDigit(Message[FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4]) && FChar::IsDigit(Message[FORBOCAI_SDK_AUTHORED_NUMBERV6AC392A47561])
             ? FCString::Atoi(*Message.Left(FORBOCAI_SDK_AUTHORED_NUMBERV32732DCF7787))
             : FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA;
}

/** User Story: As a features errors consumer, I need to invoke remove leading status code through a stable signature so the features errors workflow remains explicit and composable. @fn inline FString removeLeadingStatusCode(const FString &Message, int32 StatusCode) */
inline FString removeLeadingStatusCode(const FString &Message, int32 StatusCode) {
  return StatusCode <= FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA ||
                 !Message.StartsWith(FString::FromInt(StatusCode))
             ? Message
             : [&]() {
                 FString Reason = Message.Mid(FORBOCAI_SDK_AUTHORED_NUMBERV32732DCF7787).TrimStartAndEnd();
                 Reason.RemoveFromStart(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVE512A1A85234));
                 return Reason.TrimStartAndEnd();
               }();
}

/**
 * Summarizes provider HTML error pages into stable HTTP error text.
 * User Story: As CLI and micro-game diagnostics, I need non-2xx provider pages
 * reduced to status and title so strict failures stay readable.
 * @fn inline FString summarizeHttpError(int32 StatusCode, const FString &Message)
 */
inline FString summarizeHttpError(int32 StatusCode, const FString &Message) {
  return !isHttpErrorHtml(Message)
             ? (StatusCode > FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA && !Message.IsEmpty()
                    ? FString::Printf(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVC9C45592B28C), StatusCode,
                                      *Message)
                    : Message)
             : [&]() {
                 const FString HtmlTitle = readHtmlTitle(Message);
                 const FString Title =
                     HtmlTitle.IsEmpty()
                         ? stripHtmlTags(Message.Left(FORBOCAI_SDK_AUTHORED_NUMBERV5E5E9F7B6299))
                         : HtmlTitle;
                 const int32 EffectiveStatus =
                     StatusCode > FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA ? StatusCode
                                    : parseLeadingStatusCode(Title);
                 const FString Reason =
                     removeLeadingStatusCode(Title, EffectiveStatus);
                 return EffectiveStatus > FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA
                            ? FString::Printf(
                                  TEXT(FORBOCAI_SDK_AUTHORED_STRINGV63C7F7D440F4), EffectiveStatus,
                                  Reason.IsEmpty() ? TEXT("") : TEXT(FORBOCAI_SDK_AUTHORED_STRINGVB244656FFA69),
                                  *Reason)
                            : FString::Printf(
                                  TEXT(FORBOCAI_SDK_AUTHORED_STRINGVA332E4637EAD),
                                  Reason.IsEmpty() ? TEXT("") : TEXT(FORBOCAI_SDK_AUTHORED_STRINGVB244656FFA69),
                                  *Reason);
               }();
}

/**
 * Returns a non-empty thunk error message for FString-based failures.
 * User Story: As thunk adapters, I need a guaranteed message string so CLI and
 * UI surfaces never render empty failure text.
 * @fn inline FString extractThunkErrorMessage(const FString &Message, const FString &DefaultMessage = TEXT(FORBOCAI_SDK_AUTHORED_STRINGV9432DABA6C0B))
 */
inline FString extractThunkErrorMessage(const FString &Message,
                                        const FString &DefaultMessage =
                                            TEXT(FORBOCAI_SDK_AUTHORED_STRINGV9432DABA6C0B)) {
  return Message.IsEmpty() ? DefaultMessage : summarizeHttpError(FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA, Message);
}

/**
 * Returns a non-empty thunk error message for std::string-based failures.
 * User Story: As thunk adapters, I need a guaranteed message string even for
 * native std::string errors so reporting stays consistent across layers.
 * @fn inline FString extractThunkErrorMessage(const std::string &Message, const FString &DefaultMessage = TEXT(FORBOCAI_SDK_AUTHORED_STRINGV9432DABA6C0B))
 */
inline FString extractThunkErrorMessage(const std::string &Message,
                                        const FString &DefaultMessage =
                                            TEXT(FORBOCAI_SDK_AUTHORED_STRINGV9432DABA6C0B)) {
  return Message.empty() ? DefaultMessage
                         : extractThunkErrorMessage(
                               FString(UTF8_TO_TCHAR(Message.c_str())),
                               DefaultMessage);
}

/**
 * Returns guidance when production API calls are missing an API key.
 * User Story: As SDK setup flows, I need targeted guidance for missing keys so
 * production calls fail with actionable remediation instead of a dead end.
 * @fn inline func::Maybe<FString> requireApiKeyGuidance(const FString &ApiUrl, const FString &ApiKey)
 */
inline func::Maybe<FString>
requireApiKeyGuidance(const FString &ApiUrl, const FString &ApiKey) {
  return (ApiKey.IsEmpty() && ApiUrl.Contains(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV359A57F23E0E)))
             ? func::just<FString>(
                   TEXT(FORBOCAI_SDK_AUTHORED_STRINGV082C9C7A34DB
                        FORBOCAI_SDK_AUTHORED_STRINGV996E26E55CE0))
             : func::nothing<FString>();
}

} // namespace Errors
