// warp-drive.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "rtc/rtc.hpp"

using namespace rtc;

int main()
{
  std::cout << "Hello World!\n";

  InitLogger(LogLevel::Debug);

  WebSocket ws;
  ws.onOpen([] { printf("WebSocket open\n"); });
  ws.onClosed([] { printf("WebSocket closed\n"); });
  ws.onError([](const string& error) { printf("WebSocket error: %s\n", error.c_str()); });
  ws.onMessage([](variant<binary, string> message) {
    printf("WebSocket message received.\n");
    });
  ws.open("ws://127.0.0.1:9999");

  Configuration config;
  config.iceServers.emplace_back("stun:stun.l.google.com:19302");
  config.disableAutoNegotiation = true;

  system("pause");
}
