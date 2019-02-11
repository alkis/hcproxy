// Copyright 2018 Roman Perepelitsa
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef ROMKATV_HCPROXY_LOGGING_H_
#define ROMKATV_HCPROXY_LOGGING_H_

#include <cstdlib>
#include <memory>
#include <optional>
#include <ostream>
#include <sstream>

#ifndef HCP_MIN_LOG_LVL
#define HCP_MIN_LOG_LVL INFO
#endif

#define LOG(severity) LOG_I(severity)

#define LOG_I(severity)                                                                    \
  (::hcproxy::internal_logging::severity < ::hcproxy::internal_logging::HCP_MIN_LOG_LVL)   \
      ? static_cast<void>(0)                                                               \
      : ::hcproxy::internal_logging::Assignable<::hcproxy::internal_logging::severity>() = \
            ::hcproxy::internal_logging::LogStream(__FILE__, __LINE__,                     \
                                                   ::hcproxy::internal_logging::severity)  \
                .ref()

namespace hcproxy {

namespace internal_logging {

enum Severity {
  INFO = 0,
  WARN = 1,
  ERROR = 2,
  FATAL = 3,
};

template <Severity>
struct Assignable {
  template <class T>
  void operator=(const T&) const {}
};

template <>
struct Assignable<FATAL> {
  ~Assignable() __attribute__((noreturn)) { std::abort(); }

  template <class T>
  void operator=(const T&) const {}
};

class LogStream {
 public:
  LogStream(const char* file, int line, Severity severity);
  LogStream(LogStream&&) = delete;
  ~LogStream();

  LogStream& ref() { return *this; }
  std::ostream& strm() { return *strm_; }
  int stashed_errno() const { return errno_; }

 private:
  int errno_;
  const char* file_;
  int line_;
  Severity severity_;
  std::unique_ptr<std::ostringstream> strm_;
};

template <class T>
LogStream& operator<<(LogStream& strm, const T& val) {
  strm.strm() << val;
  return strm;
}

inline LogStream& operator<<(LogStream& strm, std::ostream& (*manip)(std::ostream&)) {
  strm.strm() << manip;
  return strm;
}

struct Errno {
  int err;
};

std::ostream& operator<<(std::ostream& strm, Errno e);

struct StashedErrno {};

inline LogStream& operator<<(LogStream& strm, StashedErrno) {
  return strm << Errno{strm.stashed_errno()};
}

}  // namespace internal_logging

inline internal_logging::Errno Errno(int err) { return {err}; }
inline internal_logging::StashedErrno Errno() { return {}; }

}  // namespace hcproxy

#endif  // ROMKATV_HCPROXY_LOGGING_H_
