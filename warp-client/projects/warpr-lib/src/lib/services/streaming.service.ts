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
  private _controlConnection?: RTCDataChannel;

  private _streamConnection?: RTCDataChannel;
  private _messageBuilder?: MessageAssembler;

  private _auxConnection?: RTCDataChannel;

  private readonly _events = new EventOwner();
  public readonly FrameReceived = new EventPublisher<StreamingService, EncodedFrame>(this._events);
  public readonly Connected = new EventPublisher<StreamingService, any>(this._events);

  public readonly ControlMessageReceived = new EventPublisher<StreamingService, any>(this._events);
  public readonly AuxMessageReceived = new EventPublisher<StreamingService, any>(this._events);

  constructor(
    private _messagingService: MessagingService) {

    _messagingService.MessageReceived.Subscribe((sender: IMessagingClient<WarprSignalingMessage>, message: WarprSignalingMessage) => this.OnSignalingMessageReceived(sender, message));
    _messagingService.Connect();
  }

  private Reconnect(configuration: RTCConfiguration) {
    console.log("Establishing peer connection...");

    this._controlConnection?.close();
    this._streamConnection?.close();
    this._messageBuilder = new MessageAssembler();
    this._auxConnection?.close();
    this._peerConnection?.close();

    this._peerConnection = new RTCPeerConnection(configuration);
    this._peerConnection.onicecandidate = (event) => this.OnIceCandidateAdded(event.candidate?.candidate);
    this._peerConnection.ondatachannel = (event) => this.OnDataChannel(event);
    this._peerConnection.onconnectionstatechange = (event) => this.OnConnectionStateChanged();
  }

  public SendControlMessage(message: any) {
    let json = JSON.stringify(message);
    this._controlConnection?.send(json);
  }

  public SendAuxMessage(message: any) {
    this._auxConnection?.send(message);
  }

  private OnConnectionStateChanged() {
    console.log("WebRTC: " + this._peerConnection?.connectionState);
  }

  private OnDataChannel(event: RTCDataChannelEvent) {
    console.log(`WebRTC data channel ${event.channel.label} connected.`);

    switch (event.channel.label) {
      case "control":
        this._controlConnection = event.channel;
        this._controlConnection.binaryType = "arraybuffer"
        this._controlConnection.onmessage = (event) => this._events.Raise(this.ControlMessageReceived, this, event.data);
        this._events.Raise(this.Connected, this, null);
        break;
      case "stream":
        this._streamConnection = event.channel;
        this._streamConnection.binaryType = "arraybuffer"
        this._streamConnection.onmessage = (event) => this.OnLowLatencyMessage(event);
        break;
      case "aux":
        this._auxConnection = event.channel;
        this._auxConnection.binaryType = "arraybuffer"
        this._auxConnection.onmessage = (event) => this._events.Raise(this.AuxMessageReceived, this, event.data);
        this._events.Raise(this.Connected, this, null);
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

  private async OnSignalingMessageReceived(sender: IMessagingClient<WarprSignalingMessage>, message: WarprSignalingMessage) {

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
