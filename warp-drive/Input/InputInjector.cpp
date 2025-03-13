#include "warpr_includes.h"
#include "InputInjector.h"
#include "Capture/FrameProvider.h"
#include "Capture/WindowSource.h"

using namespace Axodox::Infrastructure;
using namespace Warpr::Capture;
using namespace std;
using namespace winrt;
using namespace winrt::Windows::Graphics;
using namespace winrt::Windows::Graphics::Display;
using namespace winrt::Windows::UI::Input::Preview::Injection;

namespace Warpr::Input
{
  void TestInput()
  {
    auto injector = InputInjector::TryCreate();
    injector.InitializeTouchInjection(InjectedInputVisualizationMode::Indirect);
    
    InjectedInputPointerInfo pointerInfo{};
    

    InjectedInputTouchInfo touchInput{};
    
  }

  InputProvider::InputProvider(Axodox::Infrastructure::dependency_container* container)
  {
    auto frameProvider = container->resolve<FrameProvider>();
    auto windowSource = dynamic_cast<WindowSource*>(frameProvider->Source());
    if (!windowSource)
    {
      _logger.log(log_severity::warning, "Cannot use input provider with non-windowed capture windowSource");
      return;
    }

    auto source = windowSource->Source();
    auto displayInformationFactory = get_activation_factory<DisplayInformation, IDisplayInformationStaticsInterop>();

    if (holds_alternative<DisplayId>(source))
    {
      auto x = get<DisplayId>(source);
      //displayInformationFactory->GetForMonitor(reinterpret_cast<HMONITOR>(get<DisplayId>(source).Value), )

    }

  }
}