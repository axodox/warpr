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
  Released = "Released"
}

export class PointerInputMessage {
  public PointerId = -1;
  public PointerType = PointerType.Unknown;
  public PointerAction = PointerAction.Unknown;
  public Position: Point = { X: 0, Y: 0 };

  constructor(event: PointerEvent) {

    this.PointerId = event.pointerId;

    switch (event.pointerType) {
      case "mouse":
        this.PointerType = PointerType.Mouse;
        break;
      case "pen":
        this.PointerType = PointerType.Pen;
        break;
      case "touch":
        this.PointerType = PointerType.Touch;
        break;
    }

    switch (event.type) {
      case "pointerdown":
        this.PointerAction = PointerAction.Pressed;
        break;
      case "pointerup":
        this.PointerAction = PointerAction.Released;
        break;
      case "pointermove":
        this.PointerAction = PointerAction.Released;
        break;
    }

    this.Position = PointerInputMessage.GetRemotePosition(event);
  }

  private static GetRemotePosition(event: PointerEvent): Point {
    let canvas = event.target as HTMLCanvasElement;

    let offsetX = 0;
    let offsetY = 0;
    let scale = 1;
    let canvasAspectRatio = canvas.clientWidth / canvas.clientHeight;
    let videoAspectRatio = canvas.width / canvas.height;

    if (videoAspectRatio > canvasAspectRatio) {
      offsetY = 0.5 * (canvas.clientHeight - canvas.clientWidth / videoAspectRatio);
      scale = canvas.width / canvas.clientWidth;
    } else {
      offsetX = 0.5 * (canvas.clientWidth - canvas.clientHeight * videoAspectRatio);
      scale = canvas.height / canvas.clientHeight;
    }

    return {
      X: (event.x - offsetX) * scale,
      Y: (event.y - offsetY) * scale
    };
  }
}
