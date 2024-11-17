@group(0) @binding(1) var mySampler: sampler;
@group(0) @binding(2) var myTexture: texture_external;
    
@fragment
fn frag_main(@location(0) uv : vec2<f32>) -> @location(0) vec4<f32> {
  return textureSampleBaseClampToEdge(myTexture, mySampler, uv);
}
