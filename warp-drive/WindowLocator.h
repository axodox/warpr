#pragma once

#include <string>
#include <vector>
#include <map>
#include <mutex>

#undef FindWindow

namespace warper
{
  struct WindowInfo
  {
    uint32_t Pid;
    void* Handle;
    std::wstring Id;
    std::wstring Title;
  };

  class WindowLocator
  {
  public:
    std::vector<WindowInfo> GetWindows() const;

    void* GetWindow(const std::wstring& id);
    void* FindWindow(const std::wstring& id) const;

    void* GetMainWindow() const;

  private:
    std::mutex _mutex;
    std::map<std::wstring, void*> _knownWindows;
  };
}