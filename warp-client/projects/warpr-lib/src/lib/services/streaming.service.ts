import { Injectable } from '@angular/core';
import { MessagingService } from './messaging.service';
import { GetConfiguration, PeerConnectionCandidateMessage, PeerConnectionDescriptionMessage, WarprSignalingMessage, WarprSignalingMessageType } from '../data/signaling-messages';
import { IMessagingClient } from '../networking/messaging-client';
import { EncodedFrame } from '../data/frames';
import { EventOwner, EventPublisher } from '../infrastructure/events';
import { MessageAssembler } from '../infrastructure/message-assembler';
import { MessageSplitter } from '../infrastructure/message-splitter';
import { MessageContent } from '../infrastructure/messages';

@Injectable({
  providedIn: 'root'
})
export class StreamingService {

  private _peerConnection?: RTCPeerConnection;
  private _controlConnection?: RTCDataChannel;

  private _streamConnection?: RTCDataChannel;
  private _streamMessageBuilder?: MessageAssembler;

  private _auxConnection?: RTCDataChannel;
  private _auxMessageBuilder?: MessageAssembler;
  private _auxMessageSplitter?: MessageSplitter;

  private _isConnected = false;

  private readonly _events = new EventOwner();
  public readonly FrameReceived = new EventPublisher<StreamingService, EncodedFrame>(this._events);
  public readonly Connected = new EventPublisher<StreamingService, any>(this._events);
  public readonly Disconnected = new EventPublisher<StreamingService, any>(this._events);
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

    //The previous session ends here
    this.Disconnect();

    this._streamMessageBuilder = new MessageAssembler(3);
    this._auxMessageBuilder = new MessageAssembler();
    this._auxMessageSplitter = new MessageSplitter();

    this._peerConnection = new RTCPeerConnection(configuration);
    this._peerConnection.onicecandidate = (event) => this.OnIceCandidateAdded(event.candidate?.candidate);
    this._peerConnection.ondatachannel = (event) => this.OnDataChannel(event);
    this._peerConnection.onconnectionstatechange = (event) => this.OnConnectionStateChanged();
  }

  //Ends the session: closes the connections, detaching their handlers first so their closing does not affect a new session
  private Disconnect() {
    for (let channel of [this._controlConnection, this._streamConnection, this._auxConnection]) {
      if (!channel) continue;

      channel.onmessage = null;
      channel.onclose = null;
      channel.close();
    }

    this._controlConnection = undefined;
    this._streamConnection = undefined;
    this._auxConnection = undefined;

    if (this._peerConnection) {
      this._peerConnection.onicecandidate = null;
      this._peerConnection.ondatachannel = null;
      this._peerConnection.onconnectionstatechange = null;
      this._peerConnection.close();
      this._peerConnection = undefined;
    }

    this.OnDisconnected();
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

    let state = this._peerConnection?.connectionState;
    if (state === "connected") {
      let maxMessageSize = this._peerConnection?.sctp?.maxMessageSize;
      if (maxMessageSize) this._auxMessageSplitter!.MaxMessageSize = maxMessageSize;
    }
    else if (state === "failed" || state === "closed") { //"disconnected" is left out, as the connection may recover from it
      this.Disconnect();
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
        this._auxConnection.onclose = () => this.Disconnect();
        this.OnConnected();
        break;
    }
  }

  //The session is usable once the aux channel is available
  private OnConnected() {
    if (this._isConnected) return;
    this._isConnected = true;

    console.log("WebRTC session connected.");
    this._events.Raise(this.Connected, this, null);
  }

  //The session ends when the aux channel closes, the peer connection fails or a new session replaces it
  private OnDisconnected() {
    if (!this._isConnected) return;
    this._isConnected = false;

    console.log("WebRTC session disconnected.");
    this._events.Raise(this.Disconnected, this, null);
  }

  private _lastFramerateRefreshTime = performance.now();
  private _frameCounter = 0;

  private OnStreamMessage(event: MessageEvent<any>) {

    let message = this._streamMessageBuilder?.PushMessage(event.data as ArrayBuffer);
    if (!message) return;

    this._frameCounter++;

    let now = performance.now();
    let elapsed = now - this._lastFramerateRefreshTime;
    if (elapsed > 1000) {
      this._lastFramerateRefreshTime = now;

      console.debug(`Messages: ${this._frameCounter / elapsed * 1000} FPS`);
      this._frameCounter = 0;
    }

    let frame = new EncodedFrame(message as ArrayBuffer);
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
