# Warpr: slice-based loss resilience — handover

As of 2026-10-13 · Peter Major

## Status

Slice-based packetization works end to end. Lost packets now cost slices, not whole frames, and on H.264 (Firefox) the corruption heals. On H.265 (Chrome) the corrupted areas never heal; that is the one open problem.

- **Where the code is:** branch `slicing`, commit `1b84021` "WIP". The working tree was clean at handover.
- **Temporary diagnostics are in that commit** and must be removed before merging. Search for `DIAGNOSTIC`.
- **Resume at:** the diagnostic run under [Next steps](#next-steps). It decides whether the H.265 problem is in the encoder or in Chrome.
- **Build gotcha:** close desk-beamer before building warp-drive. Otherwise linking fails with `LNK1201` because `WarpDrive.pdb` is locked.

## Design

Each frame is sent as many packets of at most 1,100 bytes, cut at slice boundaries. A lost packet then loses only its slices, and intra refresh repaints them. Before, a frame was one message of up to 256 KB. The stream channel is unordered with zero retransmits, so one lost network packet dropped the whole frame.

1. **Encoder** (`VideoEncoder.cpp`). H.265 uses byte-capped slices: `sliceMode = 1`, `MaxSliceSize = 1000`. H.264 uses one slice per macroblock row (`sliceMode = 2`, `sliceModeData = 1`), because NVENC refuses byte-based slices for H.264 on this GPU. NVENC's slice-offset reporting is off and `EncodedFrame::Slices` is gone.
2. **Packetizer** (`Messaging/VideoPacketizer.*`). It scans for Annex B start codes to find every NAL unit, packs whole NALs into packets, and splits a NAL bigger than one packet across several.
3. **Transport** (`WebRtcClient::SendVideoPacket`). Each packet is one data-channel message on the `stream` channel; the generic `MessageSplitter` is no longer used for video.
4. **Assembler** (`infrastructure/frame-assembler.ts`). It groups packets by frame. A complete frame is emitted at once. An incomplete one is emitted when a newer frame completes or after 50 ms. A NAL with any piece missing is left out, and late packets for an emitted frame are dropped.
5. **Decoder** (`stream-host.component.ts`). It decodes whatever arrived. A decoder error still requests a key frame. A codec is not blacklisted after a failure if its decoder was fed incomplete frames.

Packet header, 24 bytes, little-endian, before each payload:

| Field | Type | Notes |
| --- | --- | --- |
| FrameIndex | uint32 | Packetizer counter, never reset by encoder re-init |
| FrameSize | uint32 | Bytes in the whole frame |
| Offset | uint32 | Payload position within the frame |
| PacketIndex, PacketCount | uint16 each | |
| Width, Height | uint16 each | |
| Type, Codec | uint8 each | `FrameType`, `VideoCodec` |
| Flags | uint8 | 1 = starts a NAL, 2 = ends at a NAL boundary |
| Reserved | uint8 | |

Every packet carries the frame metadata, so a frame can be rebuilt even when its first packet is lost. The assembler passed a randomized test: about 1,200 frames with shuffled order, 10% loss and multi-packet NALs. That test lived in the scratchpad and is not in the repo.

## Changes in the WIP commit

The WIP commit changes 14 files on top of `239d324` "Fix performance". The wire format changed, so server and browser must ship together.

| File | Change |
| --- | --- |
| `warp-drive/Messaging/VideoPacketizer.h`, `.cpp` | New: packet header and slice-aware packetizer |
| `warp-drive/Encoder/VideoEncoder.cpp`, `.h` | Slice modes per codec; infinite GOP; intra refresh 60/30 constants; NVENC errors now include status and `nvEncGetLastErrorString` |
| `warp-drive/Core/WarpSession.cpp`, `.h` | Sends frames through `VideoPacketizer` |
| `warp-drive/Messaging/WebRtcClient.cpp`, `.h` | `SendVideoFrame` replaced by `SendVideoPacket`; stream splitter removed |
| `warp-drive/warp-drive.vcxproj`, `.filters` | New files registered |
| `warpr-lib/.../infrastructure/frame-assembler.ts` | New: rebuilds frames from packets |
| `warpr-lib/.../data/frames.ts` | `EncodedFrame` is now a plain object with `IsComplete` |
| `warpr-lib/.../services/streaming.service.ts` | Stream channel feeds the assembler |
| `warpr-lib/.../stream-host/stream-host.component.ts` | Incomplete-frame guard on codec fallback |

Remove before merging (all marked `DIAGNOSTIC`):

- [ ] `VideoEncoder.cpp`: `.getRCStats = true` and the once-a-second `DIAG` log block in `EncodeFrame`
- [ ] `VideoEncoder.h`: the `_diag*` members
- [ ] `stream-host.component.ts`: the `?codec=` URL override in `GetSupportedCodecsAsync`
- [ ] Squash or reword the "WIP" commit

## Open problem: H.265 in Chrome does not heal

On H.265 in Chrome, areas lost from a frame stay corrupted or grey indefinitely. On H.264 in Firefox the same losses heal. On master, H.265 refresh did clean up the image, so it worked with the old 2048-fixed-slice layout and the preset's default GOP.

What is known:

- **H.264 refresh only started working after two fixes** from the NVENC header. `gopLength` must be `NVENC_INFINITE_GOPLENGTH`, otherwise periodic refresh is disabled. And `intraRefreshCnt` must be smaller than `intraRefreshPeriod`; both were 120. Now `gopLength` is infinite and the values are 60/30.
- **The main H.265 change since master is the slice layout,** from 2048 fixed slices to byte-capped slices of at most 1,000 bytes. NVENC builds intra refresh on slices (see `singleSliceIntraRefresh` in the header).
- **Loss looks high for a local link.** A 4K key frame is now around a thousand packets sent in one burst with zero retransmits. The grey areas are probably slices lost from the first key frame.

Hypotheses, most likely first:

1. NVENC silently disables H.265 intra refresh with byte-based slices.
2. Chrome's H.265 decoder handles missing slices badly. For example, a lost first slice (`first_slice_segment_in_pic_flag`) may drop or corrupt the picture, or errors trigger key frames that are themselves lossy.
3. The refresh runs, but corruption leaks back from not-yet-refreshed areas through motion vectors.

## Next steps

Run the diagnostics first; they decide between the hypotheses before any setting changes.

- [ ] Close desk-beamer, rebuild warp-drive and the `warpr-lib` package.
- [ ] Stream to Chrome on H.265 for a few seconds and copy the `DIAG H265: …` lines from the desk-beamer log.
- [ ] Open the Chrome console and note any decoder errors.
- [ ] Reload Chrome with `?codec=H264`, watch whether corruption heals, and copy the `DIAG H264` lines.

How to read the results:

| Observation | Means | Fix |
| --- | --- | --- |
| H.265: about 0% intra blocks, 0 refresh starts per second | NVENC drops refresh with byte slices (hypothesis 1) | Give H.265 one slice per CTU row (`sliceMode = 2`, `sliceModeData = 1`), like H.264 |
| H.265: steady intra share, about 1 refresh start per second, no healing | Encoder fine; problem is Chrome's decoder (hypothesis 2 or 3) | Check console errors; consider resending a frame's first packet, or requesting a key frame when one arrives incomplete |
| Chrome heals on H.264 but not H.265 | H.265-specific | As the first two rows |
| Chrome does not heal on H.264 either | Chrome-specific | Look at how Chrome's decoder treats incomplete frames |

Separately, reduce loss on key-frame bursts. The cheapest option is configuration only: set the gateway's `StreamChannelReliability` to `MaxPacketLifetime` of about 30–50 ms instead of `MaxRetransmits = 0`, so lost packets get resent while they are still useful. The other option is pacing a key frame's packets over a few milliseconds.

## Background worth remembering

These were found earlier on the same work and explain choices that look odd out of context.

- **Codec negotiation.** The browser sends `VideoCodecPreferenceMessage` once on connect; the server picks the first codec its GPU supports. Firefox's `isConfigSupported` sometimes reports H.265 as supported when it cannot decode it. The client therefore drops a codec whose decoder fails before producing a frame, and sends the server an updated list.
- **Firefox rendering.** Drawing 4K frames on a 2D canvas round-tripped each frame through the CPU, which capped Firefox at about 15 fps. WebGL rendering (`video-frame-renderer.ts`) keeps frames on the GPU; Firefox now holds 60 fps at 4K.
- **Encoder rebuild bug (fixed).** `ReleaseResources` now clears `_frameResource`. Without that, switching codec at the same frame size failed every frame.
- **NVENC constraints found so far:**
    - Byte-based slices are refused for H.264 ("Invalid Slice Mode. Byte based slice encoding is not supported.").
    - Periodic intra refresh needs `gopLength = NVENC_INFINITE_GOPLENGTH`.
    - `intraRefreshCnt` must be below `intraRefreshPeriod`.
- **H.264 `bitstreamRestrictionFlag = 1`** is kept. It tells decoders no frames are reordered, which helps any decoder that would otherwise buffer.
