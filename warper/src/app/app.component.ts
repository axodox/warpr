import { Component } from '@angular/core';
import { StreamingService } from './services/streaming.service';
import { StreamHostComponent } from './components/stream-host/stream-host.component';

@Component({
  selector: 'app-root',
  templateUrl: './app.component.html',
  styleUrl: './app.component.css'
})
export class AppComponent {
  title = 'warper';


  constructor(
    private _streamingService: StreamingService) {

  }
}
