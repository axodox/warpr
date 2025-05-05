export enum WarprStreamingMessageType {
  PointerInputMessage = "PointerInputMessage",
  ResizeSurfaceMessage = "ResizeSurfaceMessage",
  RequestKeyFrameMessage = "RequestKeyFrameMessage",
}

export type Point = {
  X: number;
  Y: number;
}

export enum PointerType {
  Unknown = "Unknown",
  Mouse = "Mouse",
  Pen = "Pen",
  Touch = "Touch"
}

export enum PointerAction {
  Unknown = "Unknown",
  Pressed = "Pressed",
  Moved = "Moved",
  Released = "Released",
  WheelChanged = "WheelChanged"
}

export enum PointerFlags {
  None = 0,
  LeftButton = 1,
  MiddleButton = 2,
  RightButton = 4,
  Wheel = 8,
  Captured = 16
};

export class PointerState {
  public Buttons = 0;
}

export type PointerStates = { [key: number]: PointerState };

export class PointerInputMessage {
  public readonly $type = WarprStreamingMessageType.PointerInputMessage;

  public Id = -1;
  public Type = PointerType.Unknown;
  public Action = PointerAction.Unknown;
  public Position: Point = { X: 0, Y: 0 };
  public Flags = PointerFlags.None;
  public WheelDelta = 0;

  constructor(event: PointerEvent, states: PointerStates, target: HTMLCanvasElement, wheelDelta: number) {

    let state = states[event.pointerId];
    if (state === undefined) {
      state = new PointerState();
      states[event.pointerId] = state;
    }

    this.Id = event.pointerId;

    switch (event.pointerType) {
      case "mouse":
        this.Type = PointerType.Mouse;
        break;
      case "pen":
        this.Type = PointerType.Pen;
        break;
      case "touch":
        this.Type = PointerType.Touch;
        break;
      default:
        this.Type = PointerType.Mouse;
        break;
    }

    switch (event.type) {
      case "pointerdown":
        this.Action = PointerAction.Pressed;
        state.Buttons |= 1 << event.button
        break;
      case "pointerup":
        this.Action = PointerAction.Released;
        state.Buttons &= ~(1 << event.button)
        break;
      case "pointermove":
        this.Action = PointerAction.Moved;
        break;
      case "wheel":
        this.Action = PointerAction.WheelChanged;
        break;
    }

    if (state.Buttons & 1) this.Flags |= PointerFlags.LeftButton;
    if (state.Buttons & 2) this.Flags |= PointerFlags.MiddleButton;
    if (state.Buttons & 4) this.Flags |= PointerFlags.RightButton;
    if (wheelDelta != 0) this.Flags |= PointerFlags.Wheel;
    if (target.hasPointerCapture(event.pointerId)) this.Flags |= PointerFlags.Captured;
    
    this.Position = PointerInputMessage.GetRemotePosition(event, target);
    this.WheelDelta = -wheelDelta;
  }

  private static GetRemotePosition(event: PointerEvent, target: HTMLCanvasElement): Point {
    let offsetX = 0;
    let offsetY = 0;
    let scale = 1;
    let canvasAspectRatio = target.clientWidth / target.clientHeight;
    let videoAspectRatio = target.width / target.height;

    if (videoAspectRatio > canvasAspectRatio) {
      offsetY = 0.5 * (target.clientHeight - target.clientWidth / videoAspectRatio);
      scale = target.width / target.clientWidth;
    } else {
      offsetX = 0.5 * (target.clientWidth - target.clientHeight * videoAspectRatio);
      scale = target.height / target.clientHeight;
    }

    return {
      X: (event.x - offsetX) * scale,
      Y: (event.y - offsetY) * scale
    };
  }
}

export class ResizeSurfaceMessage {
  public readonly $type = WarprStreamingMessageType.ResizeSurfaceMessage;

  constructor(
    public Width: number,
    public Height: number
  ) { }
}

export class RequestKeyFrameMessage {
  public readonly $type = WarprStreamingMessageType.RequestKeyFrameMessage;
}

export type WarprStreamingMessage =
  PointerInputMessage |
  ResizeSurfaceMessage |
  RequestKeyFrameMessage;
