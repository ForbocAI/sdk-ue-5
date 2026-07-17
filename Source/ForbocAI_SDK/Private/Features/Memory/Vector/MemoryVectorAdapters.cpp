#include "Features/Memory/Vector/MemoryVectorAdapters.h"

#include "Core/fp.hpp"
#include "Features/Memory/Configuration/MemoryConfigurationAdapters.h"
#include "Internationalization/Regex.h"

namespace {

struct FSimilarityComponents {
  float Dot;
  float NormA;
  float NormB;
};

/** User Story: As SDK-owned vector memory, I need deterministic unsigned hash mixing so every supported runtime assigns features to the same buckets. @fn uint32 mixHash(uint32 Hash, uint32 Value) */
uint32 mixHash(uint32 Hash, uint32 Value) {
  return (Hash ^ Value) * MemoryConfiguration::memoryData().Vector.HashPrime;
}

/** User Story: As SDK-owned vector memory, I need text hashed recursively so embedding behavior remains pure and reproducible. @fn uint32 hashText(const FString &Text, int32 Index, uint32 Hash) */
uint32 hashText(const FString &Text, int32 Index, uint32 Hash) {
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
  return Index >= Text.Len()
             ? Hash
             : hashText(Text, Index + Data.Iteration.Step,
                        mixHash(Hash, static_cast<uint32>(Text[Index])));
}

/** User Story: As SDK-owned vector memory, I need regex matches collected recursively so TS and UE segment recall text with the same authored grammar. @fn TArray<FString> collectTokens(FRegexMatcher &Matcher, const TArray<FString> &Tokens) */
TArray<FString> collectTokens(FRegexMatcher &Matcher,
                              const TArray<FString> &Tokens) {
  return !Matcher.FindNext()
             ? Tokens
             : collectTokens(
                   Matcher,
                   func::append_value<FString>(
                       Tokens, Matcher.GetCaptureGroup(
                                   MemoryConfiguration::memoryData()
                                   .Iteration.InitialIndex)));
}

/** User Story: As semantic memory recall, I need common discourse tokens removed recursively so formatting requests cannot dominate subject similarity. @fn TArray<FString> filterContentTokens(const TArray<FString> &Tokens, int32 Index, const TArray<FString> &ContentTokens) */
TArray<FString> filterContentTokens(const TArray<FString> &Tokens, int32 Index,
                                    const TArray<FString> &ContentTokens) {
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
  return Index >= Tokens.Num()
             ? ContentTokens
             : filterContentTokens(
                   Tokens, Index + Data.Iteration.Step,
                   Data.Vector.StopWords.Contains(Tokens[Index])
                       ? ContentTokens
                       : func::append_value<FString>(ContentTokens,
                                                     Tokens[Index]));
}

/** User Story: As SDK-owned vector memory, I need an authored token grammar so TS and UE segment recall text identically. @fn TArray<FString> tokenize(const FString &Text) */
TArray<FString> tokenize(const FString &Text) {
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
  const FRegexPattern Pattern(
      Data.Vector.TokenPattern);
  FRegexMatcher Matcher(Pattern, Text.ToLower());
  return filterContentTokens(
      collectTokens(Matcher, TArray<FString>()), Data.Iteration.InitialIndex,
      TArray<FString>());
}

/** User Story: As SDK-owned vector memory, I need every lexical feature namespaced through one authored separator so TS and UE hash identical keys. @fn FString featureKey(const FString &Prefix, const TArray<FString> &Values) */
FString featureKey(const FString &Prefix, const TArray<FString> &Values) {
  const FString &Separator =
      MemoryConfiguration::memoryData().Vector.FeatureSeparator;
  return Prefix + Separator + FString::Join(Values, *Separator);
}

/** User Story: As semantic memory recall, I need morphology-tolerant character features collected recursively so natural word variants remain comparable. @fn TArray<FString> collectCharacterNgrams(const FString &Token, int32 Index, const TArray<FString> &Ngrams) */
TArray<FString> collectCharacterNgrams(const FString &Token, int32 Index,
                                       const TArray<FString> &Ngrams) {
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
  return Index + Data.Vector.CharacterNgramSize > Token.Len()
             ? Ngrams
             : collectCharacterNgrams(
                   Token, Index + Data.Iteration.Step,
                   func::append_value<FString>(
                       Ngrams,
                       Token.Mid(Index, Data.Vector.CharacterNgramSize)));
}

/** User Story: As semantic memory recall, I need character features omitted for undersized tokens and derived through one pure boundary otherwise. @fn TArray<FString> characterNgrams(const FString &Token) */
TArray<FString> characterNgrams(const FString &Token) {
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
  return Token.Len() < Data.Vector.MinimumCharacterTokenLength
             ? TArray<FString>()
             : collectCharacterNgrams(Token, Data.Iteration.InitialIndex,
                                      TArray<FString>());
}

/** User Story: As SDK-owned vector memory, I need immutable signed feature updates so embedding construction stays functional. @fn TArray<float> addFeature(const TArray<float> &Vector, uint32 Hash, float Weight) */
TArray<float> addFeature(const TArray<float> &Vector, uint32 Hash,
                         float Weight) {
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
  TArray<float> Next = Vector;
  const int32 Index = static_cast<int32>(
      Hash % static_cast<uint32>(Data.Vector.Dimension));
  const float Sign = (Hash & Data.Vector.SignMask) == Data.Vector.EvenRemainder
                         ? Data.Vector.PositiveWeight
                         : Data.Vector.NegativeWeight;
  Next[Index] += Sign * Weight;
  return Next;
}

/** User Story: As semantic memory recall, I need character features folded without mutation so morphology tolerance stays deterministic across runtimes. @fn TArray<float> addCharacterFeatures(const TArray<FString> &Ngrams, int32 Index, const TArray<float> &Vector) */
TArray<float> addCharacterFeatures(const TArray<FString> &Ngrams, int32 Index,
                                   const TArray<float> &Vector) {
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
  return Index >= Ngrams.Num()
             ? Vector
             : addCharacterFeatures(
                   Ngrams, Index + Data.Iteration.Step,
                   addFeature(
                       Vector,
                       hashText(featureKey(Data.Vector.CharacterPrefix,
                                           {Ngrams[Index]}),
                                Data.Iteration.InitialIndex,
                                Data.Vector.HashSeed),
                       Data.Vector.CharacterNgramWeight));
}

/** User Story: As semantic memory recall, I need position-independent lexical features so a fact remains retrievable when the same subject appears later in a question. @fn TArray<float> embedTokens(const TArray<FString> &Tokens, int32 Index, const TArray<float> &Vector) */
TArray<float> embedTokens(const TArray<FString> &Tokens, int32 Index,
                          const TArray<float> &Vector) {
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
  return Index >= Tokens.Num()
             ? Vector
             : embedTokens(
                   Tokens, Index + Data.Iteration.Step,
                   addCharacterFeatures(
                       characterNgrams(Tokens[Index]),
                       Data.Iteration.InitialIndex,
                       addFeature(
                           Vector,
                           hashText(featureKey(Data.Vector.UnigramPrefix,
                                               {Tokens[Index]}),
                                    Data.Iteration.InitialIndex,
                                    Data.Vector.HashSeed),
                           Data.Vector.UnigramWeight)));
}

/** User Story: As semantic memory recall, I need adjacent content words represented without absolute positions so local meaning improves without making word order brittle. @fn TArray<float> embedBigrams(const TArray<FString> &Tokens, int32 Index, const TArray<float> &Vector) */
TArray<float> embedBigrams(const TArray<FString> &Tokens, int32 Index,
                           const TArray<float> &Vector) {
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
  return Index + Data.Iteration.Step >= Tokens.Num()
             ? Vector
             : embedBigrams(
                   Tokens, Index + Data.Iteration.Step,
                   addFeature(
                       Vector,
                       hashText(
                           featureKey(Data.Vector.BigramPrefix,
                                      {Tokens[Index],
                                       Tokens[Index + Data.Iteration.Step]}),
                           Data.Iteration.InitialIndex, Data.Vector.HashSeed),
                       Data.Vector.BigramWeight));
}

/** User Story: As SDK-owned vector memory, I need vector magnitude derived recursively so normalization remains a pure transformation. @fn float sumSquares(const TArray<float> &Vector, int32 Index, float Sum) */
float sumSquares(const TArray<float> &Vector, int32 Index, float Sum) {
  return Index >= Vector.Num()
             ? Sum
             : sumSquares(
                   Vector,
                   Index + MemoryConfiguration::memoryData().Iteration.Step,
                   Sum + Vector[Index] * Vector[Index]);
}

/** User Story: As SDK-owned vector memory, I need L2-normalized features so cosine comparisons are consistent across storage engines. @fn TArray<float> normalize(const TArray<float> &Vector, int32 Index, float Norm, const TArray<float> &Normalized) */
TArray<float> normalize(const TArray<float> &Vector, int32 Index, float Norm,
                        const TArray<float> &Normalized) {
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
  return Index >= Vector.Num()
             ? Normalized
             : normalize(
                   Vector, Index + Data.Iteration.Step, Norm,
                   func::append_value<float>(
                       Normalized,
                       Norm > Data.Defaults.Similarity
                           ? Vector[Index] / Norm
                           : Data.Defaults.Similarity));
}

/** User Story: As SDK-owned vector memory, I need vector allocation driven by authored dimensions so storage schemas and embeddings cannot drift. @fn TArray<float> emptyVector() */
TArray<float> emptyVector() {
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
  TArray<float> Vector;
  Vector.Init(Data.Defaults.Similarity, Data.Vector.Dimension);
  return Vector;
}

/**
 * User Story: As memory recall, I need cosine components accumulated functionally so every engine ranks records with one shared implementation.
 * @fn FSimilarityComponents computeSimilarityComponents( const TArray<float> &A, const TArray<float> &B, int32 Index, int32 Length, const FSimilarityComponents &Components)
 */
FSimilarityComponents computeSimilarityComponents(
    const TArray<float> &A, const TArray<float> &B, int32 Index, int32 Length,
    const FSimilarityComponents &Components) {
  return Index >= Length
             ? Components
             : computeSimilarityComponents(
                   A, B,
                   Index + MemoryConfiguration::memoryData().Iteration.Step,
                   Length,
                   {Components.Dot + A[Index] * B[Index],
                    Components.NormA + A[Index] * A[Index],
                    Components.NormB + B[Index] * B[Index]});
}

} // namespace

