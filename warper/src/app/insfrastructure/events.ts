export type EventHandler<TSender, TEventArgs> = (sender: TSender, eventArgs: TEventArgs) => void;

export interface IEventSource {

  Unsubscribe(token: symbol): void;
}

export class EventSubscription {

  constructor(
    private owner: IEventSource,
    private token: symbol) { }

  Unsubscribe() {
    this.owner.Unsubscribe(this.token);
  }
}

export class EventOwner {

  public readonly Token = Symbol();

  Raise<TSender, TEventArgs>(event: EventPublisher<TSender, TEventArgs>, sender: TSender, eventArgs: TEventArgs) {
    event.Raise(this, sender, eventArgs);
  }
}

export class EventPublisher<TSender, TEventArgs> implements IEventSource {

  private readonly token: symbol;
  private readonly handlers = new Map<symbol, EventHandler<TSender, TEventArgs>>;

  constructor(owner: EventOwner) {
    this.token = owner.Token;
  }

  Subscribe(handler: EventHandler<TSender, TEventArgs>) {
    const token = Symbol();
    this.handlers.set(token, handler);
    return new EventSubscription(this, token);
  }

  Raise(owner: EventOwner, sender: TSender, eventArgs: TEventArgs): void {
    if (owner.Token === this.token) {
      for (let [symbol, handler] of this.handlers) {
        handler(sender, eventArgs);
      }
    }
    else {
      throw Error("Cannot raise event: this event source does not belong to the specified owner.");
    }
  }

  Unsubscribe(token: symbol): void {
    this.handlers.delete(token);
  }
}
