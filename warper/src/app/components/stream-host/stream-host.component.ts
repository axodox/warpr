import { Component, ElementRef, ViewChild } from '@angular/core';
import { StreamingService } from '../../services/streaming.service';
import { EncodedFrame, FrameType } from '../../data/frames';
import { Point, PointerAction, PointerInputMessage, PointerType } from '../../data/streaming-messages';

@Component({
  selector: 'app-stream-host',
  templateUrl: './stream-host.component.html',
  styleUrl: './stream-host.component.css'
})
export class StreamHostComponent {

  @ViewChild("renderTarget")
  private _canvas?: ElementRef<HTMLCanvasElement>;
  private _renderingContext: CanvasRenderingContext2D | null = null;

  private _decoder: VideoDecoder;
  private _framePending?: VideoFrame;
  private _width = 0;
  private _height = 0;
  private _isDecoderInitialized = false;

  public constructor(
    private _streamingService: StreamingService) {
    this._streamingService.FrameReceived.Subscribe((sender, eventArgs) => this.OnFrameReceived(eventArgs));

    this._decoder = new VideoDecoder({
      output: (frame) => this.OnFrameDecoded(frame),
      error: (error) => console.log(error)
    });
  }

  private async ngAfterViewInit() {

    console.log("Initializing stream host...");
    this._renderingContext = this._canvas!.nativeElement.getContext("2d")!;
    this._canvas!.nativeElement.onpointerdown = (event) => this.OnPointerEvent(event);
    this._canvas!.nativeElement.onpointermove = (event) => this.OnPointerEvent(event);
    this._canvas!.nativeElement.onpointerup = (event) => this.OnPointerEvent(event);

    if (this._renderingContext) {
      console.log("Stream host initialized.");
    }
    else {
      console.log("Stream host failed to initialize.");
    }

    requestAnimationFrame(() => this.RenderFrame());
  }

  private OnPointerEvent(event: PointerEvent) {
    //console.log(event);

    let message = new PointerInputMessage(event);
    //console.log(message);

    this._streamingService.SendMessage(message);
  }

  private OnFrameReceived(frame: EncodedFrame) {
    let chunk = new EncodedVideoChunk({
      data: frame.Bytes,
      type: frame.Type == FrameType.Key ? "key" : "delta",
      timestamp: frame.Index
    });

    //Initialize decoder
    if (frame.Width != this._width || frame.Height != this._height) {
      console.log("Configuring decoder...");
      this._decoder.configure({
        codec: "hvc1.1.6.L93.B0",
        codedWidth: frame.Width,
        codedHeight: frame.Height,
        hardwareAcceleration: "prefer-hardware",
        optimizeForLatency: true
      });
      console.log("Decoder configured.");

      this._width = frame.Width;
      this._height = frame.Height;
    }

    //Decode frame
    if (!this._isDecoderInitialized && chunk.type != "key") return;
    this._isDecoderInitialized = true;

    this._decoder.decode(chunk);
  }

  private OnFrameDecoded(frame: VideoFrame) {
    if (this._framePending) this._framePending.close();
    this._framePending = frame;
    requestAnimationFrame(() => this.RenderFrame());
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

  }
}
