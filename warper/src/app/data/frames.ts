import { ArrayStream } from "../insfrastructure/array-stream";

export enum FrameType {
  Unknown,
  Key,
  Delta
}

export class EncodedFrame {
  public Index: number;
  public Type: FrameType;
  public Width: number;
  public Height: number;
  public Bytes: ArrayBuffer;

  public constructor(buffer: ArrayBuffer) {
    let stream = new ArrayStream(buffer);

    this.Type = stream.ReadUInt32();
    this.Index = stream.ReadUInt32();
    this.Width = stream.ReadUInt32();
    this.Height = stream.ReadUInt32();
    this.Bytes = stream.ReadBuffer();
  }
}
