#include "warpr_includes.h"
#include "Core/WarpDrive.h"
#include "Capture/WindowSource.h"

using namespace Warpr;
using namespace Warpr::Capture;
using namespace std;
using namespace winrt;
using namespace winrt::Windows::Graphics::Display;

int main()
{
  check_bool(SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2));

  WarpConfiguration configuration{
    .FrameSource = make_unique<WindowSourceDescription>(DisplayServices::FindAll().front())
  };

  WarpDrive drive{ configuration };

  this_thread::sleep_for(10000s);
  return 0;
}
