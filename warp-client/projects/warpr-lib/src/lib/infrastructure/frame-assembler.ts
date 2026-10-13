import { EncodedFrame, FrameType, VideoCodec } from "../data/frames";

enum VideoPacketFlags {
  None = 0,
  NalStart = 1, //Packet starts with a NAL unit
  NalEnd = 2    //Packet ends at a NAL unit boundary
}

class VideoPacketHeader {
  public static readonly Size = 24;

  public readonly FrameIndex: number;
  public readonly FrameSize: number;
  public readonly Offset: number;
  public readonly PacketIndex: number;
  public readonly PacketCount: number;
  public readonly Width: number;
  public readonly Height: number;
  public readonly Type: FrameType;
  public readonly Codec: VideoCodec;
  public readonly Flags: VideoPacketFlags;

  public constructor(view: DataView) {
    this.FrameIndex = view.getUint32(0, true);
    this.FrameSize = view.getUint32(4, true);
    this.Offset = view.getUint32(8, true);
    this.PacketIndex = view.getUint16(12, true);
    this.PacketCount = view.getUint16(14, true);
    this.Width = view.getUint16(16, true);
    this.Height = view.getUint16(18, true);
    this.Type = view.getUint8(20);
    this.Codec = view.getUint8(21);
    this.Flags = view.getUint8(22);
  }
}

class PartialFrame {
  public readonly Payloads: (Uint8Array | undefined)[];
  public readonly Flags: VideoPacketFlags[];
  public readonly FirstPacketTime = performance.now();
  public PacketsReceived = 0;

  public constructor(
    public readonly Header: VideoPacketHeader) {
    this.Payloads = new Array(Header.PacketCount);
    this.Flags = new Array(Header.PacketCount);
  }

  public get IsComplete(): boolean {
    return this.PacketsReceived === this.Header.PacketCount;
  }

  public AddPacket(header: VideoPacketHeader, payload: Uint8Array) {
    if (header.PacketIndex >= this.Payloads.length || this.Payloads[header.PacketIndex]) return;

    this.Payloads[header.PacketIndex] = payload;
    this.Flags[header.PacketIndex] = header.Flags;
    this.PacketsReceived++;
  }

  public ToFrame(): EncodedFrame | null {
    //Keep NAL units which arrived completely, a NAL unit may span multiple packets
    let parts: Uint8Array[] = [];
    let size = 0;
    for (let index = 0; index < this.Payloads.length; index++) {
      if (!this.Payloads[index] || !(this.Flags[index] & VideoPacketFlags.NalStart)) continue;

      let first = index;
      while (this.Payloads[index] && !(this.Flags[index] & VideoPacketFlags.NalEnd)) index++;
      if (!this.Payloads[index]) continue;

      for (let part = first; part <= index; part++) {
        parts.push(this.Payloads[part]!);
        size += this.Payloads[part]!.length;
      }
    }

    if (size === 0) return null;

    let bytes = new Uint8Array(size);
    let position = 0;
    for (let part of parts) {
      bytes.set(part, position);
      position += part.length;
    }

    let header = this.Header;
    return new EncodedFrame(header.FrameIndex, header.Type, header.Width, header.Height, header.Codec, bytes, this.IsComplete);
  }
}

//Reassembles frames from video packets received over an unordered, unreliable channel.
//Incomplete frames are emitted with their missing slices left out once a newer frame completes or they time out.
export class FrameAssembler {
  private static readonly _maxFrameDelay = 50;

  private _frames = new Map<number, PartialFrame>();
  private _lastFrameIndex?: number;
  private _flushTimer?: ReturnType<typeof setTimeout>;

  public constructor(
    private _onFrame: (frame: EncodedFrame) => void) { }

  public PushPacket(buffer: ArrayBuffer) {
    if (buffer.byteLength < VideoPacketHeader.Size) return;

    let header = new VideoPacketHeader(new DataView(buffer));
    if (this._lastFrameIndex !== undefined && !FrameAssembler.IsNewer(header.FrameIndex, this._lastFrameIndex)) return;

    let frame = this._frames.get(header.FrameIndex);
    if (!frame) {
      frame = new PartialFrame(header);
      this._frames.set(header.FrameIndex, frame);
    }

    frame.AddPacket(header, new Uint8Array(buffer, VideoPacketHeader.Size));

    if (frame.IsComplete) {
      this.EmitThrough(header.FrameIndex);
    }
    else {
      this.ScheduleFlush();
    }
  }

  public Reset() {
    clearTimeout(this._flushTimer);
    this._flushTimer = undefined;
    this._frames.clear();
    this._lastFrameIndex = undefined;
  }

  private static IsNewer(index: number, reference: number) {
    return ((index - reference) | 0) > 0;
  }

  //Emits all pending frames up to and including the given one in order
  private EmitThrough(frameIndex: number) {
    let indices = [...this._frames.keys()]
      .filter(p => !FrameAssembler.IsNewer(p, frameIndex))
      .sort((a, b) => (a - b) | 0);

    for (let index of indices) {
      let frame = this._frames.get(index)!;
      this._frames.delete(index);

      let result = frame.ToFrame();
      if (result) this._onFrame(result);
    }

    this._lastFrameIndex = frameIndex;
    this.ScheduleFlush();
  }

  private ScheduleFlush() {
    if (this._flushTimer !== undefined || this._frames.size === 0) return;

    let oldest = Math.min(...[...this._frames.values()].map(p => p.FirstPacketTime));
    let delay = Math.max(0, oldest + FrameAssembler._maxFrameDelay - performance.now());
    this._flushTimer = setTimeout(() => this.OnFlushTimer(), delay);
  }

  private OnFlushTimer() {
    this._flushTimer = undefined;

    let deadline = performance.now() - FrameAssembler._maxFrameDelay;
    let expired = [...this._frames.entries()].filter(([, frame]) => frame.FirstPacketTime <= deadline).map(([index]) => index);
    if (expired.length > 0) {
      let newest = expired.reduce((a, b) => FrameAssembler.IsNewer(b, a) ? b : a);
      this.EmitThrough(newest);
    }
    else {
      this.ScheduleFlush();
    }
  }
}
