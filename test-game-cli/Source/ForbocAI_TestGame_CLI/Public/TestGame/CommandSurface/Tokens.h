#pragma once

#include "CoreMinimal.h"

namespace TestGame {
namespace CommandSurface {
namespace detail {

inline FString MapToCommandKey(const TArray<FString> &Tokens) {
  return Tokens.Num() < 2
             ? FString(TEXT("unknown"))
             : Tokens.Num() < 3
                   ? Tokens[1]
                   : Tokens[1] + FString(TEXT("_")) + Tokens[2];
}

inline TArray<FString> ExtractArgsRecursive(const TArray<FString> &Tokens,
                                            int32 Idx,
                                            TArray<FString> Acc) {
  return Idx >= Tokens.Num()
             ? Acc
             : (Tokens[Idx] == TEXT("--text")
                    ? ExtractArgsRecursive(Tokens, Idx + 1, Acc)
                    : (Acc.Add(Tokens[Idx]),
                       ExtractArgsRecursive(Tokens, Idx + 1, Acc)));
}

inline TArray<FString> ExtractArgs(const TArray<FString> &Tokens) {
  return Tokens.Num() <= 3 ? TArray<FString>()
                           : ExtractArgsRecursive(Tokens, 3, {});
}

inline TArray<FString> Tokenize(const FString &Command) {
  TArray<FString> Tokens;
  FString Current;
  struct Tokenizer {
    static void Process(const FString &Cmd, int32 Idx, bool bQuoted,
                        FString &Cur, TArray<FString> &Out) {
      return Idx >= Cmd.Len()
                 ? (!Cur.IsEmpty() ? (Out.Add(Cur), void()) : void())
                 : Cmd[Idx] == TEXT('"')
                       ? Process(Cmd, Idx + 1, !bQuoted, Cur, Out)
                       : (!bQuoted && FChar::IsWhitespace(Cmd[Idx]))
                             ? (!Cur.IsEmpty()
                                    ? (Out.Add(Cur), Cur.Reset(),
                                       Process(Cmd, Idx + 1, bQuoted, Cur, Out))
                                    : Process(Cmd, Idx + 1, bQuoted, Cur, Out))
                             : (Cur.AppendChar(Cmd[Idx]),
                                Process(Cmd, Idx + 1, bQuoted, Cur, Out));
    }
  };
  Tokenizer::Process(Command, 0, false, Current, Tokens);
  return Tokens;
}

} // namespace detail
} // namespace CommandSurface
} // namespace TestGame