namespace MemoryVectorAdapters {

/** User Story: As SDK-owned memory, I need deterministic model-free embeddings so the API can own the SLM while each SDK owns vector memory. @fn TArray<float> embed(const FString &Text) */
TArray<float> embed(const FString &Text) {
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
  const TArray<FString> Tokens = tokenize(Text);
  const TArray<float> Features = embedBigrams(
      Tokens, Data.Iteration.InitialIndex,
      embedTokens(Tokens, Data.Iteration.InitialIndex, emptyVector()));
  const float Norm = FMath::Sqrt(sumSquares(
      Features, Data.Iteration.InitialIndex, Data.Defaults.Similarity));
  return normalize(Features, Data.Iteration.InitialIndex, Norm,
                   TArray<float>());
}

/** User Story: As memory recall, I need one cosine selector shared by all engines so ranking semantics cannot diverge. @fn float cosineSimilarity(const TArray<float> &A, const TArray<float> &B) */
float cosineSimilarity(const TArray<float> &A, const TArray<float> &B) {
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
  const int32 Length = FMath::Min(A.Num(), B.Num());
  return Length == Data.Iteration.InitialIndex
             ? Data.Defaults.Similarity
             : [&]() {
                 const FSimilarityComponents Components =
                     computeSimilarityComponents(
                         A, B, Data.Iteration.InitialIndex, Length,
                         {Data.Defaults.Similarity,
                          Data.Defaults.Similarity,
                          Data.Defaults.Similarity});
                 const float Denominator =
                     FMath::Sqrt(Components.NormA) *
                     FMath::Sqrt(Components.NormB);
                 return Denominator > Data.Defaults.Similarity
                            ? Components.Dot / Denominator
                            : Data.Defaults.Similarity;
               }();
}

} // namespace MemoryVectorAdapters
