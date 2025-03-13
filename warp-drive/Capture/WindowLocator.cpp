#include "warpr_includes.h"
#include "WindowLocator.h"

using namespace std;
using namespace winrt;

namespace Warpr::Capture
{
  BOOL CALLBACK EnumWindowsCallback(
    _In_ HWND   rawWindowHandle,
    _In_ LPARAM context
  )
  {
    auto windowHandle = rawWindowHandle;

    //Skip invisible windows
    if (!IsWindowVisible(windowHandle)) return true;

    //Skip hidden windows
    TITLEBARINFO titlebarInfo{};
    titlebarInfo.cbSize = sizeof(titlebarInfo);
    GetTitleBarInfo(windowHandle, &titlebarInfo);
    if (titlebarInfo.rgstate[0] & STATE_SYSTEM_INVISIBLE) return true;

    //Skip zero / negative sized windows
    RECT rect{};
    GetWindowRect(windowHandle, &rect);
    if (rect.right - rect.left <= 0 || rect.bottom - rect.top <= 0) return true;

    //Get window title
    wchar_t title[256] = L"";
    GetWindowText(windowHandle, title, ARRAYSIZE(title));

    //Handle UWP app Windows
    wchar_t className[256] = L"";
    GetClassName(windowHandle, className, ARRAYSIZE(className));

    if (wcscmp(className, L"ApplicationFrameWindow") == 0)
    {
      auto appWindowHandle = FindWindowEx(windowHandle, nullptr, L"Windows.UI.Core.CoreWindow", nullptr);
      /*if (!appWindowHandle)
      {
        appWindowHandle = FindWindowEx(windowHandle, nullptr, nullptr, nullptr);
      }*/

      if (appWindowHandle)
      {
        if (!IsWindowVisible(appWindowHandle)) return true;
        windowHandle = appWindowHandle;
      }
      else
      {
        //Suspended app
        return true;
      }
    }

    //Get PID
    DWORD pid;
    GetWindowThreadProcessId(windowHandle, &pid);

    //Open process
    handle process{ OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, false, pid) };
    if (!process) return true;

    //Get package family name for UWP apps
    wchar_t package[256] = L"";
    UINT32 packageLength = ARRAYSIZE(package);
    GetPackageFullName(process.get(), &packageLength, package);

    //Get application path for regular apps
    wchar_t path[MAX_PATH] = L"";
    DWORD pathLenght = ARRAYSIZE(path);
    check_bool(QueryFullProcessImageName(process.get(), 0, path, &pathLenght));

    auto windows = (std::vector<WindowInfo>*)context;

    WindowInfo windowInfo{};
    windowInfo.Handle = rawWindowHandle;
    windowInfo.Title = title;
    windowInfo.Id = wcslen(package) == 0 ? PathFindFileName(path) : package;
    windowInfo.Pid = pid;
    windows->push_back(move(windowInfo));

    return true;
  }

  std::vector<WindowInfo> WindowLocator::GetWindows() const
  {
    std::vector<WindowInfo> windows;
    EnumWindows(&EnumWindowsCallback, uintptr_t(&windows));
    return windows;
  }

  void* WindowLocator::GetWindow(const std::wstring& id)
  {
    lock_guard<mutex> lock(_mutex);
    auto& handle = _knownWindows[id];
    if (!handle || !IsWindow(HWND(handle)))
    {
      handle = FindWindow(id);
    }
    return handle;
  }

  void* WindowLocator::FindWindow(const std::wstring& id) const
  {
    if (id.empty()) return nullptr;

    auto windows = GetWindows();
    auto it = find_if(windows.begin(), windows.end(), [&](const WindowInfo& info) {
      return info.Id == id;
      });

    if (it != windows.end())
    {
      return it->Handle;
    }
    else
    {
      return nullptr;
    }
  }

  void* WindowLocator::GetMainWindow() const
  {
    auto id = GetCurrentProcessId();

    auto windows = GetWindows();
    auto it = find_if(windows.begin(), windows.end(), [&](const WindowInfo& info) {
      return id == info.Pid;
      });

    if (it != windows.end())
    {
      return it->Handle;
    }
    else
    {
      return nullptr;
    }
  }
}