export interface IVideoFrameRenderer {
  readonly Name: string;
  Draw(frame: VideoFrame): void;
}

export function CreateVideoFrameRenderer(canvas: HTMLCanvasElement): IVideoFrameRenderer | null {
  return WebGLVideoFrameRenderer.TryCreate(canvas) ?? Canvas2DVideoFrameRenderer.TryCreate(canvas);
}

//Keeps decoded frames on the GPU, a 2D canvas round-trips each frame through the CPU in some browsers (e.g. Firefox)
class WebGLVideoFrameRenderer implements IVideoFrameRenderer {
  private static readonly _vertexShaderSource = `
    attribute vec2 xy;
    varying highp vec2 uv;

    void main(void) {
      gl_Position = vec4(xy, 0.0, 1.0);
      uv = vec2((1.0 + xy.x) / 2.0, (1.0 - xy.y) / 2.0);
    }
  `;

  private static readonly _fragmentShaderSource = `
    varying highp vec2 uv;
    uniform sampler2D frame;

    void main(void) {
      gl_FragColor = texture2D(frame, uv);
    }
  `;

  public readonly Name: string;

  private constructor(
    private _context: WebGLRenderingContext | WebGL2RenderingContext) {
    this.Name = _context instanceof WebGL2RenderingContext ? "WebGL 2" : "WebGL";
    let gl = _context;

    let program = gl.createProgram()!;
    gl.attachShader(program, WebGLVideoFrameRenderer.CompileShader(gl, gl.VERTEX_SHADER, WebGLVideoFrameRenderer._vertexShaderSource));
    gl.attachShader(program, WebGLVideoFrameRenderer.CompileShader(gl, gl.FRAGMENT_SHADER, WebGLVideoFrameRenderer._fragmentShaderSource));
    gl.linkProgram(program);
    if (!gl.getProgramParameter(program, gl.LINK_STATUS)) throw new Error(gl.getProgramInfoLog(program) ?? "Failed to link program.");
    gl.useProgram(program);

    //Full screen quad
    gl.bindBuffer(gl.ARRAY_BUFFER, gl.createBuffer());
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array([-1, -1, -1, +1, +1, +1, +1, -1]), gl.STATIC_DRAW);

    let xyLocation = gl.getAttribLocation(program, "xy");
    gl.vertexAttribPointer(xyLocation, 2, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(xyLocation);

    //Frame texture
    gl.bindTexture(gl.TEXTURE_2D, gl.createTexture());
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
  }

  public static TryCreate(canvas: HTMLCanvasElement): IVideoFrameRenderer | null {
    let options: WebGLContextAttributes = { alpha: false, antialias: false, depth: false, stencil: false };
    let context = canvas.getContext("webgl2", options) ?? canvas.getContext("webgl", options);
    if (!context) return null;

    try {
      return new WebGLVideoFrameRenderer(context);
    }
    catch (error) {
      console.warn("Failed to initialize WebGL renderer.", error);
      return null;
    }
  }

  private static CompileShader(gl: WebGLRenderingContext, type: number, source: string): WebGLShader {
    let shader = gl.createShader(type)!;
    gl.shaderSource(shader, source);
    gl.compileShader(shader);
    if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) throw new Error(gl.getShaderInfoLog(shader) ?? "Failed to compile shader.");
    return shader;
  }

  public Draw(frame: VideoFrame) {
    let gl = this._context;
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, frame);
    gl.viewport(0, 0, gl.drawingBufferWidth, gl.drawingBufferHeight);
    gl.drawArrays(gl.TRIANGLE_FAN, 0, 4);
  }
}

class Canvas2DVideoFrameRenderer implements IVideoFrameRenderer {
  public readonly Name = "2D";

  private constructor(
    private _context: CanvasRenderingContext2D) { }

  public static TryCreate(canvas: HTMLCanvasElement): IVideoFrameRenderer | null {
    let context = canvas.getContext("2d");
    return context ? new Canvas2DVideoFrameRenderer(context) : null;
  }

  public Draw(frame: VideoFrame) {
    this._context.drawImage(frame, 0, 0, frame.displayWidth, frame.displayHeight);
  }
}
