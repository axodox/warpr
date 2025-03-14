#pragma once
#include "warpr_includes.h"
#include "PointerInput.h"

namespace Warpr::Input
{
  enum class InputSinkKind
  {
    Unknown,
    Factory
  };

  struct WARP_DRIVE_API InputSinkDescription
  {
    virtual InputSinkKind Type() const = 0;
    virtual ~InputSinkDescription() = default;
  };

  class WARP_DRIVE_API InputSink
  {
    inline static const Axodox::Infrastructure::logger _logger{ "InputSink" };

  public:
    virtual ~InputSink() = default;

    virtual void OnPointerInput(const PointerInput& input);

    static std::unique_ptr<InputSink> Create(Axodox::Infrastructure::dependency_container* container, const InputSinkDescription* description);
  };
}