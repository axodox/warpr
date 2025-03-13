#include "Include\WarpDrive.h"

using namespace Warpr;
using namespace Warpr::Capture;
using namespace std;
using namespace winrt::Windows::Graphics::Display;

int main()
{
  WarpConfiguration configuration{
   .FrameSource = make_unique<WindowSourceDescription>(DisplayServices::FindAll().front())
  };

  WarpDrive drive{ configuration };

  system("pause");
  return 0;
}