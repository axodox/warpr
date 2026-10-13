#include "warpr_includes.h"
#include "VideoPacketizer.h"

using namespace Warpr::Encoder;
using namespace std;

namespace
{
  Warpr::Messaging::VideoPacketFlags operator|(Warpr::Messaging::VideoPacketFlags a, Warpr::Messaging::VideoPacketFlags b)
  {
    return Warpr::Messaging::VideoPacketFlags(uint8_t(a) | uint8_t(b));
  }
}

namespace Warpr::Messaging
{
  void VideoPacketizer::Packetize(const Encoder::EncodedFrame& frame, const std::function<void(const rtc::binary&)>& onPacket)
  {
    FindNalStarts(frame.Bytes);
    PlanPackets(frame.Bytes.size());

    VideoPacketHeader header{
      .FrameIndex = _frameIndex++,
      .FrameSize = uint32_t(frame.Bytes.size()),
      .PacketCount = uint16_t(_packets.size()),
      .Width = uint16_t(frame.Width),
      .Height = uint16_t(frame.Height),
      .Type = uint8_t(frame.Type),
      .Codec = uint8_t(frame.Codec)
    };

    for (auto& packet : _packets)
    {
      header.Offset = uint32_t(packet.Offset);
      header.Flags = packet.Flags;

      _buffer.resize(sizeof(VideoPacketHeader) + packet.Size);
      memcpy(_buffer.data(), &header, sizeof(VideoPacketHeader));
      memcpy(_buffer.data() + sizeof(VideoPacketHeader), frame.Bytes.data() + packet.Offset, packet.Size);
      onPacket(_buffer);

      header.PacketIndex++;
    }
  }

  void VideoPacketizer::FindNalStarts(std::span<const uint8_t> bytes)
  {
    _nalStarts.clear();

    //Annex B start codes (00 00 01 or 00 00 00 01), emulation prevention ensures they do not occur inside NAL units
    for (size_t i = 0; i + 2 < bytes.size(); i++)
    {
      if (bytes[i] != 0 || bytes[i + 1] != 0 || bytes[i + 2] != 1) continue;

      _nalStarts.push_back(i > 0 && bytes[i - 1] == 0 ? i - 1 : i);
      i += 2;
    }

    if (_nalStarts.empty() || _nalStarts.front() != 0) _nalStarts.insert(_nalStarts.begin(), 0);
  }

  void VideoPacketizer::PlanPackets(size_t frameSize)
  {
    _packets.clear();

    //Pack whole NAL units into packets, split the ones which do not fit into a single packet
    PacketRange current{ .Flags = VideoPacketFlags::NalStart | VideoPacketFlags::NalEnd };
    for (size_t i = 0; i < _nalStarts.size(); i++)
    {
      auto nalStart = _nalStarts[i];
      auto nalSize = (i + 1 < _nalStarts.size() ? _nalStarts[i + 1] : frameSize) - nalStart;

      if (nalSize <= MaxPayloadSize)
      {
        if (current.Size + nalSize > MaxPayloadSize)
        {
          _packets.push_back(current);
          current.Size = 0;
        }

        if (current.Size == 0) current.Offset = nalStart;
        current.Size += nalSize;
      }
      else
      {
        if (current.Size > 0)
        {
          _packets.push_back(current);
          current.Size = 0;
        }

        for (size_t position = 0; position < nalSize; position += MaxPayloadSize)
        {
          auto flags = VideoPacketFlags::None;
          if (position == 0) flags = flags | VideoPacketFlags::NalStart;
          if (position + MaxPayloadSize >= nalSize) flags = flags | VideoPacketFlags::NalEnd;

          _packets.push_back({
            .Offset = nalStart + position,
            .Size = min(MaxPayloadSize, nalSize - position),
            .Flags = flags
            });
        }
      }
    }

    if (current.Size > 0) _packets.push_back(current);
  }
}
