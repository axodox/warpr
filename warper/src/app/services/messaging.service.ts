import { Injectable } from '@angular/core';
import { WebSocketClient } from '../networking/web-socket-client';
import { WarprMessage } from '../data/messages';

@Injectable({
  providedIn: 'root'
})
export class MessagingService extends WebSocketClient<WarprMessage> {

  private static readonly _connectionUri = 'api/sources/connect';

  constructor() {
    const uri = MessagingService.GetServerUri();
    super(uri);
  }

  private static GetServerUri(): string {
    const location = window.location;
    const protocol = location.protocol === 'https:' ? 'wss' : 'ws';
    return `${protocol}://${location.host}/${this._connectionUri}`;
  }

  
}
