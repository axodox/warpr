
export enum WarprMessageType {
  ConnectionRequest = "ConnectionRequest",
  PeerConnectionDescriptionMessage = "PeerConnectionDescriptionMessage",
  PeerConnectionCandidateMessage = "PeerConnectionCandidateMessage",
  StreamingSourcesMessage = "StreamingSourcesMessage",
}

export class ConnectionRequest {
  public readonly Type = WarprMessageType.ConnectionRequest;
  public SessionId?: string;
}

export class PeerConnectionDescriptionMessage {
  public readonly Type = WarprMessageType.PeerConnectionDescriptionMessage;
  public Description?: string;
}

export class PeerConnectionCandidateMessage {
  public readonly Type = WarprMessageType.PeerConnectionCandidateMessage;
  public Description?: string;
}
export class StreamingSourcesMessage {
  public readonly Type = WarprMessageType.StreamingSourcesMessage;
  public Sources?: string[];
}

export type WarprMessage =
  ConnectionRequest |
  PeerConnectionDescriptionMessage |
  PeerConnectionCandidateMessage |
  StreamingSourcesMessage;
