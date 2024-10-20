import { EventOwner, EventPublisher } from "../insfrastructure/events";
import { IMessagingClient } from "./messaging-client";

export class WebSocketClient<TMessage> implements IMessagingClient<TMessage> {
  private readonly socket: WebSocket;

  private readonly events = new EventOwner();
  public readonly MessageReceived = new EventPublisher<IMessagingClient<TMessage>, TMessage>(this.events);

  constructor(uri: string) {
    console.log(`Connecting to ${uri}...`);

    let socket = new WebSocket(uri);
    socket.onopen = () => this.OnConnected();
    socket.onmessage = (e: MessageEvent<string>) => this.OnMessageReceived(e);
    socket.onclose = () => this.OnDisconnected();
    socket.onerror = (e: Event) => this.OnError(e);
    this.socket = socket;
  }

  SendMessage(message: TMessage): void {
    let content = JSON.stringify(message);
    console.log(`Sending message: ${content}`);

    this.socket.send(content);
  }

  private OnConnected(): void {
    console.log(`Connected to: ${this.socket.url}.`);
  }

  private OnMessageReceived(eventArgs: MessageEvent<string>): void {
    console.log('Message received: ' + eventArgs.data);

    let message = JSON.parse(eventArgs.data) as TMessage;
    this.events.Raise(this.MessageReceived, this, message);
  }

  private OnDisconnected(): void {
    console.log(`Disconnected from: ${this.socket.url}.`);
  }

  private OnError(error: Event): void {
    console.log('Connection failed: ' + error);
  }
}
