export type EventHandler<TSender, TEventArgs> = (sender: TSender, eventArgs: TEventArgs) => void;

export interface IEventSource {

  Unsubscribe(token: symbol): void;
}

export class EventSubscription {

  constructor(
    private _owner: IEventSource,
    private _token: symbol) { }

  Unsubscribe() {
    this._owner.Unsubscribe(this._token);
  }
}

export class EventOwner {

  public readonly Token = Symbol();

  Raise<TSender, TEventArgs>(event: EventPublisher<TSender, TEventArgs>, sender: TSender, eventArgs: TEventArgs) {
    event.Raise(this, sender, eventArgs);
  }
}

export class EventArgs {
  public static readonly Empty = new EventArgs();
}

export class EventPublisher<TSender, TEventArgs> implements IEventSource {

  private readonly _token: symbol;
  private readonly _handlers = new Map<symbol, EventHandler<TSender, TEventArgs>>;

  constructor(owner: EventOwner) {
    this._token = owner.Token;
  }

  Subscribe(handler: EventHandler<TSender, TEventArgs>) {
    const token = Symbol();
    this._handlers.set(token, handler);
    return new EventSubscription(this, token);
  }

  Raise(owner: EventOwner, sender: TSender, eventArgs: TEventArgs): void {
    if (owner.Token === this._token) {
      for (let [symbol, handler] of this._handlers) {
        handler(sender, eventArgs);
      }
    }
    else {
      throw Error("Cannot raise event: this event source does not belong to the specified owner.");
    }
  }

  Unsubscribe(token: symbol): void {
    this._handlers.delete(token);
  }
}
