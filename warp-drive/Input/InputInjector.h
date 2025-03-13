#pragma once
#include "warpr_includes.h"

namespace Warpr::Input
{
  class InputProvider
  {
    inline static const Axodox::Infrastructure::logger _logger{"InputProvider"};

  public:
    InputProvider(Axodox::Infrastructure::dependency_container* container);

  private:
  };
}