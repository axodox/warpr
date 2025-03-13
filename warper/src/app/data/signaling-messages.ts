
export enum WarprSignalingMessageType {
  ConnectionRequest = "ConnectionRequest",
  PairingCompleteMessage = "PairingCompleteMessage",
  PeerConnectionDescriptionMessage = "PeerConnectionDescriptionMessage",
  PeerConnectionCandidateMessage = "PeerConnectionCandidateMessage",
  StreamingSourcesMessage = "StreamingSourcesMessage",
}

export class ConnectionRequest {
  public readonly $type = WarprSignalingMessageType.ConnectionRequest;
  public SessionId?: string;
}
export class PairingCompleteMessage {
  public readonly $type = WarprSignalingMessageType.PairingCompleteMessage;
}

export class PeerConnectionDescriptionMessage {
  public readonly $type = WarprSignalingMessageType.PeerConnectionDescriptionMessage;
  public Description?: string;
}

export class PeerConnectionCandidateMessage {
  public readonly $type = WarprSignalingMessageType.PeerConnectionCandidateMessage;
  public Candidate?: string;
}
export class StreamingSourcesMessage {
  public readonly $type = WarprSignalingMessageType.StreamingSourcesMessage;
  public Sources?: string[];
}

export type WarprSignalingMessage =
  ConnectionRequest |
  PeerConnectionDescriptionMessage |
  PeerConnectionCandidateMessage |
  StreamingSourcesMessage;
