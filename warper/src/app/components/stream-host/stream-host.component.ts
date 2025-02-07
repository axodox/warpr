import { Component, ElementRef, ViewChild } from '@angular/core';
import VertexShaderSource from "./vertex-shader.wgsl";
import PixelShaderSource from "./pixel-shader.wgsl";
import { StreamingService } from '../../services/streaming.service';
import { EncodedFrame, FrameType } from '../../data/frames';
import { EncodedVideoFrame } from './stream-decoder-worker';

@Component({
  selector: 'app-stream-host',
  templateUrl: './stream-host.component.html',
  styleUrl: './stream-host.component.css'
})
export class StreamHostComponent {

  @ViewChild("renderTarget")
  private _canvas?: ElementRef<HTMLCanvasElement>;
  private _renderingContext: CanvasRenderingContext2D | null = null;

  private _decoder: Worker;
  private _framePending?: VideoFrame;


  public constructor(streamingService: StreamingService) {
    streamingService.FrameReceived.Subscribe((sender, eventArgs) => this.OnFrameReceived(eventArgs));

    this._decoder = new Worker(new URL('./stream-decoder-worker', import.meta.url));
    this._decoder.onmessage = (event) => this.OnFrameDecoded(event.data as VideoFrame);
  }

  private async ngAfterViewInit() {

    console.log("Initializing stream host...");
    this._renderingContext = this._canvas?.nativeElement.getContext("2d")!;

    if (this._renderingContext) {
      console.log("Stream host initialized.");
    }
    else {
      console.log("Stream host failed to initialize.");
    }

    requestAnimationFrame(() => this.RenderFrame());
  }

  private OnFrameReceived(frame: EncodedFrame) {
    let chunk = new EncodedVideoChunk({
      data: frame.Bytes,
      type: frame.Type == FrameType.Key ? "key" : "delta",
      timestamp: frame.Index
    });

    let message = new EncodedVideoFrame(chunk, frame.Width, frame.Height);
    this._decoder.postMessage(message);
  }

  private OnFrameDecoded(frame: VideoFrame) {
    if (this._framePending) this._framePending.close();
    this._framePending = frame;
  }

  private RenderFrame() {

    let frame = this._framePending;
    this._framePending = undefined;

    if (frame) {

      this._canvas!.nativeElement.width = frame.displayWidth;
      this._canvas!.nativeElement.height = frame.displayHeight;

      if (this._renderingContext) {
        this._renderingContext.drawImage(frame, 0, 0, frame.displayWidth, frame.displayHeight);
      }

      frame.close();
    }

    requestAnimationFrame(() => this.RenderFrame());
  }
}
