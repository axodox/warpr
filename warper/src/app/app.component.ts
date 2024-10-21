import { Component } from '@angular/core';
import { MessagingService } from './services/messaging.service';

@Component({
  selector: 'app-root',
  templateUrl: './app.component.html',
  styleUrl: './app.component.css'
})
export class AppComponent {
  title = 'warper';

  constructor(
    private messagingService: MessagingService) {

    let config: RTCConfiguration = {
      iceServers: [
        { urls: "stun:stun.l.google.com:19302" }
      ]      
    };

    let peerConnection = new RTCPeerConnection(config);
    

    let channel = peerConnection.createDataChannel("test");
    
    console.log(peerConnection.localDescription);
  }
}
