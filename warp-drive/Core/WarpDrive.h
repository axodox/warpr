#pragma once
#include "WarpConfiguration.h"

namespace Warpr
{
  struct WARP_DRIVE_API WarpDrive
  {
    inline static const Axodox::Infrastructure::logger _logger{ "WarpDrive" };

  public:
    WarpDrive(const WarpConfiguration& configuration = {});
    ~WarpDrive();

  private:
    Axodox::Infrastructure::dependency_container _container;
  };
}