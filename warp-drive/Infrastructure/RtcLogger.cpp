#include "RtcLogger.h"
#include "rtc/rtc.hpp"

using namespace Axodox::Infrastructure;
using namespace std;
using namespace rtc;

namespace {
  log_severity ToSeverity(LogLevel level)
  {
    switch (level)
    {
    case LogLevel::None:
    case LogLevel::Fatal:
      return log_severity::fatal;
    case LogLevel::Error:
      return log_severity::error;
    case LogLevel::Warning:
      return log_severity::warning;
    case LogLevel::Info:
      return log_severity::information;
    case LogLevel::Debug:
    case LogLevel::Verbose:
      return log_severity::debug;
    default:
      throw logic_error("Unexpected log level!");
    }
  }

  LogLevel ToLevel(log_severity severity)
  {
    switch (severity)
    {
    case log_severity::fatal:
      return LogLevel::Fatal;
    case log_severity::error:
      return LogLevel::Error;
    case log_severity::warning:
      return LogLevel::Warning;
    case log_severity::information:
      return LogLevel::Info;
    case log_severity::debug:
      return LogLevel::Debug;
    default:
      throw logic_error("Unexpected log severity!");
    }
  }

  void RtcLogCallback(LogLevel level, string message)
  {
    static logger logger{ "LibDataChannel" };
    logger.log(ToSeverity(level), message);
  }
}

namespace Warpr::Infrastructure
{
  void InitRtcLogger(Axodox::Infrastructure::log_severity severity)
  {
    InitLogger(ToLevel(severity), RtcLogCallback);
  }
}
