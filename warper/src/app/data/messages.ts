
export enum WarprMessageType {
  StreamingSourcesMessage = "StreamingSourcesMessage",
  StreamSignalingMessage = "StreamSignalingMessage"
}

export class StreamingSourcesMessage {
  public readonly Type = WarprMessageType.StreamingSourcesMessage;
  public Sources?: string[];
}

export class StreamSignalingMessage {
  public readonly Type = WarprMessageType.StreamSignalingMessage;
  public Source?: string;
  public Data?: string;
}

export type WarprMessage =
  StreamingSourcesMessage |
  StreamSignalingMessage;
