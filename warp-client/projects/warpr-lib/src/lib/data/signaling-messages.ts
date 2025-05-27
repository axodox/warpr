
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

  public IceServers?: string[];
}

export function GetConfiguration(iceServers?: string[]): RTCConfiguration {
  let result: RTCConfiguration = {};
  result.iceServers = new Array<RTCIceServer>();

  if (iceServers) {
    let regex = /^(?<protocol>stun|turn):(?:(?<username>\w+):(?<password>\w+)@)?(?<url>[\w\.\-:?=&]+)$/im;
    for (let server of iceServers) {
      let match = regex.exec(server);
      if (!match || !match.groups) {
        console.warn(`Failed to parse ICE server URI ${server}.`);
        continue;
      }

      result.iceServers.push({
        urls: `${match.groups['protocol']}:${match.groups['url']}`,
        username: match.groups['username'],
        credential: match.groups['password']
      });
    }
  }

  return result;
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
  PairingCompleteMessage |
  PeerConnectionDescriptionMessage |
  PeerConnectionCandidateMessage |
  StreamingSourcesMessage;
