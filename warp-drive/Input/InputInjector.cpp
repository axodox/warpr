#include "warpr_includes.h"
#include "InputInjector.h"
#include <winrt/windows.ui.input.preview.injection.h>

using namespace winrt;
using namespace winrt::Windows::UI::Input::Preview::Injection;

namespace Warpr::Input
{
  void TestInput()
  {
    auto injector = InputInjector::TryCreate();
    injector.InitializeTouchInjection(InjectedInputVisualizationMode::Indirect);
    
    InjectedInputPointerInfo pointerInfo{};
    

    InjectedInputTouchInfo touchInput{};
    

    __nop();
  }
}