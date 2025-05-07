import { Component, ElementRef, Input, ViewChild } from '@angular/core';
import { StreamingService } from '../../services/streaming.service';
import { EncodedFrame, FrameType, Size } from '../../data/frames';
import { PointerInputMessage, PointerStates, RequestKeyFrameMessage, ResizeSurfaceMessage } from '../../data/streaming-messages';
import { CommonModule } from '@angular/common';

@Component({
  selector: 'warpr-stream-host',
  imports: [CommonModule],
  templateUrl: './stream-host.component.html',
  styleUrl: './stream-host.component.scss'
})
export class StreamHostComponent {
  private static readonly _maxFramesInDecodeQueue = 10;

  @Input()
  public isMenuVisible = true;

  @ViewChild("renderTarget")
  private _canvas?: ElementRef<HTMLCanvasElement>;
  private _renderingContext: CanvasRenderingContext2D | null = null;

  private _decoder: VideoDecoder | null = null;
  private _framePending?: VideoFrame;
  private _videoSize = Size.Empty;
  private _displaySize = Size.Empty;
  private _isDecoderInitialized = false;
  private _pointerStates: PointerStates = {};
  private _framesInDecodeQueue = 0;

  public get IsFullScreenAvailable(): boolean {
    return document.fullscreenEnabled && this.isMenuVisible;
  }

  public constructor(
    private _streamingService: StreamingService,
    private _root: ElementRef<HTMLDivElement>) {
    this._streamingService.Connected.Subscribe((sender, eventArgs) => this.OnConnected());
    this._streamingService.FrameReceived.Subscribe((sender, eventArgs) => this.OnFrameReceived(eventArgs));
  }

  private async ngAfterViewInit() {

    console.log("Initializing stream host...");

    let canvas = this._canvas!.nativeElement;
    this._renderingContext = canvas.getContext("2d")!;
    canvas.onpointerdown = (event) => this.OnPointerEvent(event);
    canvas.onpointermove = (event) => this.OnPointerEvent(event);
    canvas.onpointerup = (event) => this.OnPointerEvent(event);
    canvas.onwheel = (event) => this.OnWheelEvent(event);
    canvas.oncontextmenu = (event) => event.preventDefault();

    if (this._renderingContext) {
      console.log("Stream host initialized.");
    }
    else {
      console.log("Stream host failed to initialize.");
    }

    requestAnimationFrame(() => this.RenderFrame());
  }

  private OnPointerEvent(event: PointerEvent, wheelDelta = 0) {
    //console.log(event);
    event.preventDefault();

    if (event.type == "pointerdown") this._canvas!.nativeElement.setPointerCapture(event.pointerId);

    let message = new PointerInputMessage(event, this._pointerStates, this._canvas!.nativeElement, wheelDelta);
    //console.log(message);

    this._streamingService.SendControlMessage(message);
  }

  private OnWheelEvent(event: WheelEvent) {
    //console.log(event);
    event.preventDefault();

    let pointerEvent: PointerEvent = new PointerEvent(event.type, event);
    this.OnPointerEvent(pointerEvent, event.deltaY);
  }

  private OnConnected() {
    this._displaySize = Size.Empty;
  }

  private OnFrameReceived(frame: EncodedFrame) {
    let chunk = new EncodedVideoChunk({
      data: frame.Bytes,
      type: frame.Type == FrameType.Key ? "key" : "delta",
      timestamp: frame.Index
    });

    //Initialize decoder
    let frameSize = new Size(frame.Width, frame.Height);

    if (this._decoder === null || this._decoder.state === 'closed' || !Size.AreEqual(frameSize, this._videoSize) || this._framesInDecodeQueue > StreamHostComponent._maxFramesInDecodeQueue) {
      console.log("Configuring decoder...");

      if (this._decoder !== null && this._decoder.state !== 'closed') this._decoder.close();
      this._decoder = new VideoDecoder({
        output: (frame) => this.OnFrameDecoded(frame),
        error: (error) => {
          console.error(error);
          this._streamingService.SendControlMessage(new RequestKeyFrameMessage());
        }
      });
      this._isDecoderInitialized = false;

      this._decoder.configure({
        codec: "hvc1.1.6.L93.B0",
        codedWidth: frame.Width,
        codedHeight: frame.Height,
        hardwareAcceleration: "prefer-hardware",
        optimizeForLatency: true
      });
      console.log("Decoder configured.");
      console.log(`Resolution is ${frame.Width} x ${frame.Height}.`);

      this._videoSize = frameSize;
      this._framesInDecodeQueue = 0;
    }

    //Decode frame
    if (!this._isDecoderInitialized && chunk.type != "key") {
      this._streamingService.SendControlMessage(new RequestKeyFrameMessage());
      return;
    }
    this._isDecoderInitialized = true;

    this._decoder.decode(chunk);
    this._framesInDecodeQueue++;
  }

  private OnFrameDecoded(frame: VideoFrame) {
    this._framesInDecodeQueue--;

    if (this._framePending) this._framePending.close();
    this._framePending = frame;
    requestAnimationFrame(() => this.RenderFrame());
  }

  private RenderFrame() {

    let frame = this._framePending;
    this._framePending = undefined;

    if (frame) {

      //Set canvas resolution to video resolution
      let canvas = this._canvas!.nativeElement;
      canvas.width = frame.displayWidth;
      canvas.height = frame.displayHeight;

      //Check for control size change
      let canvasRect = canvas.getBoundingClientRect();
      let canvasSize = new Size(Math.round(canvasRect.width * window.devicePixelRatio), Math.round(canvasRect.height * window.devicePixelRatio));
      if (!Size.AreEqual(canvasSize, this._displaySize)) {
        this._displaySize = canvasSize;

        console.log(`Resized to ${canvasSize.Width} x ${canvasSize.Height}`);
        let message = new ResizeSurfaceMessage(canvasSize.Width, canvasSize.Height);
        this._streamingService.SendControlMessage(message);
      }

      //Draw new frame
      if (this._renderingContext) {
        this._renderingContext.drawImage(frame, 0, 0, frame.displayWidth, frame.displayHeight);
      }

      //Release memory
      frame.close();
    }
  }

  public ToggleFullScreen() {
    if (document.fullscreenElement === this._root?.nativeElement) {
      document.exitFullscreen();
    }
    else {
      this._root?.nativeElement.requestFullscreen();
    }
  }
}
