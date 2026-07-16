#pragma once

#include "Core/rtk.hpp"

namespace APISlice::SoulStorageEndpoint {

/** User Story: As a local RTK Query endpoint, I need SDK storage AsyncResults represented through the same query success and custom-error lifecycle as transport requests. @fn template <typename Result> inline func::AsyncResult<rtk::QueryReturnValue<Result>> storageQueryResult(const func::AsyncResult<Result> &Operation) */
template <typename Result>
inline func::AsyncResult<rtk::QueryReturnValue<Result>>
storageQueryResult(const func::AsyncResult<Result> &Operation) {
  return func::AsyncResult<rtk::QueryReturnValue<Result>>::create(
      [Operation](
          std::function<void(rtk::QueryReturnValue<Result>)> Resolve,
          std::function<void(std::string)> Reject) {
        (void)Reject;
        Operation
            .then([Resolve](const Result &Value) {
              Resolve(rtk::QueryReturnValue<Result>::success(Value));
            })
            .catch_([Resolve](const std::string &Error) {
              Resolve(rtk::QueryReturnValue<Result>::failure(
                  rtk::FetchBaseQueryError::customError(
                      FString(UTF8_TO_TCHAR(Error.c_str())))));
            })
            .execute();
      });
}

} // namespace APISlice::SoulStorageEndpoint
