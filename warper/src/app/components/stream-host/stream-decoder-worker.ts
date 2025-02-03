export class EncodedVideoFrame {
  constructor(
    public Chunk: EncodedVideoChunk,
    public Width: number,
    public Height: number) { }
}

let Width = 0;
let Height = 0;

let isDecoderInitialized = false;
let decoder = new VideoDecoder({
  output: (frame) => { postMessage(frame); frame.close(); },
  error: (error) => console.log(error)
});

self.onmessage = (event) => {
  let frame = event.data as EncodedVideoFrame;

  //Initialize decoder
  if (frame.Width != Width || frame.Height != Height) {
    decoder.configure({
      codec: "hvc1.1.6.L93.B0",
      codedWidth: frame.Width,
      codedHeight: frame.Height,
      hardwareAcceleration: "prefer-hardware",
      optimizeForLatency: true
    });

    Width = frame.Width;
    Height = frame.Height;
  }

  //Decode frame
  if (!isDecoderInitialized && frame.Chunk.type != "key") return;
  isDecoderInitialized = true;

  decoder.decode(frame.Chunk);
};
