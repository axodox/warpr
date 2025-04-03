import { Injectable } from '@angular/core';
import { WebSocketClient } from '../networking/web-socket-client';
import { WarprSignalingMessage, ConnectionRequest } from '../data/signaling-messages';
import { v4 as uuid } from 'uuid';

@Injectable({
  providedIn: 'root'
})
export class MessagingService extends WebSocketClient<WarprSignalingMessage> {

  private static readonly _connectionUri = 'api/sinks/connect';
  private readonly _sessionId = uuid();

  constructor() {
    const uri = MessagingService.GetServerUri();
    super(uri);
  }

  private static GetServerUri(): string {
    const location = window.location;
    const protocol = location.protocol === 'https:' ? 'wss' : 'ws';
    return `${protocol}://${location.host}/${this._connectionUri}`;
  }

  protected override OnConnected(): void {
    super.OnConnected();

    let request = new ConnectionRequest();
    request.SessionId = this._sessionId;
    this.SendMessage(request);
  }
}
