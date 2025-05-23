import { EventOwner, EventPublisher, EventArgs } from "../infrastructure/events";
import { IMessagingClient } from "./messaging-client";

export class WebSocketClient<TMessage> implements IMessagingClient<TMessage> {
  private readonly _events = new EventOwner();

  public readonly Connected = new EventPublisher<IMessagingClient<TMessage>, EventArgs>(this._events);
  public readonly MessageReceived = new EventPublisher<IMessagingClient<TMessage>, TMessage>(this._events);
  public readonly Disconnected = new EventPublisher<IMessagingClient<TMessage>, EventArgs>(this._events);

  private _socket?: WebSocket;

  constructor(private _uri: string) { }

  Connect(): void {
    console.log(`Connecting to ${this._uri}...`);

    let socket = new WebSocket(this._uri);
    socket.onopen = () => this.OnConnected();
    socket.onmessage = (e: MessageEvent<string>) => this.OnMessageReceived(e);
    socket.onclose = () => this.OnDisconnected();
    socket.onerror = (e: Event) => this.OnError(e);
    this._socket = socket;
  }

  SendMessage(message: TMessage): void {
    if (!this._socket) return;

    let content = JSON.stringify(message);
    console.debug(`Sending message: ${content}`);

    this._socket.send(content);
  }

  protected OnConnected(): void {
    console.log(`Connected to: ${this._uri}.`);
    this._events.Raise(this.Connected, this, EventArgs.Empty);
  }

  protected OnMessageReceived(eventArgs: MessageEvent<string>): void {
    console.debug('Message received: ' + eventArgs.data);

    let message = JSON.parse(eventArgs.data) as TMessage;
    this._events.Raise(this.MessageReceived, this, message);
  }

  protected OnDisconnected(): void {
    console.log(`Disconnected from: ${this._uri}.`);
    this._events.Raise(this.Disconnected, this, EventArgs.Empty);
  }

  protected OnError(error: Event): void {
    console.log('Connection failed: ' + error);
  }
}
