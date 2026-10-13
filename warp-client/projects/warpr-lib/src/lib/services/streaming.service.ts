import { Injectable } from '@angular/core';
import { MessagingService } from './messaging.service';
import { GetConfiguration, PeerConnectionCandidateMessage, PeerConnectionDescriptionMessage, WarprSignalingMessage, WarprSignalingMessageType } from '../data/signaling-messages';
import { IMessagingClient } from '../networking/messaging-client';
import { EncodedFrame } from '../data/frames';
import { EventOwner, EventPublisher } from '../infrastructure/events';
import { MessageAssembler } from '../infrastructure/message-assembler';
import { MessageSplitter } from '../infrastructure/message-splitter';
import { MessageContent } from '../infrastructure/messages';
import { FrameAssembler } from '../infrastructure/frame-assembler';

@Injectable({
  providedIn: 'root'
})
export class StreamingService {

  private _peerConnection?: RTCPeerConnection;
  private _controlConnection?: RTCDataChannel;

  private _streamConnection?: RTCDataChannel;
  private _frameAssembler?: FrameAssembler;

  private _auxConnection?: RTCDataChannel;
  private _auxMessageBuilder?: MessageAssembler;
  private _auxMessageSplitter?: MessageSplitter;

  private readonly _events = new EventOwner();
  public readonly FrameReceived = new EventPublisher<StreamingService, EncodedFrame>(this._events);
  public readonly Connected = new EventPublisher<StreamingService, any>(this._events);
  public readonly AuxChannelReady = new EventPublisher<StreamingService, any>(this._events);

  public readonly ControlMessageReceived = new EventPublisher<StreamingService, MessageContent>(this._events);
  public readonly AuxMessageReceived = new EventPublisher<StreamingService, MessageContent>(this._events);

  constructor(
    private _messagingService: MessagingService) {

    _messagingService.MessageReceived.Subscribe((sender: IMessagingClient<WarprSignalingMessage>, message: WarprSignalingMessage) => this.OnSignalingMessageReceived(sender, message));
    _messagingService.Connect();
  }

  private Reconnect(configuration: RTCConfiguration) {
    console.log("Establishing peer connection...");

    this._controlConnection?.close();
    this._streamConnection?.close();
    this._auxConnection?.close();
    this._peerConnection?.close();

    this._frameAssembler?.Reset();
    this._frameAssembler = new FrameAssembler((frame) => this.OnFrameAssembled(frame));
    this._auxMessageBuilder = new MessageAssembler();
    this._auxMessageSplitter = new MessageSplitter();

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
    if (!this._auxMessageSplitter) return;

    let fragments = this._auxMessageSplitter.SplitMessage(message);
    for (let fragment of fragments) {
      this._auxConnection?.send(fragment);
    }
  }

  private OnConnectionStateChanged() {
    console.log("WebRTC: " + this._peerConnection?.connectionState);

    if (this._peerConnection?.connectionState === "connected") {
      let maxMessageSize = this._peerConnection?.sctp?.maxMessageSize;
      if (maxMessageSize) this._auxMessageSplitter!.MaxMessageSize = maxMessageSize;
    }
  }

  private OnDataChannel(event: RTCDataChannelEvent) {
    console.log(`WebRTC data channel ${event.channel.label} connected.`);

    switch (event.channel.label) {
      case "control":
        this._controlConnection = event.channel;
        this._controlConnection.binaryType = "arraybuffer"
        this._controlConnection.onmessage = (event) => this._events.Raise(this.ControlMessageReceived, this, event.data);
        break;
      case "stream":
        this._streamConnection = event.channel;
        this._streamConnection.binaryType = "arraybuffer"
        this._streamConnection.onmessage = (event) => this.OnStreamMessage(event);
        break;
      case "aux":
        this._auxConnection = event.channel;
        this._auxConnection.binaryType = "arraybuffer"
        this._auxConnection.onmessage = (event) => this.OnAuxMessage(event);
        this._events.Raise(this.Connected, this, null);
        break;
    }
  }

  private _lastFramerateRefreshTime = performance.now();
  private _frameCounter = 0;

  private OnStreamMessage(event: MessageEvent<any>) {
    this._frameAssembler?.PushPacket(event.data as ArrayBuffer);
  }

  private OnFrameAssembled(frame: EncodedFrame) {
    this._frameCounter++;

    let now = performance.now();
    let elapsed = now - this._lastFramerateRefreshTime;
    if (elapsed > 1000) {
      this._lastFramerateRefreshTime = now;

      console.debug(`Messages: ${this._frameCounter / elapsed * 1000} FPS`);
      this._frameCounter = 0;
    }

    this._events.Raise(this.FrameReceived, this, frame);
  }

  private OnAuxMessage(event: MessageEvent<any>) {
    let message = this._auxMessageBuilder?.PushMessage(event.data);
    if (message) this._events.Raise(this.AuxMessageReceived, this, message);
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
