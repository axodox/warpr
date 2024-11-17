export class ArrayStream {
  private _view: DataView;
  private _position: number;

  public constructor(buffer: ArrayBuffer) {
    this._view = new DataView(buffer);
    this._position = 0;
  }

  public get Position(): number {
    return this._position;
  }

  public set Position(value: number) {
    if (value < 0 || this._view.buffer.byteLength < value) {
      throw new RangeError("The provided value lies outside the stream's bounds.");
    }

    this._position = value;
  }

  public ReadBool(): boolean {
    let result = this._view.getUint8(this._position) != 0;
    this._position += 1;
    return result;
  }

  public ReadUInt8(): number {
    let result = this._view.getUint8(this._position);
    this._position += 1;
    return result;
  }

  public ReadUInt16(): number {
    let result = this._view.getUint16(this._position, true);
    this._position += 2;
    return result;
  }

  public ReadUInt32(): number {
    let result = this._view.getUint32(this._position, true);
    this._position += 4;
    return result;
  }

  public ReadUInt64(): bigint {
    let result = this._view.getBigUint64(this._position, true);
    this._position += 8;
    return result;
  }

  public ReadInt8(): number {
    let result = this._view.getInt8(this._position);
    this._position += 1;
    return result;
  }

  public ReadInt16(): number {
    let result = this._view.getInt16(this._position, true);
    this._position += 2;
    return result;
  }

  public ReadInt32(): number {
    let result = this._view.getInt32(this._position, true);
    this._position += 4;
    return result;
  }

  public ReadInt64(): bigint {
    let result = this._view.getBigInt64(this._position, true);
    this._position += 8;
    return result;
  }

  public ReadSingle(): number {
    let result = this._view.getFloat32(this._position, true);
    this._position += 4;
    return result;
  }

  public ReadDouble(): number {
    let result = this._view.getFloat64(this._position, true);
    this._position += 8;
    return result;
  }

  public ReadBuffer(): ArrayBuffer {
    let length = this.ReadUInt32();
    let result = this._view.buffer.slice(this._position, length);
    this._position += length;
    return result;
  }

  public ReadString() {
    let length = this.ReadUInt32();
    let decoder = new TextDecoder();
    let result = decoder.decode(this._view.buffer.slice(this._position, length));
    this._position += length;
    return result;
  }
}
