import { ArrayStream } from "../insfrastructure/array-stream";

export enum FrameType {
  Unknown,
  Key,
  Delta
}

export class EncodedFrame {
  public Index: number;
  public Type: FrameType;
  public Bytes: ArrayBuffer;

  public constructor(buffer: ArrayBuffer) {
    let stream = new ArrayStream(buffer);
    this.Index = stream.ReadUInt32();
    this.Type = stream.ReadUInt32();
    this.Bytes = stream.ReadBuffer();
  }
}
