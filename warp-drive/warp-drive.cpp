#include "warpr_includes.h"
#include "Core/WarpDrive.h"
#include "Capture/WindowSource.h"

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

  this_thread::sleep_for(10000s);
  return 0;
}
