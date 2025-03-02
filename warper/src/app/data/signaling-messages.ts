
export enum WarprMessageType {
  ConnectionRequest = "ConnectionRequest",
  PairingCompleteMessage = "PairingCompleteMessage",
  PeerConnectionDescriptionMessage = "PeerConnectionDescriptionMessage",
  PeerConnectionCandidateMessage = "PeerConnectionCandidateMessage",
  StreamingSourcesMessage = "StreamingSourcesMessage",
}

export class ConnectionRequest {
  public readonly $type = WarprMessageType.ConnectionRequest;
  public SessionId?: string;
}
export class PairingCompleteMessage {
  public readonly $type = WarprMessageType.PairingCompleteMessage;
}

export class PeerConnectionDescriptionMessage {
  public readonly $type = WarprMessageType.PeerConnectionDescriptionMessage;
  public Description?: string;
}

export class PeerConnectionCandidateMessage {
  public readonly $type = WarprMessageType.PeerConnectionCandidateMessage;
  public Candidate?: string;
}
export class StreamingSourcesMessage {
  public readonly $type = WarprMessageType.StreamingSourcesMessage;
  public Sources?: string[];
}

export type WarprMessage =
  ConnectionRequest |
  PeerConnectionDescriptionMessage |
  PeerConnectionCandidateMessage |
  StreamingSourcesMessage;
