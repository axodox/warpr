export enum FrameType {
  Unknown,
  Key,
  Delta
}

export enum VideoCodec {
  Unknown,
  H264,
  H265
}

export class EncodedFrame {
  public constructor(
    public Index: number,
    public Type: FrameType,
    public Width: number,
    public Height: number,
    public Codec: VideoCodec,
    public Bytes: Uint8Array,
    public IsComplete: boolean) { }
}

export class Size {
  public constructor(
    public Width: number,
    public Height: number) { }

  public static Empty: Size = { Width: 0, Height: 0 };

  public static AreEqual(a: Size, b: Size) {
    return a.Width === b.Width && a.Height === b.Height;
  }
}
