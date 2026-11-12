import { Component, ElementRef, Input, ViewChild } from '@angular/core';
import { StreamingService } from '../../services/streaming.service';
import { EncodedFrame, FrameType, Size, VideoCodec } from '../../data/frames';
import { PointerInputMessage, PointerStates, RequestKeyFrameMessage, ResizeSurfaceMessage, VideoCodecPreferenceMessage } from '../../data/streaming-messages';
import { CommonModule } from '@angular/common';
import { CreateVideoFrameRenderer, IVideoFrameRenderer } from './video-frame-renderer';

@Component({
  selector: 'warpr-stream-host',
  imports: [CommonModule],
  templateUrl: './stream-host.component.html',
  styleUrl: './stream-host.component.scss'
})
export class StreamHostComponent {
  private static readonly _maxFramesInDecodeQueue = 10;

  //In order of preference
  private static readonly _codecStrings = new Map<VideoCodec, string>([
    [VideoCodec.H265, "hvc1.1.6.L93.B0"],
    [VideoCodec.H264, "avc1.640033"]
  ]);

  private static _supportedCodecs?: Promise<VideoCodec[]>;

  @Input()
  public isMenuVisible = true;

  @ViewChild("renderTarget")
  private _canvas?: ElementRef<HTMLCanvasElement>;
  private _renderer: IVideoFrameRenderer | null = null;

  private _decoder: VideoDecoder | null = null;
  private _framePending?: VideoFrame;
  private _videoSize = Size.Empty;
  private _displaySize = Size.Empty;
  private _isDecoderInitialized = false;
  private _pointerStates: PointerStates = {};
  private _mousePointerId = 1;
  private _framesInDecodeQueue = 0;
  private _videoCodec = VideoCodec.Unknown;
  private _decodableCodecs?: Set<VideoCodec>;
  private _workingCodecs = new Set<VideoCodec>();

  public get IsFullScreenAvailable(): boolean {
    return document.fullscreenEnabled && this.isMenuVisible;
  }

  public constructor(
    private _streamingService: StreamingService,
    private _root: ElementRef<HTMLDivElement>) {
    this._streamingService.Connected.Subscribe((sender, eventArgs) => this.OnConnected());
    this._streamingService.FrameReceived.Subscribe((sender, eventArgs) => this.OnFrameReceived(eventArgs));
  }

  private static GetSupportedCodecsAsync(): Promise<VideoCodec[]> {
    StreamHostComponent._supportedCodecs ??= (async () => {
      let result: VideoCodec[] = [];
      for (let [codec, codecString] of StreamHostComponent._codecStrings) {
        try {
          let support = await VideoDecoder.isConfigSupported({
            codec: codecString,
            hardwareAcceleration: "prefer-hardware",
            optimizeForLatency: true
          });
          if (support.supported) result.push(codec);
        }
        catch (error) {
          console.warn(`Failed to check decoder support for ${codecString}.`, error);
        }
      }

      console.log(`Supported codecs: ${result.map(p => VideoCodec[p]).join(", ") || "none"}.`);
      return result;
    })();

    return StreamHostComponent._supportedCodecs;
  }

  private async SendCodecPreferenceAsync() {
    this._decodableCodecs ??= new Set(await StreamHostComponent.GetSupportedCodecsAsync());
    this._streamingService.SendControlMessage(new VideoCodecPreferenceMessage([...this._decodableCodecs].map(p => VideoCodec[p])));
  }

  private OnDecoderError(codec: VideoCodec, error: DOMException) {
    console.error(error);

    //Some browsers report support for a codec they cannot decode (e.g. HEVC in Firefox), so drop codecs which never produced a frame
    if (!this._workingCodecs.has(codec) && this._decodableCodecs?.delete(codec)) {
      console.warn(`Decoding ${VideoCodec[codec]} failed, falling back to other codecs.`);
      this.SendCodecPreferenceAsync();
      return;
    }

    this._streamingService.SendControlMessage(new RequestKeyFrameMessage());
  }

  private async ngAfterViewInit() {

    console.log("Initializing stream host...");

    let canvas = this._canvas!.nativeElement;
    this._renderer = CreateVideoFrameRenderer(canvas);
    canvas.onpointerdown = (event) => this.OnPointerEvent(event);
    canvas.onpointermove = (event) => this.OnPointerEvent(event);
    canvas.onpointerup = (event) => this.OnPointerEvent(event);
    canvas.onwheel = (event) => this.OnWheelEvent(event);
    canvas.oncontextmenu = (event) => event.preventDefault();

    if (this._renderer) {
      console.log(`Stream host initialized with ${this._renderer.Name} renderer.`);
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
    if (event.pointerType == "mouse") this._mousePointerId = event.pointerId;

    let message = new PointerInputMessage(event, this._pointerStates, this._canvas!.nativeElement, wheelDelta);
    //console.log(message);

    this._streamingService.SendControlMessage(message);
  }

  private OnWheelEvent(event: WheelEvent) {
    //console.log(event);
    event.preventDefault();

    //Wheel events have no pointer id, and the mouse pointer id differs between browsers (e.g. 1 in Chrome, 0 in Firefox)
    let pointerEvent: PointerEvent = new PointerEvent(event.type, {
      pointerId: this._mousePointerId,
      pointerType: "mouse",
      clientX: event.clientX,
      clientY: event.clientY
    });
    this.OnPointerEvent(pointerEvent, event.deltaY);
  }

  private OnConnected() {
    this._displaySize = Size.Empty;
    this.SendCodecPreferenceAsync();
  }

  private OnFrameReceived(frame: EncodedFrame) {
    //Skip frames we cannot decode, the server will switch codec once it receives our preference
    let codecString = StreamHostComponent._codecStrings.get(frame.Codec);
    if (!codecString || !this._decodableCodecs?.has(frame.Codec)) return;

    let chunk = new EncodedVideoChunk({
      data: frame.Bytes,
      type: frame.Type == FrameType.Key ? "key" : "delta",
      timestamp: frame.Index
    });

    //Initialize decoder
    let frameSize = new Size(frame.Width, frame.Height);

    if (this._decoder === null || this._decoder.state === 'closed' || !Size.AreEqual(frameSize, this._videoSize) || frame.Codec !== this._videoCodec || this._framesInDecodeQueue > StreamHostComponent._maxFramesInDecodeQueue) {
      console.log("Configuring decoder...");

      if (this._decoder !== null && this._decoder.state !== 'closed') this._decoder.close();
      let codec = frame.Codec;
      this._decoder = new VideoDecoder({
        output: (frame) => this.OnFrameDecoded(frame),
        error: (error) => this.OnDecoderError(codec, error)
      });
      this._isDecoderInitialized = false;

      this._decoder.configure({
        codec: codecString,
        codedWidth: frame.Width,
        codedHeight: frame.Height,
        hardwareAcceleration: "prefer-hardware",
        optimizeForLatency: true
      });
      console.log(`Decoder configured for ${VideoCodec[frame.Codec]}.`);
      console.log(`Resolution is ${frame.Width} x ${frame.Height}.`);

      this._videoSize = frameSize;
      this._videoCodec = frame.Codec;
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
    this._workingCodecs.add(this._videoCodec);

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
      
      //Assigning the size resets the canvas even if unchanged, which is expensive at high resolutions
      if (canvas.width !== frame.displayWidth) canvas.width = frame.displayWidth;
      if (canvas.height !== frame.displayHeight) canvas.height = frame.displayHeight;

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
      this._renderer?.Draw(frame);

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
