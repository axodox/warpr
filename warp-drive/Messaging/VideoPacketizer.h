#pragma once
#include "warpr_includes.h"
#include "Encoder/VideoEncoder.h"

namespace Warpr::Messaging
{
  enum class VideoPacketFlags : uint8_t
  {
    None = 0,
    NalStart = 1, //Packet starts with a NAL unit
    NalEnd = 2    //Packet ends at a NAL unit boundary
  };

#pragma pack(push, 1)
  struct VideoPacketHeader
  {
    uint32_t FrameIndex = 0;
    uint32_t FrameSize = 0;
    uint32_t Offset = 0;
    uint16_t PacketIndex = 0;
    uint16_t PacketCount = 0;
    uint16_t Width = 0;
    uint16_t Height = 0;
    uint8_t Type = 0;  //Encoder::FrameType
    uint8_t Codec = 0; //Encoder::VideoCodec
    VideoPacketFlags Flags = VideoPacketFlags::None;
    uint8_t Reserved = 0;
  };
#pragma pack(pop)

  static_assert(sizeof(VideoPacketHeader) == 24, "VideoPacketHeader must be 24 bytes.");

  //Splits frames along NAL unit (slice) boundaries into packets which each fit into a single network packet,
  //so on an unreliable channel a lost packet only loses the slices it contains instead of the whole frame
  class VideoPacketizer
  {
  public:
    static const size_t MaxPacketSize = 1100;
    static const size_t MaxPayloadSize = MaxPacketSize - sizeof(VideoPacketHeader);

    void Packetize(const Encoder::EncodedFrame& frame, const std::function<void(const rtc::binary&)>& onPacket);

  private:
    struct PacketRange
    {
      size_t Offset = 0, Size = 0;
      VideoPacketFlags Flags = VideoPacketFlags::None;
    };

    uint32_t _frameIndex = 0;
    std::vector<size_t> _nalStarts;
    std::vector<PacketRange> _packets;
    rtc::binary _buffer;

    void FindNalStarts(std::span<const uint8_t> bytes);
    void PlanPackets(size_t frameSize);
  };
}
