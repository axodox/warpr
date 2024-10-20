import { EventPublisher } from "../insfrastructure/events";

export interface IMessagingClient<TMessage> {
  readonly MessageReceived: EventPublisher<IMessagingClient<TMessage>, TMessage>;

  SendMessage(message: TMessage): void;
}
