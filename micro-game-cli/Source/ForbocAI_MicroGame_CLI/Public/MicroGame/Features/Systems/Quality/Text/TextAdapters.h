#pragma once

#include "Internationalization/Regex.h"
#include "MicroGame/Features/Systems/Quality/QualityAdapters.h"

namespace MicroGame {
namespace QualityTextAdaptersDetail {

/** User Story: As a text evaluator, I need regex matches replaced recursively without mutable loop state. @fn inline FString replaceMatches(FRegexMatcher &Matcher, const FString &Value, const FString &Replacement, int32 Cursor, FString Result) */
inline FString replaceMatches(FRegexMatcher &Matcher, const FString &Value,
                              const FString &Replacement, int32 Cursor,
                              FString Result) {
  return Matcher.FindNext()
             ? replaceMatches(
                   Matcher, Value, Replacement, Matcher.GetMatchEnding(),
                   Result + Value.Mid(
                                Cursor,
                                Matcher.GetMatchBeginning() - Cursor) +
                       Replacement)
             : Result + Value.Mid(Cursor);
}

/** User Story: As a text evaluator, I need regex replacement isolated in a pure adapter boundary. @fn inline FString replacePattern(const FString &Value, const FString &Pattern, const FString &Replacement) */
inline FString replacePattern(const FString &Value, const FString &Pattern,
                              const FString &Replacement) {
  FRegexMatcher Matcher(FRegexPattern(Pattern), Value);
  return replaceMatches(Matcher, Value, Replacement, int32{}, FString{});
}

/** User Story: As a lexical evaluator, I need every regex match collected in stable source order. @fn inline void appendMatches(FRegexMatcher &Matcher, const FString &Value, TArray<FString> &Matches) */
inline void appendMatches(FRegexMatcher &Matcher, const FString &Value,
                          TArray<FString> &Matches) {
  Matcher.FindNext()
      ? (Matches.Add(Value.Mid(Matcher.GetMatchBeginning(),
                              Matcher.GetMatchEnding() -
                                  Matcher.GetMatchBeginning())),
         appendMatches(Matcher, Value, Matches))
      : void();
}

/** User Story: As a boundary evaluator, I need case-insensitive authored expressions tested consistently. @fn inline bool matchesPattern(const FString &Value, const FString &Pattern) */
inline bool matchesPattern(const FString &Value, const FString &Pattern) {
  FRegexMatcher Matcher(
      FRegexPattern(qualityData().Patterns.CaseInsensitivePrefix + Pattern),
      Value);
  return Matcher.FindNext();
}

/** User Story: As a repetition evaluator, I need every fixed-width phrase derived recursively without hidden mutable state. @fn inline TArray<FString> ngrams(const TArray<FString> &Words, int32 Index, TArray<FString> Result) */
inline TArray<FString> ngrams(const TArray<FString> &Words, int32 Index,
                              TArray<FString> Result) {
  const int32 Count = Words.Num() - qualityData().Numbers.RepetitionNgramSize +
                      qualityData().Numbers.NextIndex;
  return Index >= Count
             ? Result
             : ngrams(
                   Words, Index + qualityData().Numbers.NextIndex,
                   func::append_value<FString>(
                       MoveTemp(Result),
                       FString::Join(
                           func::slice_array<FString>(
                               Words, Index,
                               qualityData().Numbers.RepetitionNgramSize),
                           *qualityData().Output.Space)));
}

/** User Story: As a dialogue evaluator, I need the first authored prefix match selected deterministically. @fn inline func::Maybe<FString> prefixedDialogue(const TArray<FString> &Lines, int32 LineIndex, int32 PrefixIndex) */
inline func::Maybe<FString>
prefixedDialogue(const TArray<FString> &Lines, int32 LineIndex,
                 int32 PrefixIndex) {
  return LineIndex >= Lines.Num()
             ? func::nothing<FString>()
         : PrefixIndex >= qualityData().Output.DialoguePrefixes.Num()
             ? prefixedDialogue(Lines,
                                LineIndex + qualityData().Numbers.NextIndex,
                                qualityData().Numbers.InitialIndex)
         : Lines[LineIndex].StartsWith(
               qualityData().Output.DialoguePrefixes[PrefixIndex])
             ? func::just(Lines[LineIndex].RightChop(
                   qualityData().Output.DialoguePrefixes[PrefixIndex].Len())
                              .TrimStartAndEnd())
             : prefixedDialogue(
                   Lines, LineIndex,
                   PrefixIndex + qualityData().Numbers.NextIndex);
}

/** User Story: As a dialogue evaluator, I need presentation-only lines excluded from raw model evidence. @fn inline bool isIgnoredLine(const FString &Line) */
inline bool isIgnoredLine(const FString &Line) {
  return qualityData().Output.IgnoredPrefixes.ContainsByPredicate(
      [&Line](const FString &Prefix) { return Line.StartsWith(Prefix); });
}

} // namespace QualityTextAdaptersDetail

/** User Story: As a live evaluator, I need terminal decoration removed before metadata or dialogue is decoded. @fn inline FString stripQualityAnsi(const FString &Value) */
inline FString stripQualityAnsi(const FString &Value) {
  return QualityTextAdaptersDetail::replacePattern(
      Value, qualityData().Patterns.Ansi, qualityData().Output.Empty);
}

/** User Story: As a quality evaluator, I need punctuation and whitespace normalized before exact comparisons. @fn inline FString normalizeQualityText(const FString &Value) */
inline FString normalizeQualityText(const FString &Value) {
  return QualityTextAdaptersDetail::replacePattern(
             QualityTextAdaptersDetail::replacePattern(
                 Value.TrimStartAndEnd(), qualityData().Patterns.Whitespace,
                 qualityData().Output.Space),
             qualityData().Patterns.TrailingPunctuation,
             qualityData().Output.Empty)
      .ToLower();
}

/** User Story: As a quality evaluator, I need deterministic lexical tokens for semantic overlap and repetition scoring. @fn inline TArray<FString> qualityWords(const FString &Value) */
inline TArray<FString> qualityWords(const FString &Value) {
  FRegexMatcher Matcher(FRegexPattern(qualityData().Patterns.Words), Value);
  TArray<FString> Matches;
  QualityTextAdaptersDetail::appendMatches(Matcher, Value, Matches);
  return func::map_array<FString, FString>(
      Matches, [](const FString &Word) { return Word.ToLower(); });
}

/** User Story: As a model evaluator, I need deterministic semantic overlap measured under the same contract on every host. @fn inline double referenceTokenF1(const FString &Reference, const FString &Response) */
inline double referenceTokenF1(const FString &Reference,
                               const FString &Response) {
  const TArray<FString> ReferenceWords = qualityWords(Reference);
  const TArray<FString> ResponseWords = qualityWords(Response);
  return ReferenceWords.IsEmpty() || ResponseWords.IsEmpty()
             ? qualityData().Numbers.MinimumReferenceF1
             : [&]() {
                 const TMap<FString, int32> ReferenceCounts =
                     func::fold_array<FString, TMap<FString, int32>>(
                         ReferenceWords, {},
                         [](const TMap<FString, int32> &Counts,
                            const FString &Word) {
                           return func::upsert_map_value<FString, int32>(
                               Counts, Word, int32{}, [](int32 Count) {
                                 return Count +
                                        qualityData().Numbers.NextIndex;
                               });
                         });
                 const TMap<FString, int32> ResponseCounts =
                     func::fold_array<FString, TMap<FString, int32>>(
                         ResponseWords, {},
                         [](const TMap<FString, int32> &Counts,
                            const FString &Word) {
                           return func::upsert_map_value<FString, int32>(
                               Counts, Word, int32{}, [](int32 Count) {
                                 return Count +
                                        qualityData().Numbers.NextIndex;
                               });
                         });
                 TArray<FString> Keys;
                 ReferenceCounts.GetKeys(Keys);
                 const int32 Overlap = func::fold_array<FString, int32>(
                     Keys, qualityData().Numbers.EmptyCount,
                     [&ReferenceCounts, &ResponseCounts](
                         int32 Total, const FString &Key) {
                       return Total + FMath::Min(
                                          ReferenceCounts[Key],
                                          ResponseCounts.FindRef(Key));
                     });
                 const double Precision =
                     static_cast<double>(Overlap) /
                     static_cast<double>(ResponseWords.Num());
                 const double Recall =
                     static_cast<double>(Overlap) /
                     static_cast<double>(ReferenceWords.Num());
                 return Precision + Recall ==
                                qualityData().Numbers.MinimumReferenceF1
                            ? qualityData().Numbers.MinimumReferenceF1
                            : roundQualityNumber(
                                  static_cast<double>(
                                      qualityData().Numbers.SingularCount +
                                      qualityData().Numbers.SingularCount) *
                                  Precision * Recall /
                                  (Precision + Recall));
               }();
}

/** User Story: As a dialogue evaluator, I need repeated phrase density measured independently of response length. @fn inline double repeatedNgramRatio(const FString &Value) */
inline double repeatedNgramRatio(const FString &Value) {
  const TArray<FString> Ngrams = QualityTextAdaptersDetail::ngrams(
      qualityWords(Value), qualityData().Numbers.InitialIndex, {});
  return Ngrams.IsEmpty()
             ? qualityData().Numbers.MinimumReferenceF1
             : roundQualityNumber(
                   static_cast<double>(Ngrams.Num() -
                                       uniqueQualityValues(Ngrams).Num()) /
                   static_cast<double>(Ngrams.Num()));
}

/** User Story: As a character consistency evaluator, I need repeated answers compared by stable lexical agreement. @fn inline double tokenJaccard(const FString &Left, const FString &Right) */
inline double tokenJaccard(const FString &Left, const FString &Right) {
  const TArray<FString> LeftWords = uniqueQualityValues(qualityWords(Left));
  const TArray<FString> RightWords = uniqueQualityValues(qualityWords(Right));
  const TArray<FString> Union = uniqueQualityValues(
      func::concat_arrays<FString>({LeftWords, RightWords}));
  const int32 Intersection = func::filter_array<FString>(
                                 LeftWords, [&RightWords](const FString &Word) {
                                   return RightWords.Contains(Word);
                                 })
                                 .Num();
  return Union.IsEmpty()
             ? qualityData().Numbers.MinimumReferenceF1
             : roundQualityNumber(static_cast<double>(Intersection) /
                                  static_cast<double>(Union.Num()));
}

/** User Story: As a live evaluator, I need actual model dialogue extracted from both TS presentation output and UE raw CLI results. @fn inline FString extractQualityDialogue(const FString &Output) */
inline FString extractQualityDialogue(const FString &Output) {
  TArray<FString> RawLines;
  stripQualityAnsi(Output).ParseIntoArray(
      RawLines, *qualityData().Output.LineSeparator,
      qualityData().Numbers.EmptyCount != qualityData().Numbers.EmptyCount);
  const TArray<FString> Lines = func::filter_array<FString>(
      func::map_array<FString, FString>(
          RawLines,
          [](const FString &Line) { return Line.TrimStartAndEnd(); }),
      [](const FString &Line) { return !Line.IsEmpty(); });
  return func::match(
      QualityTextAdaptersDetail::prefixedDialogue(
          Lines, qualityData().Numbers.InitialIndex,
          qualityData().Numbers.InitialIndex),
      [](const FString &Dialogue) { return Dialogue; },
      [&Lines]() {
        return FString::Join(
            func::filter_array<FString>(
                Lines, [](const FString &Line) {
                  return !QualityTextAdaptersDetail::isIgnoredLine(Line);
                }),
            *qualityData().Output.LineSeparator);
      });
}

/** User Story: As a boundary evaluator, I need authored expressions applied consistently to each response. @fn inline TArray<FString> matchingQualityPatterns(const FString &Value, const TArray<FString> &Patterns) */
inline TArray<FString>
matchingQualityPatterns(const FString &Value,
                        const TArray<FString> &Patterns) {
  return func::filter_array<FString>(
      Patterns, [&Value](const FString &Pattern) {
        return QualityTextAdaptersDetail::matchesPattern(Value, Pattern);
      });
}

} // namespace MicroGame
