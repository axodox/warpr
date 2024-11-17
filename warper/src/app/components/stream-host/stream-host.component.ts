import { Component, ElementRef, ViewChild } from '@angular/core';
import VertexShaderSource from "./vertex-shader.wgsl";
import PixelShaderSource from "./pixel-shader.wgsl";
import { StreamingService } from '../../services/streaming.service';
import { EncodedFrame, FrameType } from '../../data/frames';

@Component({
  selector: 'app-stream-host',
  templateUrl: './stream-host.component.html',
  styleUrl: './stream-host.component.css'
})
export class StreamHostComponent {

  @ViewChild("renderTarget")
  private _canvas?: ElementRef<HTMLCanvasElement>;
  private _adapter?: GPUAdapter;
  private _device?: GPUDevice;
  private _pipeline?: GPURenderPipeline;
  private _sampler?: GPUSampler;
  private _renderingContext?: GPUCanvasContext;

  private _decoder: VideoDecoder;
  private _framePending?: VideoFrame;
  private _isDecoderInitialized = false;
  private _isRendererInitialized = false;


  public constructor(streamingService: StreamingService) {
    streamingService.FrameReceived.Subscribe((sender, eventArgs) => this.OnFrameReceived(eventArgs));

    this._decoder = new VideoDecoder({
      output: (frame) => this.OnFrameDecoded(frame),
      error: (error) => this.OnFrameDecodeFailed(error)
    });

    this._decoder.configure({
      codec: "hvc1.1.6.L93.B0",
      codedWidth: 2560,
      codedHeight: 1440
    });
  }

  private async ngAfterViewInit() {
    this._canvas!.nativeElement.width = 2560;
    this._canvas!.nativeElement.height = 1440;

    console.log("Initializing stream host...");
    this._adapter = await navigator.gpu.requestAdapter() ?? undefined;
    if (!this._adapter) {
      console.log("Cannot access GPU adapter.");
      return;
    }

    this._device = await this._adapter.requestDevice();
    if (!this._device) {
      console.log("Cannot access GPU device.");
      return;
    }

    let format = navigator.gpu.getPreferredCanvasFormat();
    this._renderingContext = this._canvas?.nativeElement.getContext("webgpu")!;

    this._renderingContext.configure({
      device: this._device,
      format: format,
      alphaMode: "opaque"
    });

    this._pipeline = this._device.createRenderPipeline({
      layout: "auto",
      vertex: {
        module: this._device.createShaderModule({ code: VertexShaderSource }),
        entryPoint: "vert_main"
      },
      fragment: {
        module: this._device.createShaderModule({ code: PixelShaderSource }),
        entryPoint: "frag_main",
        targets: [{ format: format }]
      },
      primitive: {
        topology: "triangle-list"
      }
    });

    this._sampler = this._device.createSampler({});

    console.log("Stream host initialized.");
    this._isRendererInitialized = true;
  }

  private OnFrameReceived(frame: EncodedFrame) {
    if (this._decoder.state != "configured") return;
    if (!this._isDecoderInitialized && frame.Type != FrameType.Key) return;

    this._isDecoderInitialized = true;

    let chunk = new EncodedVideoChunk({
      data: frame.Bytes,
      type: frame.Type == FrameType.Key ? "key" : "delta",
      timestamp: frame.Index
    });
    this._decoder.decode(chunk);
  }

  private OnFrameDecoded(frame: VideoFrame) {
    if (this._framePending) {
      this._framePending.close();
      this._framePending = frame;
    } else {
      this._framePending = frame;
      requestAnimationFrame(() => this.RenderFrame());
    }
  }

  private RenderFrame() {
    let frame = this._framePending;
    this._framePending = undefined;
    if (!frame) return; 

    if (this._isRendererInitialized) {
      let binding = this._device?.createBindGroup({
        layout: this._pipeline!.getBindGroupLayout(0),
        entries: [
          { binding: 1, resource: this._sampler! },
          { binding: 2, resource: this._device.importExternalTexture({ source: frame }) }
        ]
      });

      let commandEncoder = this._device!.createCommandEncoder();
      let textureView = this._renderingContext!.getCurrentTexture().createView();
      let passEncoder = commandEncoder.beginRenderPass({
        colorAttachments: [{
          view: textureView,
          clearValue: [1.0, 0.0, 0.0, 1.0],
          loadOp: "clear",
          storeOp: "store"
        }]
      });

      passEncoder.setPipeline(this._pipeline!);
      passEncoder.setBindGroup(0, binding!);
      passEncoder.draw(6, 1, 0, 0);
      passEncoder.end();
      this._device?.queue.submit([commandEncoder.finish()]);
    }

    frame.close();
  }

  private OnFrameDecodeFailed(error: Error) {
    throw error.message;
  }
}
