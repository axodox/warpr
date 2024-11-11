import { Component } from '@angular/core';
import { StreamingService } from './services/streaming.service';

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
