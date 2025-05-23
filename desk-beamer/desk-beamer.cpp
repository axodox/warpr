#include "Include\WarpDrive.h"

using namespace Axodox::Infrastructure;
using namespace Warpr;
using namespace Warpr::Capture;
using namespace std;
using namespace winrt::Windows::Graphics::Display;

int main()
{
  logger::severity(log_severity::information);

  WarpConfiguration configuration{
   .FrameSource = make_unique<WindowSourceDescription>(DisplayServices::FindAll().front()),
   .MinimumFrameInterval = 12ms
  };

  WarpDrive drive{ configuration };

  system("pause");
  return 0;
}