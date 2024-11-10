import { EventOwner, EventPublisher, EventArgs } from "../insfrastructure/events";
import { IMessagingClient } from "./messaging-client";

export class WebSocketClient<TMessage> implements IMessagingClient<TMessage> {
  private readonly _socket: WebSocket;
  private readonly _events = new EventOwner();

  public readonly Connected = new EventPublisher<IMessagingClient<TMessage>, EventArgs>(this._events);
  public readonly MessageReceived = new EventPublisher<IMessagingClient<TMessage>, TMessage>(this._events);
  public readonly Disconnected = new EventPublisher<IMessagingClient<TMessage>, EventArgs>(this._events);

  constructor(uri: string) {
    console.log(`Connecting to ${uri}...`);

    let socket = new WebSocket(uri);
    socket.onopen = () => this.OnConnected();
    socket.onmessage = (e: MessageEvent<string>) => this.OnMessageReceived(e);
    socket.onclose = () => this.OnDisconnected();
    socket.onerror = (e: Event) => this.OnError(e);
    this._socket = socket;
  }

  SendMessage(message: TMessage): void {
    let content = JSON.stringify(message);
    console.log(`Sending message: ${content}`);

    this._socket.send(content);
  }

  protected OnConnected(): void {
    console.log(`Connected to: ${this._socket.url}.`);
    this._events.Raise(this.Connected, this, EventArgs.Empty);
  }

  protected OnMessageReceived(eventArgs: MessageEvent<string>): void {
    console.log('Message received: ' + eventArgs.data);

    let message = JSON.parse(eventArgs.data) as TMessage;
    this._events.Raise(this.MessageReceived, this, message);
  }

  protected OnDisconnected(): void {
    console.log(`Disconnected from: ${this._socket.url}.`);
    this._events.Raise(this.Disconnected, this, EventArgs.Empty);
  }

  protected OnError(error: Event): void {
    console.log('Connection failed: ' + error);
  }
}
