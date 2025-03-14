#include "warpr_includes.h"
#include "InputSinkFactory.h"

using namespace std;

namespace Warpr::Input
{
  InputSinkFactory::InputSinkFactory(std::function<std::unique_ptr<InputSink>()>&& factory) :
    Factory(move(factory))
  { }

  InputSinkKind InputSinkFactory::Type() const
  {
    return InputSinkKind::Factory;
  }
}