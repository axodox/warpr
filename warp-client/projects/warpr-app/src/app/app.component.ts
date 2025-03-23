import { Component } from '@angular/core';
import { RouterOutlet } from '@angular/router';
import { StreamHostComponent, StreamingService } from "warpr-lib";

@Component({
  selector: 'app-root',
  standalone: true,
  imports: [RouterOutlet, StreamHostComponent],
  templateUrl: './app.component.html',
  styleUrl: './app.component.scss',
})
export class AppComponent {
  title = 'warper';

  constructor(
    private _streamingService: StreamingService) {

  }
}
