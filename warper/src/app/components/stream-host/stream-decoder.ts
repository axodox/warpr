import { EncodedFrame, FrameType } from "../../data/frames";

let isDecoderInitialized = false;
let decoder = new VideoDecoder({
  output: (frame) => { postMessage(frame); frame.close(); },
  error: (error) => console.log(error)
});

decoder.configure({
  codec: "hvc1.1.6.L93.B0",
  codedWidth: 1920,
  codedHeight: 1080,
  hardwareAcceleration: "prefer-hardware"
});

self.onmessage = (event) => {
  let chunk = event.data as EncodedVideoChunk;
  if (!isDecoderInitialized && chunk.type != "key") return;

  isDecoderInitialized = true;

  decoder.decode(chunk);
};
