import { ArrayStream } from "./array-stream";

class MessageBuilder {
  public readonly FragmentCount: number;
  public FragmentsReady: number = 0;
  public Buffer: Uint8Array;

  constructor(
    public readonly Id: number,
    public readonly Size: number,
    public readonly FragmentSize: number
  ) {
    this.FragmentCount = Math.ceil(Size / FragmentSize);
    this.Buffer = new Uint8Array(Size);
  }

  AddFragment(index: number, buffer: Uint8Array): boolean {
    if (this.FragmentCount == this.FragmentsReady) throw RangeError("All message parts have been already collected.");

    this.Buffer.set(buffer, this.FragmentSize * index);
    this.FragmentsReady++;

    return this.FragmentCount == this.FragmentsReady;
  }
}

export class MessageAssembler {

  private _builders = new Map<number, MessageBuilder>();

  public constructor(private _maxBuilderCount = 3) { }

  public PushMessage(buffer: ArrayBuffer): ArrayBuffer | null {
    let stream = new ArrayStream(buffer);
    let messageIndex = stream.ReadUInt32();
    let messageSize = stream.ReadUInt32();
    let fragmentSize = stream.ReadUInt32();
    let fragmentIndex = stream.ReadUInt32();
    let fragment = new Uint8Array(stream.ReadToEnd());

    let builder = this._builders.get(messageIndex);
    if (builder === undefined) {
      builder = new MessageBuilder(messageIndex, messageSize, fragmentSize);
      this._builders.set(messageIndex, builder);

      if (this._builders.size > this._maxBuilderCount) {
        let [oldest] = this._builders.keys();
        this._builders.delete(oldest);
        console.log("Message lost.");
      }
    }

    if (builder.AddFragment(fragmentIndex, fragment)) {
      this._builders.delete(messageIndex);
      return builder.Buffer.buffer;
    } else {
      return null;
    }
  }
}
