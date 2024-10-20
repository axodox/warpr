// warp-drive.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <winrt/base.h>
#include "rtc/rtc.hpp"
#include "WindowLocator.h"

using namespace std;
using namespace rtc;
using namespace winrt;
using namespace warper;

struct window_station_handle_traits
{
  using type = HWINSTA;

  static void close(type value) noexcept
  {
    CloseWindowStation(value);
  }

  static type invalid() noexcept
  {
    return reinterpret_cast<type>(-1);
  }
};

using window_station_handle = handle_type<window_station_handle_traits>;


struct desktop_handle_traits
{
  using type = HDESK;

  static void close(type value) noexcept
  {
    CloseDesktop(value);
  }

  static type invalid() noexcept
  {
    return reinterpret_cast<type>(-1);
  }
};

using desktop_handle = handle_type<desktop_handle_traits>;


void start_desktop()
{
  //InitializeTouchInjection(1, TOUCH_FEEDBACK_INDIRECT);


  /*handle logon;
  check_bool(LogonUser(L"test", nullptr, L"test", LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT, logon.put()));*/

  auto originalStation = GetProcessWindowStation();

  desktop_handle originalDesktop{ check_pointer(OpenInputDesktop(0, false, GENERIC_EXECUTE)) };
  //window_station_handle myStation{ check_pointer(CreateWindowStation(L"warpr", 0, READ_CONTROL | WRITE_DAC, nullptr))};
  desktop_handle myDesktop{ check_pointer(CreateDesktop(L"warpr", nullptr, nullptr, 0, GENERIC_ALL, nullptr)) };
  
  check_bool(SetThreadDesktop(myDesktop.get()));

  //SetCursorPos(0, 0);

  STARTUPINFO startupInfo;
  ZeroMemory(&startupInfo, sizeof(startupInfo));

  std::wstring desktopName = L"warpr";
  startupInfo.lpDesktop = &desktopName[0];

  PROCESS_INFORMATION processInfo;
  ZeroMemory(&processInfo, sizeof(processInfo));
  
  std::wstring target = L"notepad.exe";
  check_bool(CreateProcess(nullptr, &target[0], nullptr, nullptr, false, 0, nullptr, nullptr, &startupInfo, &processInfo));

  check_bool(SwitchDesktop(myDesktop.get()));
  Sleep(10000);
  check_bool(SwitchDesktop(originalDesktop.get()));

  check_bool(SetThreadDesktop(originalDesktop.get()));

  /*window_station_handle interactiveStation{ check_pointer(OpenWindowStation(L"winsta0", false, READ_CONTROL | WRITE_DAC)) };  
  check_bool(SetProcessWindowStation(interactiveStation.get()));

  desktop_handle desktop{ OpenDesktop(L"default", 0, false, READ_CONTROL | WRITE_DAC | DESKTOP_WRITEOBJECTS | DESKTOP_READOBJECTS) };*/
  check_bool(SetProcessWindowStation(originalStation));

  
}

void SendInputMessage(void* windowHandle, uint16_t x, uint16_t y, uint32_t message, uint32_t flags)
{
  auto hwnd = RealChildWindowFromPoint((HWND)windowHandle, { x, y });
  SendMessageW(hwnd, message, flags, (y << 16) | x);
}

void mouse_click()
{
  WindowLocator locator;
  auto windows = locator.GetWindows();
  auto window = locator.GetWindow(L"Microsoft.Paint_11.2408.30.0_x64__8wekyb3d8bbwe");

  //SendInputMessage(window, 124, 70, WM_CLOSE, 0);
  SendInputMessage(window, 0, 0, WM_SETFOCUS, 0);
  SendMessageW((HWND)window, WM_ACTIVATE, WA_CLICKACTIVE, NULL);
  SendInputMessage(window, 400, 400, WM_MOUSELEAVE, 0);
  SendInputMessage(window, 400, 400, WM_MOUSEMOVE, 0);
  Sleep(100);
  SendInputMessage(window, 400, 400, WM_LBUTTONDOWN, MK_LBUTTON);
  Sleep(100);
  SendInputMessage(window, 400, 400, WM_MOUSEMOVE, MK_LBUTTON);
  Sleep(100);
  SendInputMessage(window, 400, 400, WM_LBUTTONUP, 0);
  __nop();
}

int main()
{
  //std::cout << "Hello World!\n";
  //mouse_click();

  //start_desktop();

  /*InitLogger(LogLevel::Debug);

  WebSocket ws;
  ws.onOpen([] { printf("WebSocket open\n"); });
  ws.onClosed([] { printf("WebSocket closed\n"); });
  ws.onError([](const string& error) { printf("WebSocket error: %s\n", error.c_str()); });
  ws.onMessage([](variant<binary, string> message) {
    printf("WebSocket message received.\n");
    });
  ws.open("wss://127.0.0.1:5001/api/sources/connect");

  Configuration config;
  config.iceServers.emplace_back("stun:stun.l.google.com:19302");
  config.disableAutoNegotiation = true;*/



  std::this_thread::sleep_for(10000s);
}
