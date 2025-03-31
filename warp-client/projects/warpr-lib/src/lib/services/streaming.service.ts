import { Injectable } from '@angular/core';
import { MessagingService } from './messaging.service';
import { GetConfiguration, PairingCompleteMessage, PeerConnectionCandidateMessage, PeerConnectionDescriptionMessage, WarprSignalingMessage, WarprSignalingMessageType } from '../data/signaling-messages';
import { IMessagingClient } from '../networking/messaging-client';
import { EncodedFrame } from '../data/frames';
import { EventOwner, EventPublisher } from '../infrastructure/events';
import { MessageAssembler } from '../infrastructure/message-assembler';

@Injectable({
  providedIn: 'root'
})
export class StreamingService {

  private _peerConnection?: RTCPeerConnection;
  private _reliableConnection?: RTCDataChannel;
  private _lowLatencyConnection?: RTCDataChannel;
  private _messageBuilder? : MessageAssembler;

  private readonly _events = new EventOwner();
  public readonly FrameReceived = new EventPublisher<StreamingService, EncodedFrame>(this._events);
  public readonly Connected = new EventPublisher<StreamingService, any>(this._events);

  constructor(
    private _messagingService: MessagingService) {

    _messagingService.MessageReceived.Subscribe((sender: IMessagingClient<WarprSignalingMessage>, message: WarprSignalingMessage) => this.OnMessageReceived(sender, message));
    _messagingService.Connect();
  }

  private Reconnect(configuration: RTCConfiguration) {
    console.log("Establishing peer connection...");

    this._reliableConnection?.close();
    this._lowLatencyConnection?.close();
    this._peerConnection?.close();
    this._messageBuilder = new MessageAssembler();

    this._peerConnection = new RTCPeerConnection(configuration);
    this._peerConnection.onicecandidate = (event) => this.OnIceCandidateAdded(event.candidate?.candidate);
    this._peerConnection.ondatachannel = (event) => this.OnDataChannel(event);
    this._peerConnection.onconnectionstatechange = (event) => this.OnConnectionStateChanged();
  }
  
  public SendMessage(message: any) {
    let json = JSON.stringify(message);
    this._reliableConnection?.send(json);
  }

  private OnConnectionStateChanged() {
    console.log("WebRTC: " + this._peerConnection?.connectionState);
  }

  private OnDataChannel(event: RTCDataChannelEvent) {
    console.log(`WebRTC data channel ${event.channel.label} connected.`);

    switch (event.channel.label) {
      case "reliable":
        this._reliableConnection = event.channel;
        this._events.Raise(this.Connected, this, null);
        break;
      case "low_latency":
        this._lowLatencyConnection = event.channel;
        this._lowLatencyConnection.binaryType = "arraybuffer"
        this._lowLatencyConnection.onmessage = (event) => this.OnLowLatencyMessage(event);
        break;
    }
  }

  private _lastRefreshTime = performance.now();
  private _count = 0;

  private OnLowLatencyMessage(event: MessageEvent<any>) {

    let message = this._messageBuilder?.PushMessage(event.data as ArrayBuffer);
    if (!message) return;

    let now = performance.now();
    let elapsed = now - this._lastRefreshTime;
    this._count++;

    if (elapsed > 1000) {
      this._lastRefreshTime = now;

      console.debug(`Messages: ${this._count / elapsed * 1000} FPS`);
      this._count = 0;
    }

    let frame = new EncodedFrame(message);
    this._events.Raise(this.FrameReceived, this, frame);
  }

  private OnIceCandidateAdded(candidate?: string) {
    if (candidate == null) return;

    let message = new PeerConnectionCandidateMessage();
    message.Candidate = candidate;
    this._messagingService.SendMessage(message);
  }

  private async OnMessageReceived(sender: IMessagingClient<WarprSignalingMessage>, message: WarprSignalingMessage) {

    switch (message.$type) {
      case WarprSignalingMessageType.PairingCompleteMessage:
        this.Reconnect(GetConfiguration(message.IceServers));
        break;
      case WarprSignalingMessageType.PeerConnectionDescriptionMessage:
        if (!this._peerConnection) return;

        await this._peerConnection.setRemoteDescription({ type: "offer", sdp: message.Description });
        let answer = await this._peerConnection.createAnswer();
        await this._peerConnection.setLocalDescription(answer);

        let answerMessage = new PeerConnectionDescriptionMessage();
        answerMessage.Description = answer.sdp;
        this._messagingService.SendMessage(answerMessage);

        break;
      case WarprSignalingMessageType.PeerConnectionCandidateMessage:
        if (!this._peerConnection) return;

        await this._peerConnection.addIceCandidate({ candidate: message.Candidate, sdpMid: "0" });
        break;
    }

  }
}
