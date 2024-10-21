// warp-drive.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <winrt/base.h>
//#include <print>
#include "rtc/rtc.hpp"
#include "WindowLocator.h"

#include "Messaging/Messages.h"
#include "Infrastructure/RtcLogger.h"

using namespace std;
using namespace rtc;
using namespace winrt;
using namespace warper;


using namespace Axodox::Infrastructure;
using namespace Axodox::Json;
using namespace Warpr::Infrastructure;
using namespace Warpr::Messaging;

int main()
{
  logger log{ "main" };
  log.log(log_severity::information, "Hello Warpr!");
  
  InitRtcLogger(log_severity::debug);

  /*WebSocket ws;
  ws.onOpen([] { printf("WebSocket open\n"); });
  ws.onClosed([] { printf("WebSocket closed\n"); });
  ws.onError([](const string& error) { printf("WebSocket error: %s\n", error.c_str()); });
  ws.onMessage([](variant<binary, string> message) {
    printf("WebSocket message received.\n");
    });
  ws.open("wss://127.0.0.1:5001/api/sources/connect");*/

  auto descriptionMessage = make_unique<ConnectionDescriptionMessage>();
  *descriptionMessage->Description = "asdad";

  auto x = intptr_t(&descriptionMessage->Description);
  auto y = intptr_t(descriptionMessage.get());
  auto z = x - y;
  
  unique_ptr<WarprMessage> message = move(descriptionMessage);



  auto text = stringify_json(message);
  auto message2 = try_parse_json<std::unique_ptr<WarprMessage>>(text);

  Configuration config;
  config.iceServers.emplace_back("stun:stun.l.google.com:19302");
  //config.disableAutoNegotiation = true;

  PeerConnection connection{ config };
  connection.onLocalDescription([](Description description) {
    cout << "Description: " << string(description) << endl;
    });

  connection.onLocalCandidate([](Candidate candidate) {
    cout << "Candidate: " << string(candidate) << endl;
    });

  auto channel = connection.createDataChannel("test");
  
  std::this_thread::sleep_for(10000s);
}
