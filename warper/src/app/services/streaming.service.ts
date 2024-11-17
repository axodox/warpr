import { Injectable } from '@angular/core';
import { MessagingService } from './messaging.service';
import { PeerConnectionCandidateMessage, PeerConnectionDescriptionMessage, WarprMessage, WarprMessageType } from '../data/messages';
import { IMessagingClient } from '../networking/messaging-client';
import { EncodedFrame, FrameType } from '../data/frames';

@Injectable({
  providedIn: 'root'
})
export class StreamingService {

  private _peerConnection: RTCPeerConnection;
  private _reliableConnection?: RTCDataChannel;
  private _lowLatencyConnection?: RTCDataChannel;

  constructor(
    private _messagingService: MessagingService) {

    _messagingService.MessageReceived.Subscribe((sender: IMessagingClient<WarprMessage>, message: WarprMessage) => this.OnMessageReceived(sender, message));

    let config: RTCConfiguration = {
      iceServers: [
        { urls: "stun:stun.l.google.com:19302" }
      ]
    };

    this._peerConnection = new RTCPeerConnection(config);
    this._peerConnection.onicecandidate = (event) => this.OnIceCandidateAdded(event.candidate?.candidate);
    this._peerConnection.ondatachannel = (event) => this.OnDataChannel(event);
  }

  private OnDataChannel(event: RTCDataChannelEvent) {
    switch (event.channel.label) {
      case "reliable":
        this._reliableConnection = event.channel;
        break;
      case "low_latency":
        this._lowLatencyConnection = event.channel;
        this._lowLatencyConnection.onmessage = (event) => this.OnLowLatencyMessage(event);
        break;
    }
  }

  private OnLowLatencyMessage(event: MessageEvent<any>) {
    let buffer = event.data as ArrayBuffer;
    let frame = new EncodedFrame(buffer);
    if (frame.Type == FrameType.Key) {
      console.log("Key frame received");
    }
  }

  private OnIceCandidateAdded(candidate?: string) {
    if (candidate == null) return;

    let message = new PeerConnectionCandidateMessage();
    message.Candidate = candidate;
    this._messagingService.SendMessage(message);
  }

  private async OnMessageReceived(sender: IMessagingClient<WarprMessage>, message: WarprMessage) {

    switch (message.Type) {
      case WarprMessageType.PeerConnectionDescriptionMessage:
        await this._peerConnection.setRemoteDescription({ type: "offer", sdp: message.Description });
        let answer = await this._peerConnection.createAnswer();
        await this._peerConnection.setLocalDescription(answer);

        let answerMessage = new PeerConnectionDescriptionMessage();
        answerMessage.Description = answer.sdp;
        this._messagingService.SendMessage(answerMessage);

        break;
      case WarprMessageType.PeerConnectionCandidateMessage:
        await this._peerConnection.addIceCandidate({ candidate: message.Candidate, sdpMid: "0" });
        break;
    }

  }
}
