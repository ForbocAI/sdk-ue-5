#pragma once

#include "Core/fp.hpp"
namespace Errors {

/** User Story: As a features errors consumer, I need to invoke is http error html through a stable signature so the features errors workflow remains explicit and composable. @fn inline bool isHttpErrorHtml(const FString &Message) */
inline bool isHttpErrorHtml(const FString &Message) {
  return Message.Contains(TEXT("<html"), ESearchCase::IgnoreCase) ||
         Message.Contains(TEXT("<!doctype html"), ESearchCase::IgnoreCase);
}

/** User Story: As a features errors consumer, I need to invoke strip html tags recursive through a stable signature so the features errors workflow remains explicit and composable. @fn inline FString stripHtmlTagsRecursive(const FString &Message, int32 Index, bool bInTag, bool bLastWasWhitespace, FString Out) */
inline FString stripHtmlTagsRecursive(const FString &Message, int32 Index,
                                      bool bInTag,
                                      bool bLastWasWhitespace, FString Out) {
  return Index >= Message.Len()
             ? Out.TrimStartAndEnd()
         : Message[Index] == TEXT('<')
             ? stripHtmlTagsRecursive(Message, Index + 1, true,
                                      bLastWasWhitespace, MoveTemp(Out))
         : Message[Index] == TEXT('>')
             ? stripHtmlTagsRecursive(Message, Index + 1, false,
                                      bLastWasWhitespace, MoveTemp(Out))
         : bInTag
             ? stripHtmlTagsRecursive(Message, Index + 1, true,
                                      bLastWasWhitespace, MoveTemp(Out))
         : FChar::IsWhitespace(Message[Index])
             ? stripHtmlTagsRecursive(
                   Message, Index + 1, false, true,
                   bLastWasWhitespace
                       ? MoveTemp(Out)
                       : (Out.AppendChar(TEXT(' ')), MoveTemp(Out)))
             : (Out.AppendChar(Message[Index]),
                stripHtmlTagsRecursive(Message, Index + 1, false, false,
                                       MoveTemp(Out)));
}

/** User Story: As a features errors consumer, I need to invoke strip html tags through a stable signature so the features errors workflow remains explicit and composable. @fn inline FString stripHtmlTags(const FString &Message) */
inline FString stripHtmlTags(const FString &Message) {
  return stripHtmlTagsRecursive(Message, 0, false, false, FString());
}

/** User Story: As a features errors consumer, I need to invoke read html title through a stable signature so the features errors workflow remains explicit and composable. @fn inline FString readHtmlTitle(const FString &Message) */
inline FString readHtmlTitle(const FString &Message) {
  const int32 TitleStart = Message.Find(TEXT("<title"), ESearchCase::IgnoreCase);
  return TitleStart == INDEX_NONE
             ? FString()
             : [&]() {
                 const int32 TitleOpenEnd =
                     Message.Find(TEXT(">"), ESearchCase::CaseSensitive,
                                  ESearchDir::FromStart, TitleStart);
                 return TitleOpenEnd == INDEX_NONE
                            ? FString()
                            : [&]() {
                                const int32 TitleClose = Message.Find(
                                    TEXT("</title>"), ESearchCase::IgnoreCase,
                                    ESearchDir::FromStart, TitleOpenEnd + 1);
                                return TitleClose == INDEX_NONE ||
                                               TitleClose <= TitleOpenEnd
                                           ? FString()
                                           : stripHtmlTags(Message.Mid(
                                                 TitleOpenEnd + 1,
                                                 TitleClose - TitleOpenEnd -
                                                     1));
                              }();
               }();
}

/** User Story: As a features errors consumer, I need to invoke parse leading status code through a stable signature so the features errors workflow remains explicit and composable. @fn inline int32 parseLeadingStatusCode(const FString &Message) */
inline int32 parseLeadingStatusCode(const FString &Message) {
  return Message.Len() >= 3 && FChar::IsDigit(Message[0]) &&
                 FChar::IsDigit(Message[1]) && FChar::IsDigit(Message[2])
             ? FCString::Atoi(*Message.Left(3))
             : 0;
}

/** User Story: As a features errors consumer, I need to invoke remove leading status code through a stable signature so the features errors workflow remains explicit and composable. @fn inline FString removeLeadingStatusCode(const FString &Message, int32 StatusCode) */
inline FString removeLeadingStatusCode(const FString &Message, int32 StatusCode) {
  return StatusCode <= 0 ||
                 !Message.StartsWith(FString::FromInt(StatusCode))
             ? Message
             : [&]() {
                 FString Reason = Message.Mid(3).TrimStartAndEnd();
                 Reason.RemoveFromStart(TEXT(":"));
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
             ? (StatusCode > 0 && !Message.IsEmpty()
                    ? FString::Printf(TEXT("HTTP %d: %s"), StatusCode,
                                      *Message)
                    : Message)
             : [&]() {
                 const FString HtmlTitle = readHtmlTitle(Message);
                 const FString Title =
                     HtmlTitle.IsEmpty()
                         ? stripHtmlTags(Message.Left(512))
                         : HtmlTitle;
                 const int32 EffectiveStatus =
                     StatusCode > 0 ? StatusCode
                                    : parseLeadingStatusCode(Title);
                 const FString Reason =
                     removeLeadingStatusCode(Title, EffectiveStatus);
                 return EffectiveStatus > 0
                            ? FString::Printf(
                                  TEXT("HTTP %d%s%s"), EffectiveStatus,
                                  Reason.IsEmpty() ? TEXT("") : TEXT(": "),
                                  *Reason)
                            : FString::Printf(
                                  TEXT("HTTP error%s%s"),
                                  Reason.IsEmpty() ? TEXT("") : TEXT(": "),
                                  *Reason);
               }();
}

/**
 * Returns a non-empty thunk error message for FString-based failures.
 * User Story: As thunk adapters, I need a guaranteed message string so CLI and
 * UI surfaces never render empty failure text.
 * @fn inline FString extractThunkErrorMessage(const FString &Message, const FString &DefaultMessage = TEXT("Request failed"))
 */
inline FString extractThunkErrorMessage(const FString &Message,
                                        const FString &DefaultMessage =
                                            TEXT("Request failed")) {
  return Message.IsEmpty() ? DefaultMessage : summarizeHttpError(0, Message);
}

/**
 * Returns a non-empty thunk error message for std::string-based failures.
 * User Story: As thunk adapters, I need a guaranteed message string even for
 * native std::string errors so reporting stays consistent across layers.
 * @fn inline FString extractThunkErrorMessage(const std::string &Message, const FString &DefaultMessage = TEXT("Request failed"))
 */
inline FString extractThunkErrorMessage(const std::string &Message,
                                        const FString &DefaultMessage =
                                            TEXT("Request failed")) {
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
  return (ApiKey.IsEmpty() && ApiUrl.Contains(TEXT("api.forboc.ai")))
             ? func::just<FString>(
                   TEXT("Missing API key. Set FORBOCAI_API_KEY (or run `forboc "
                        "config set apiKey <key>`) for production API calls."))
             : func::nothing<FString>();
}

} // namespace Errors
