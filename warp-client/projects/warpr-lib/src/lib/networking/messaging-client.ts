import { EventPublisher } from "../infrastructure/events";

export interface IMessagingClient<TMessage> {
  readonly MessageReceived: EventPublisher<IMessagingClient<TMessage>, TMessage>;

  SendMessage(message: TMessage): void;
}
