#include "warpr_includes.h"
#include "Core/WarpDrive.h"
#include "Capture/WindowSource.h"

using namespace Warpr;
using namespace std;

int main()
{
  using namespace Warpr::Capture;

  WindowSourceDescription description;
  description.Source = winrt::Windows::Graphics::Display::DisplayServices::FindAll().front();
  auto frameSource = FrameSource::Create(description);
  Sleep(100000);

  return 0;

  WarpDrive drive;

  this_thread::sleep_for(10000s);
  return 0;
}
