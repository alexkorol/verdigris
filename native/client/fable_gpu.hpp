#pragma once

// D3D11 world pass, ported from docs/reference/fable-demo. The caller owns
// authoritative scene assembly and the one shared terrain height sampler.
// Textures are straight RGBA8; the output is a top-down BGRA8 image for GDI.
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#include <wrl/client.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <climits>
#include <cstdint>
#include <cstring>
#include <limits>
#include <map>
#include <span>
#include <string>
#include <vector>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "gdi32.lib")

namespace fable_gpu {
struct Camera {
  int width = 0, height = 0;
  float cam_x = 0, cam_y = 0, d0 = 0, k = 0, a = 0, horizon = 0, dzp = 0;
  float near_depth = 40, far_depth = 1;
};
struct Vertex { float x = 0, y = 0, elevation = 0, u = 0, v = 0; };
struct Terrain {
  std::uint64_t texture = 0;
  std::span<const Vertex> vertices{};
  std::span<const std::uint32_t> indices{};
  bool nearest = false;
  float opacity = 1;
};
using Mesh = Terrain;
struct Sprite {
  std::uint64_t texture = 0;
  float x = 0, y = 0, elevation = 0, width = 1, height = 1;
  float anchor_x = .5f, anchor_y = 1, opacity = 1;
  float tint_r = 1, tint_g = 1, tint_b = 1;
  bool flip = false;
  float flash = 0;
  bool additive = false;
  float rotation = 0;  // Clockwise radians in the screen billboard plane.
  bool ground_layer = false;
};
struct Light {
  float x = 0, y = 0, elevation = 0, radius = 1;
  float r = 1, g = 1, b = 1, intensity = 1;
};
struct Cloud {
  // Full-resolution screen pixels, never projected from world coordinates.
  float screen_x = 0, screen_y = 0, radius = 1;
  float r = 196.f / 255, g = 198.f / 255, b = 208.f / 255, strength = 1;
};
struct Scene {
  Camera camera{};
  Terrain terrain{};
  std::span<const Sprite> sprites{};
  std::span<const Light> lights{};
  float ambient_r = 1, ambient_g = 1, ambient_b = 1;
  float sky_r = .38f, sky_g = .39f, sky_b = .40f;
  float dof_strength = 1, vignette = .18f;
  std::span<const Mesh> world_meshes{};
  std::span<const Mesh> ground_meshes{};  // Contact shadows/corpses before standing geometry.
  std::span<const Cloud> clouds{};
};
struct Stats {
  std::uint64_t frames = 0, texture_uploads = 0, texture_hits = 0, texture_evictions = 0;
  std::size_t texture_count = 0, texture_bytes = 0;
  std::uint32_t draw_calls = 0, terrain_triangles = 0, sprites = 0;
  double render_readback_ms = 0;
  bool hardware = false;
  std::uint32_t feature_level = 0, vendor_id = 0, device_id = 0;
};

namespace detail {
template<class T> using Com = Microsoft::WRL::ComPtr<T>;
inline constexpr std::size_t kTextures = 512, kTextureBytes = 256u * 1024u * 1024u;
inline constexpr std::size_t kVertices = 1000000, kIndices = 3000000, kSprites = 8192, kMeshes = 8192;
inline constexpr std::size_t kLights = 32;
inline constexpr std::size_t kClouds = 8;
struct GpuVertex {
  float x, y, elevation, u, v;
  float r = 1, g = 1, b = 1, alpha = 1;
  float sprite_width = 0, sprite_height = 0, flash = 0, mode = 0;
};
struct alignas(16) Constants {
  float viewport[4]{};  // W,H,camX,camY
  float projection[4]{};  // D0,K,A,HOR
  float depth[4]{};  // DZP,near,far,DoF strength
  float sky[4]{};  // RGB,vignette
  float ambient[4]{};  // RGB,light count
  float light_position[kLights][4]{};  // full-resolution screen x,y,radius,intensity
  float light_color[kLights][4]{};
  float cloud_count[4]{};
  float cloud_position[kClouds][4]{};  // full-resolution screen x,y,radius,strength
  float cloud_color[kClouds][4]{};
};
static_assert(sizeof(Constants) % 16 == 0);
inline constexpr const char* kShader = R"HLSL(
cbuffer Frame : register(b0) {
  float4 viewport; float4 projection; float4 depthInfo;
  float4 sky; float4 ambient;
  float4 lightPosition[32]; float4 lightColor[32];
  float4 cloudCount; float4 cloudPosition[8]; float4 cloudColor[8];
};
Texture2D image : register(t0);
Texture2D lightmap : register(t1);
SamplerState pointSampler : register(s0);
SamplerState groundSampler : register(s1);
SamplerState linearSampler : register(s2);
struct VertexIn {
  float3 world : POSITION; float2 uv : TEXCOORD0; float4 tint : COLOR0;
  float4 sprite : TEXCOORD1;
};
struct WorldOut {
  float4 position : SV_POSITION; float2 uv : TEXCOORD0;
  float dz : TEXCOORD1; float4 tint : COLOR0; float4 sprite : TEXCOORD2;
};
WorldOut worldVS(VertexIn input) {
  WorldOut o;
  float dz = projection.x - input.world.y;
  float xc = 2.0 / viewport.x * (input.world.x - viewport.z) * projection.y;
  float yc = dz - 2.0 / viewport.y *
      (projection.w * dz + projection.z - input.world.z * projection.y);
  // D3D's clip depth is[0,w]. Use affine view-depth z for correct clipping
  // and triangle interpolation; the reference's quadratic z changes neither
  // screen x/y nor UV, but clips triangles at the wrong near-plane location.
  float zc = depthInfo.z * (dz - depthInfo.y) / (depthInfo.z - depthInfo.y);
  o.position = float4(xc, yc, zc, dz);
  o.uv = input.uv; o.dz = dz; o.tint = input.tint; o.sprite = input.sprite;
  return o;
}
float coc(float dz) {
  return saturate((abs(dz-depthInfo.x)/depthInfo.x*1.6-.12)/.40) * depthInfo.w;
}
float haze(float dz) { return saturate((dz/depthInfo.x-1.12)/1.02)*.96; }
float4 pointPremult(float2 uv) {
  // Transparent outside the sprite, including the blur footprint.
  if (any(uv < 0) || any(uv > 1)) return 0;
  float4 c = image.Sample(pointSampler,uv); c.rgb *= c.a; return c;
}
float4 texelPremult(int2 at, uint2 size) {
  if(any(at<0)||any(at>=int2(size))) return 0;
  float4 c=image.Load(int3(at,0));c.rgb*=c.a;return c;
}
float4 blurPremult(float2 uv) {
  // Bilinear interpolation of premultiplied texels, only for the defocused
  // taps. Straight-alpha interpolation would darken transparent borders.
  uint w,h;image.GetDimensions(w,h);uint2 size=uint2(w,h);
  float2 p=uv*size-.5;int2 at=int2(floor(p));float2 f=frac(p);
  return lerp(lerp(texelPremult(at,size),texelPremult(at+int2(1,0),size),f.x),
              lerp(texelPremult(at+int2(0,1),size),texelPremult(at+int2(1,1),size),f.x),f.y);
}
float4 worldPS(WorldOut i) : SV_TARGET {
  float4 c;
  if (i.sprite.w < .5) {
    float4 sharp = image.Sample(groundSampler,i.uv);
    float4 soft = image.SampleBias(groundSampler,i.uv,3.5);
    c = lerp(sharp,soft,coc(i.dz)); c.rgb *= c.a;
  } else if (i.sprite.w < 1.5) {
    c = pointPremult(i.uv);
  } else {
    c = pointPremult(i.uv);
    // Continuous blur radius, with a point-sampled sharp center. No discrete
    // preblurred sprite variants or texture filtering on in-focus pixels.
    float radius = coc(i.dz)*3.6;
    if (radius > .001) {
      float2 delta = radius * i.dz / projection.y / max(i.sprite.xy,float2(.001,.001));
      float4 blur = blurPremult(i.uv)*4;
      blur += blurPremult(i.uv+float2(delta.x,0))*2;
      blur += blurPremult(i.uv-float2(delta.x,0))*2;
      blur += blurPremult(i.uv+float2(0,delta.y))*2;
      blur += blurPremult(i.uv-float2(0,delta.y))*2;
      blur += blurPremult(i.uv+delta)+blurPremult(i.uv-delta);
      blur += blurPremult(i.uv+float2(delta.x,-delta.y));
      blur += blurPremult(i.uv+float2(-delta.x,delta.y));
      c = lerp(c,blur/16,saturate(radius));
    }
  }
  clip(c.a-.001);
  c.rgb = lerp(c.rgb,float3(1,.941176,.760784)*c.a,saturate(i.sprite.z));
  c.rgb *= i.tint.rgb;
  c.rgb = lerp(c.rgb,sky.rgb*c.a,haze(i.dz));
  return c*i.tint.a;
}
struct FullOut { float4 position : SV_POSITION; float2 uv : TEXCOORD0; };
FullOut fullVS(uint id : SV_VertexID) {
  FullOut o; o.uv=float2((id<<1)&2,id&2);
  o.position=float4(o.uv.x*2-1,1-o.uv.y*2,0,1); return o;
}
float4 skyPS(FullOut i) : SV_TARGET {
  return float4(sky.rgb*lerp(.68,1.0,saturate(i.uv.y*2)),1);
}
float4 lightPS(FullOut i) : SV_TARGET {
  float3 lighting=ambient.rgb;
  float2 pixel=i.uv*viewport.xy;
  // Fable game_template.html cloud shadows: a screen-space radial multiply,
  // flat tint through0.45R then a linear fade to white atR. Projected lights
  // are added afterwards, so a cloud never dims a local flame contribution.
  [loop] for (int n=0;n<(int)cloudCount.x;n++) {
    float distance=length(pixel-cloudPosition[n].xy)/cloudPosition[n].z;
    float fade=saturate((distance-.45)/.55);
    float3 grade=lerp(cloudColor[n].rgb,float3(1,1,1),fade);
    lighting*=lerp(float3(1,1,1),grade,cloudPosition[n].w);
  }
  [loop] for (int n=0;n<(int)ambient.w;n++) {
    float distance=length(pixel-lightPosition[n].xy)/max(1,lightPosition[n].z);
    float weight=saturate(1-distance);
    lighting += lightColor[n].rgb*lightPosition[n].w*weight*weight;
  }
  return float4(lighting,1);
}
float4 postPS(FullOut i) : SV_TARGET {
  float3 color=image.Sample(pointSampler,i.uv).rgb;
  color*=lightmap.Sample(linearSampler,i.uv).rgb;
  float2 p=(i.uv-.5)*float2(viewport.x/viewport.y,1);
  float vignette=smoothstep(.35,1.15,length(p));
  return float4(saturate(color*(1-vignette*sky.w)),1);
}
)HLSL";
struct Texture {
  Com<ID3D11Texture2D> image;
  Com<ID3D11ShaderResourceView> view;
  int width = 0, height = 0;
  bool terrain = false;
  std::size_t bytes = 0;
  std::uint64_t revision = 0, used = 0;
};
struct Target {
  Com<ID3D11Texture2D> image;
  Com<ID3D11RenderTargetView> target;
  Com<ID3D11ShaderResourceView> view;
};
struct Draw { UINT index_count = 0, start_index = 0; std::uint64_t texture = 0; int blend = 0; bool write_depth = true; };
}  // namespace detail

class Renderer {
 public:
  Renderer() = default;
  Renderer(const Renderer&) = delete;
  Renderer& operator=(const Renderer&) = delete;
  Renderer(Renderer&&) = default;
  Renderer& operator=(Renderer&&) = default;
  // Device recovery is explicit: discard device-bound caches and reupload the
  // caller's retained decoded textures before rendering the next scene.
  void reset() { if(context_) context_->ClearState(); *this=Renderer{}; }
  const std::string& error() const { return error_; }
  const std::string& adapter_name() const { return adapter_; }
  const Stats& stats() const { return stats_; }
  // This buffer belongs to the last completed render, with width*4 row pitch.
  std::span<const std::uint8_t> pixels_bgra() const { return readback_; }
  int width() const { return width_; }
  int height() const { return height_; }

  bool initialize() {
    if (ready_) return true;
    if (device_lost_) return fail("reset renderer and reupload textures after device loss",DXGI_ERROR_DEVICE_REMOVED);
    error_.clear();
    D3D_FEATURE_LEVEL actual{};
    const D3D_FEATURE_LEVEL levels[]{D3D_FEATURE_LEVEL_11_0};
    HRESULT hr = D3D11CreateDevice(nullptr,D3D_DRIVER_TYPE_HARDWARE,nullptr,
        D3D11_CREATE_DEVICE_BGRA_SUPPORT,levels,1,D3D11_SDK_VERSION,
        device_.ReleaseAndGetAddressOf(),&actual,context_.ReleaseAndGetAddressOf());
    if (FAILED(hr)) return fail("D3D11 hardware device",hr);
    detail::Com<IDXGIDevice> dxgi;
    detail::Com<IDXGIAdapter> adapter;
    DXGI_ADAPTER_DESC description{};
    if (FAILED(device_.As(&dxgi)) || FAILED(dxgi->GetAdapter(&adapter)) ||
        FAILED(adapter->GetDesc(&description))) return fail("D3D11 adapter description",E_FAIL);
    const int count=WideCharToMultiByte(CP_UTF8,0,description.Description,-1,nullptr,0,nullptr,nullptr);
    std::vector<char> name(static_cast<std::size_t>(std::max(1,count)),0);
    WideCharToMultiByte(CP_UTF8,0,description.Description,-1,name.data(),count,nullptr,nullptr);
    adapter_=name.data(); stats_.hardware=true; stats_.feature_level=actual;
    stats_.vendor_id=description.VendorId; stats_.device_id=description.DeviceId;
    detail::Com<ID3DBlob> code;
    if (!compile("worldVS","vs_5_0",code)) return false;
    if (FAILED(hr=device_->CreateVertexShader(code->GetBufferPointer(),code->GetBufferSize(),nullptr,
                           world_vs_.ReleaseAndGetAddressOf()))) return fail("world vertex shader",hr);
    const D3D11_INPUT_ELEMENT_DESC input[]{
      {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
      {"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0},
      {"COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,20,D3D11_INPUT_PER_VERTEX_DATA,0},
      {"TEXCOORD",1,DXGI_FORMAT_R32G32B32A32_FLOAT,0,36,D3D11_INPUT_PER_VERTEX_DATA,0}};
    if (FAILED(hr=device_->CreateInputLayout(input,4,code->GetBufferPointer(),code->GetBufferSize(),
                              input_.ReleaseAndGetAddressOf()))) return fail("world vertex layout",hr);
    if (!compile("fullVS","vs_5_0",code)) return false;
    if (FAILED(hr=device_->CreateVertexShader(code->GetBufferPointer(),code->GetBufferSize(),nullptr,
                           full_vs_.ReleaseAndGetAddressOf()))) return fail("post vertex shader",hr);
    const auto pixel=[&](const char* entry,detail::Com<ID3D11PixelShader>& shader) {
      if (!compile(entry,"ps_5_0",code)) return false;
      const HRESULT result=device_->CreatePixelShader(code->GetBufferPointer(),code->GetBufferSize(),nullptr,
                                                       shader.ReleaseAndGetAddressOf());
      return SUCCEEDED(result) || fail(entry,result);
    };
    if (!pixel("worldPS",world_ps_) || !pixel("skyPS",sky_ps_) ||
        !pixel("lightPS",light_ps_) || !pixel("postPS",post_ps_)) return false;
    D3D11_BUFFER_DESC cb{}; cb.ByteWidth=sizeof(detail::Constants); cb.Usage=D3D11_USAGE_DYNAMIC;
    cb.BindFlags=D3D11_BIND_CONSTANT_BUFFER; cb.CPUAccessFlags=D3D11_CPU_ACCESS_WRITE;
    if (FAILED(hr=device_->CreateBuffer(&cb,nullptr,constants_.ReleaseAndGetAddressOf()))) return fail("frame constants",hr);
    D3D11_SAMPLER_DESC sampler{};
    sampler.AddressU=sampler.AddressV=sampler.AddressW=D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler.MaxLOD=D3D11_FLOAT32_MAX; sampler.MinLOD=0; sampler.MaxAnisotropy=1;
    sampler.Filter=D3D11_FILTER_MIN_MAG_MIP_POINT;
    if (FAILED(hr=device_->CreateSamplerState(&sampler,point_.ReleaseAndGetAddressOf()))) return fail("point sampler",hr);
    sampler.Filter=D3D11_FILTER_ANISOTROPIC; sampler.MaxAnisotropy=8;
    if (FAILED(hr=device_->CreateSamplerState(&sampler,ground_.ReleaseAndGetAddressOf()))) return fail("anisotropic sampler",hr);
    sampler.Filter=D3D11_FILTER_MIN_MAG_MIP_LINEAR; sampler.MaxAnisotropy=1;
    if (FAILED(hr=device_->CreateSamplerState(&sampler,linear_.ReleaseAndGetAddressOf()))) return fail("linear sampler",hr);
    D3D11_RASTERIZER_DESC raster{}; raster.FillMode=D3D11_FILL_SOLID;
    raster.CullMode=D3D11_CULL_NONE; raster.DepthClipEnable=TRUE;
    if (FAILED(hr=device_->CreateRasterizerState(&raster,raster_.ReleaseAndGetAddressOf()))) return fail("raster state",hr);
    D3D11_DEPTH_STENCIL_DESC depth{}; depth.DepthEnable=TRUE;
    depth.DepthFunc=D3D11_COMPARISON_LESS_EQUAL; depth.DepthWriteMask=D3D11_DEPTH_WRITE_MASK_ALL;
    if (FAILED(hr=device_->CreateDepthStencilState(&depth,depth_write_.ReleaseAndGetAddressOf()))) return fail("depth state",hr);
    depth.DepthWriteMask=D3D11_DEPTH_WRITE_MASK_ZERO;
    if (FAILED(hr=device_->CreateDepthStencilState(&depth,depth_read_.ReleaseAndGetAddressOf()))) return fail("transparent depth state",hr);
    depth.DepthEnable=FALSE;
    if (FAILED(hr=device_->CreateDepthStencilState(&depth,depth_off_.ReleaseAndGetAddressOf()))) return fail("post depth state",hr);
    D3D11_BLEND_DESC blend{};
    auto& target=blend.RenderTarget[0]; target.RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
    if (FAILED(hr=device_->CreateBlendState(&blend,opaque_.ReleaseAndGetAddressOf()))) return fail("opaque blend",hr);
    target.BlendEnable=TRUE; target.SrcBlend=D3D11_BLEND_ONE; target.DestBlend=D3D11_BLEND_INV_SRC_ALPHA;
    target.BlendOp=D3D11_BLEND_OP_ADD; target.SrcBlendAlpha=D3D11_BLEND_ONE;
    target.DestBlendAlpha=D3D11_BLEND_INV_SRC_ALPHA; target.BlendOpAlpha=D3D11_BLEND_OP_ADD;
    if (FAILED(hr=device_->CreateBlendState(&blend,alpha_.ReleaseAndGetAddressOf()))) return fail("alpha blend",hr);
    target.DestBlend=D3D11_BLEND_ONE;
    if (FAILED(hr=device_->CreateBlendState(&blend,additive_.ReleaseAndGetAddressOf()))) return fail("additive blend",hr);
    ready_=true; return true;
  }

  bool has_texture(std::uint64_t id, std::uint64_t revision) const {
    const auto found=textures_.find(id);
    return found!=textures_.end() && found->second.revision==revision;
  }
  bool upload_texture(std::uint64_t id, int width, int height, const std::uint8_t* rgba,
                      std::size_t stride, bool terrain, std::uint64_t revision) {
    if (!id || !rgba || width<=0 || height<=0 || width>8192 || height>8192 ||
        stride<static_cast<std::size_t>(width)*4 || stride>UINT_MAX)
      return fail("invalid RGBA texture packet",E_INVALIDARG);
    if (!initialize()) return false;
    auto found=textures_.find(id);
    if (found!=textures_.end() && found->second.revision==revision &&
        found->second.width==width && found->second.height==height && found->second.terrain==terrain) {
      found->second.used=++clock_; ++stats_.texture_hits; return true;
    }
    std::size_t bytes=0;
    for(int w=width,h=height;;w=std::max(1,w/2),h=std::max(1,h/2)) {
      bytes+=static_cast<std::size_t>(w)*h*4;
      if(!terrain || (w==1 && h==1)) break;
    }
    if(bytes>detail::kTextureBytes) return fail("texture exceeds GPU cache budget",E_OUTOFMEMORY);
    if(found!=textures_.end()) { stats_.texture_bytes-=found->second.bytes; textures_.erase(found); }
    while(!textures_.empty() && (textures_.size()>=detail::kTextures ||
          stats_.texture_bytes+bytes>detail::kTextureBytes)) {
      const auto victim=std::min_element(textures_.begin(),textures_.end(),
        [](const auto& a,const auto& b){return a.second.used<b.second.used;});
      stats_.texture_bytes-=victim->second.bytes; textures_.erase(victim); ++stats_.texture_evictions;
    }
    detail::Texture texture;
    D3D11_TEXTURE2D_DESC desc{}; desc.Width=width; desc.Height=height; desc.MipLevels=terrain?0:1;
    desc.ArraySize=1; desc.Format=DXGI_FORMAT_R8G8B8A8_UNORM; desc.SampleDesc.Count=1;
    desc.Usage=D3D11_USAGE_DEFAULT; desc.BindFlags=D3D11_BIND_SHADER_RESOURCE;
    if(terrain) { desc.BindFlags|=D3D11_BIND_RENDER_TARGET; desc.MiscFlags=D3D11_RESOURCE_MISC_GENERATE_MIPS; }
    HRESULT hr=device_->CreateTexture2D(&desc,nullptr,&texture.image);
    if(FAILED(hr)) return fail("RGBA texture allocation",hr);
    context_->UpdateSubresource(texture.image.Get(),0,nullptr,rgba,static_cast<UINT>(stride),0);
    if(FAILED(hr=device_->CreateShaderResourceView(texture.image.Get(),nullptr,&texture.view)))
      return fail("RGBA texture view",hr);
    if(terrain) context_->GenerateMips(texture.view.Get());
    texture.width=width;texture.height=height;texture.terrain=terrain;texture.bytes=bytes;
    texture.revision=revision;texture.used=++clock_; textures_.emplace(id,std::move(texture));
    stats_.texture_bytes+=bytes; stats_.texture_count=textures_.size(); ++stats_.texture_uploads;
    return true;
  }

  // nullptr HDC is supported for deterministic offscreen verification. A live
  // HDC gets this exact completed frame synchronously, before the caller's HUD.
  bool render(const Scene& scene, HDC dc) {
    const auto start=std::chrono::steady_clock::now(); error_.clear();
    if(!valid(scene)) return false;
    if(!initialize() || !resize(scene.camera.width,scene.camera.height)) return false;
    if(!assemble(scene)) return false;
    if(!upload_geometry() || !upload_constants(scene)) return false;
    stats_.draw_calls=stats_.terrain_triangles=stats_.sprites=0;
    ID3D11ShaderResourceView* null_views[2]{};
    context_->PSSetShaderResources(0,2,null_views);
    const float black[4]{0,0,0,1};
    context_->ClearRenderTargetView(world_.target.Get(),black);
    context_->ClearDepthStencilView(depth_view_.Get(),D3D11_CLEAR_DEPTH,1,0);
    context_->RSSetState(raster_.Get());
    ID3D11Buffer* cb=constants_.Get();
    context_->VSSetConstantBuffers(0,1,&cb);context_->PSSetConstantBuffers(0,1,&cb);
    ID3D11SamplerState* samplers[]{point_.Get(),ground_.Get(),linear_.Get()};
    context_->PSSetSamplers(0,3,samplers);
    viewport(width_,height_);
    ID3D11RenderTargetView* world=world_.target.Get();
    context_->OMSetRenderTargets(1,&world,depth_view_.Get());
    fullscreen(sky_ps_.Get());
    const UINT stride=sizeof(detail::GpuVertex),offset=0;
    ID3D11Buffer* vb=vertex_buffer_.Get();
    context_->IASetVertexBuffers(0,1,&vb,&stride,&offset);
    context_->IASetIndexBuffer(index_buffer_.Get(),DXGI_FORMAT_R32_UINT,0);
    context_->IASetInputLayout(input_.Get());
    context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context_->VSSetShader(world_vs_.Get(),nullptr,0);context_->PSSetShader(world_ps_.Get(),nullptr,0);
    for(const auto& draw:draws_) {
      auto found=textures_.find(draw.texture);
      if(found==textures_.end()) return fail("scene references evicted texture",E_FAIL);
      found->second.used=++clock_;
      ID3D11ShaderResourceView* view=found->second.view.Get();
      context_->PSSetShaderResources(0,1,&view);
      context_->OMSetDepthStencilState(draw.write_depth?depth_write_.Get():depth_read_.Get(),0);
      context_->OMSetBlendState(draw.blend==2?additive_.Get():draw.blend==1?alpha_.Get():opaque_.Get(),nullptr,UINT_MAX);
      context_->DrawIndexed(draw.index_count,draw.start_index,0);++stats_.draw_calls;
      if(draw.write_depth) stats_.terrain_triangles+=draw.index_count/3;
      else if(draw.blend) ++stats_.sprites;
    }
    context_->PSSetShaderResources(0,2,null_views);
    ID3D11RenderTargetView* lighting=light_.target.Get();
    context_->OMSetRenderTargets(1,&lighting,nullptr);viewport(light_width_,light_height_);fullscreen(light_ps_.Get());
    ID3D11RenderTargetView* output=output_.target.Get();
    context_->OMSetRenderTargets(1,&output,nullptr);viewport(width_,height_);
    ID3D11ShaderResourceView* post_views[]{world_.view.Get(),light_.view.Get()};
    context_->PSSetShaderResources(0,2,post_views);fullscreen(post_ps_.Get());
    context_->PSSetShaderResources(0,2,null_views);
    context_->OMSetRenderTargets(0,nullptr,nullptr);
    context_->CopyResource(staging_.Get(),output_.image.Get());
    D3D11_MAPPED_SUBRESOURCE mapped{};
    HRESULT hr=context_->Map(staging_.Get(),0,D3D11_MAP_READ,0,&mapped);
    if(FAILED(hr)) return fail("world GPU readback",hr);
    for(int y=0;y<height_;++y)
      std::memcpy(readback_.data()+static_cast<std::size_t>(y)*width_*4,
                  static_cast<const BYTE*>(mapped.pData)+static_cast<std::size_t>(y)*mapped.RowPitch,
                  static_cast<std::size_t>(width_)*4);
    context_->Unmap(staging_.Get(),0);
    if(dc) {
      BITMAPINFO info{};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=width_;
      info.bmiHeader.biHeight=-height_;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;
      info.bmiHeader.biCompression=BI_RGB;
      if(StretchDIBits(dc,0,0,width_,height_,0,0,width_,height_,readback_.data(),&info,DIB_RGB_COLORS,SRCCOPY)==GDI_ERROR)
        return fail("world HDC composite",HRESULT_FROM_WIN32(GetLastError()));
    }
    hr=device_->GetDeviceRemovedReason();
    if(FAILED(hr)) { ready_=false;device_lost_=true; return fail("D3D11 device removed; reset renderer and reupload textures",hr); }
    ++stats_.frames;
    stats_.render_readback_ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();
    return true;
  }

 private:
  bool fail(const char* message,HRESULT hr) {
    char code[24]{}; std::snprintf(code,sizeof(code)," (0x%08lx)",static_cast<unsigned long>(hr));
    error_=std::string(message)+code; return false;
  }
  bool compile(const char* entry,const char* profile,detail::Com<ID3DBlob>& code) {
    detail::Com<ID3DBlob> diagnostic;
    const HRESULT hr=D3DCompile(detail::kShader,std::strlen(detail::kShader),"fable_gpu.hpp",nullptr,nullptr,
        entry,profile,D3DCOMPILE_OPTIMIZATION_LEVEL3|D3DCOMPILE_ENABLE_STRICTNESS,0,
        code.ReleaseAndGetAddressOf(),&diagnostic);
    if(SUCCEEDED(hr)) return true;
    fail(entry,hr);if(diagnostic) error_+="\n"+std::string(static_cast<const char*>(diagnostic->GetBufferPointer()),diagnostic->GetBufferSize());
    return false;
  }
  bool valid(const Scene& s) {
    const auto& c=s.camera;
    if(c.width<10 || c.height<10 || c.width>8192 || c.height>8192 ||
        static_cast<std::uint64_t>(c.width)*c.height>16777216u ||
        !std::isfinite(c.cam_x)||!std::isfinite(c.cam_y)||!std::isfinite(c.d0)||
        !std::isfinite(c.k)||c.k<=0||!std::isfinite(c.a)||c.a<=0||!std::isfinite(c.horizon)||
        !std::isfinite(c.dzp)||c.dzp<=0||!std::isfinite(c.near_depth)||c.near_depth<=0||
        !std::isfinite(c.far_depth)||c.far_depth<=c.near_depth)
      return fail("invalid Fable camera/viewport",E_INVALIDARG);
    if(s.sprites.size()>detail::kSprites||s.world_meshes.size()>detail::kMeshes||
        s.ground_meshes.size()>detail::kMeshes-s.world_meshes.size()||s.lights.size()>detail::kLights||
        s.clouds.size()>detail::kClouds)
      return fail("scene packet exceeds bounded draw/light/cloud counts",E_INVALIDARG);
    for(float f:{s.ambient_r,s.ambient_g,s.ambient_b,s.sky_r,s.sky_g,s.sky_b,s.dof_strength,s.vignette})
      if(!std::isfinite(f)||f<0||f>4) return fail("invalid scene grading",E_INVALIDARG);
    return true;
  }
  bool target(detail::Target& result,int w,int h,DXGI_FORMAT format) {
    D3D11_TEXTURE2D_DESC desc{};desc.Width=w;desc.Height=h;desc.MipLevels=1;desc.ArraySize=1;
    desc.Format=format;desc.SampleDesc.Count=1;desc.BindFlags=D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE;
    HRESULT hr=device_->CreateTexture2D(&desc,nullptr,result.image.ReleaseAndGetAddressOf());
    if(FAILED(hr)) return fail("world target allocation",hr);
    if(FAILED(hr=device_->CreateRenderTargetView(result.image.Get(),nullptr,result.target.ReleaseAndGetAddressOf()))||
       FAILED(hr=device_->CreateShaderResourceView(result.image.Get(),nullptr,result.view.ReleaseAndGetAddressOf())))
      return fail("world target views",hr);
    return true;
  }
  bool resize(int w,int h) {
    if(width_==w&&height_==h&&staging_) return true;
    ID3D11ShaderResourceView* empty[2]{};context_->PSSetShaderResources(0,2,empty);context_->OMSetRenderTargets(0,nullptr,nullptr);
    detail::Target world,output,light;
    const int lw=std::max(2,(w+3)/4),lh=std::max(2,(h+3)/4);
    if(!target(world,w,h,DXGI_FORMAT_B8G8R8A8_UNORM)||!target(output,w,h,DXGI_FORMAT_B8G8R8A8_UNORM)||
       !target(light,lw,lh,DXGI_FORMAT_R16G16B16A16_FLOAT)) return false;
    D3D11_TEXTURE2D_DESC desc{};desc.Width=w;desc.Height=h;desc.MipLevels=1;desc.ArraySize=1;
    desc.Format=DXGI_FORMAT_D32_FLOAT;desc.SampleDesc.Count=1;desc.BindFlags=D3D11_BIND_DEPTH_STENCIL;
    detail::Com<ID3D11Texture2D> depth,staging;detail::Com<ID3D11DepthStencilView> view;
    HRESULT hr=device_->CreateTexture2D(&desc,nullptr,&depth);
    if(FAILED(hr)||FAILED(hr=device_->CreateDepthStencilView(depth.Get(),nullptr,&view))) return fail("world depth allocation",hr);
    desc.Format=DXGI_FORMAT_B8G8R8A8_UNORM;desc.BindFlags=0;desc.Usage=D3D11_USAGE_STAGING;desc.CPUAccessFlags=D3D11_CPU_ACCESS_READ;
    if(FAILED(hr=device_->CreateTexture2D(&desc,nullptr,&staging))) return fail("world readback allocation",hr);
    readback_.resize(static_cast<std::size_t>(w)*h*4);
    world_=std::move(world);output_=std::move(output);light_=std::move(light);
    depth_image_=std::move(depth);depth_view_=std::move(view);staging_=std::move(staging);
    width_=w;height_=h;light_width_=lw;light_height_=lh;return true;
  }
  bool mesh(const Mesh& mesh) {
    if(mesh.vertices.empty()&&mesh.indices.empty()) return true;
    if(mesh.vertices.size()>detail::kVertices-vertices_.size()||mesh.indices.size()>detail::kIndices-indices_.size()||
        mesh.indices.size()%3||mesh.vertices.empty()||!std::isfinite(mesh.opacity)||mesh.opacity<0||mesh.opacity>1||
        !textures_.contains(mesh.texture)) return fail("invalid world mesh or missing texture",E_INVALIDARG);
    if(!mesh.nearest&&!textures_.at(mesh.texture).terrain) return fail("terrain texture lacks mip chain",E_INVALIDARG);
    const UINT base=static_cast<UINT>(vertices_.size()),start=static_cast<UINT>(indices_.size());
    for(const auto& v:mesh.vertices) {
      for(float f:{v.x,v.y,v.elevation,v.u,v.v}) if(!std::isfinite(f)) return fail("nonfinite mesh vertex",E_INVALIDARG);
      vertices_.push_back({v.x,v.y,v.elevation,v.u,v.v,1,1,1,mesh.opacity,0,0,0,mesh.nearest?1.f:0.f});
    }
    for(auto index:mesh.indices) { if(index>=mesh.vertices.size()) return fail("mesh index out of range",E_INVALIDARG);indices_.push_back(base+index); }
    draws_.push_back({static_cast<UINT>(mesh.indices.size()),start,mesh.texture,mesh.opacity<1?1:0,mesh.opacity>=1});
    return true;
  }
  bool assemble(const Scene& scene) {
    vertices_.clear();indices_.clear();draws_.clear();sprite_order_.clear();
    if(!mesh(scene.terrain)) return false;
    for(const auto& m:scene.ground_meshes) if(!mesh(m)) return false;
    for(const auto& m:scene.world_meshes) if(m.opacity>=1&&!mesh(m)) return false;
    for(const auto& sprite:scene.sprites) {
      for(float f:{sprite.x,sprite.y,sprite.elevation,sprite.width,sprite.height,sprite.anchor_x,sprite.anchor_y,
                   sprite.opacity,sprite.tint_r,sprite.tint_g,sprite.tint_b,sprite.flash,sprite.rotation})
        if(!std::isfinite(f)) return fail("nonfinite billboard",E_INVALIDARG);
      if(sprite.width<=0||sprite.height<=0||sprite.opacity<0||sprite.opacity>1||!textures_.contains(sprite.texture))
        return fail("invalid billboard or missing texture",E_INVALIDARG);
      const float dz=scene.camera.d0-sprite.y;
      if(dz<scene.camera.near_depth||dz>scene.camera.far_depth||sprite.opacity==0) continue;
      sprite_order_.push_back(&sprite);
    }
    std::stable_sort(sprite_order_.begin(),sprite_order_.end(),[](const Sprite* a,const Sprite* b){
      if(a->ground_layer!=b->ground_layer) return a->ground_layer;
      return a->y<b->y;
    });
    for(const Sprite* pointer:sprite_order_) {
      if(vertices_.size()+4>detail::kVertices||indices_.size()+6>detail::kIndices) return fail("billboard geometry budget exceeded",E_INVALIDARG);
      const auto& s=*pointer;const UINT base=static_cast<UINT>(vertices_.size()),start=static_cast<UINT>(indices_.size());
      const float cosine=std::cos(s.rotation),sine=std::sin(s.rotation);
      const float dz=scene.camera.d0-s.y;
      const float coc=std::clamp((std::abs(dz-scene.camera.dzp)/scene.camera.dzp*1.6f-.12f)/.40f,0.f,1.f)*
                      std::clamp(scene.dof_strength,0.f,1.f);
      const float pad=coc>0 ? (coc*3.6f+1)*dz/scene.camera.k : 0;
      for(const auto& uv:std::array<std::array<float,2>,4>{{{0,0},{1,0},{0,1},{1,1}}}) {
        const float u=uv[0]+(uv[0]*2-1)*pad/s.width,v=uv[1]+(uv[1]*2-1)*pad/s.height;
        const float x=(u-s.anchor_x)*s.width,y=(v-s.anchor_y)*s.height;
        vertices_.push_back({s.x+cosine*x-sine*y,s.y,s.elevation-sine*x-cosine*y,
            s.flip?1-u:u,v,s.tint_r,s.tint_g,s.tint_b,s.opacity,s.width,s.height,s.flash,2});
      }
      for(UINT i:{0u,2u,1u,1u,2u,3u}) indices_.push_back(base+i);
      draws_.push_back({6,start,s.texture,s.additive?2:1,false});
    }
    for(const auto& m:scene.world_meshes) if(m.opacity<1&&!mesh(m)) return false;
    return true;
  }
  bool dynamic_buffer(detail::Com<ID3D11Buffer>& buffer,std::size_t& capacity,std::size_t required,UINT bind) {
    if(!required) required=16;
    if(buffer&&capacity>=required) return true;
    const std::size_t next=(required+65535u)&~std::size_t(65535u);
    D3D11_BUFFER_DESC desc{};desc.ByteWidth=static_cast<UINT>(next);desc.Usage=D3D11_USAGE_DYNAMIC;
    desc.BindFlags=bind;desc.CPUAccessFlags=D3D11_CPU_ACCESS_WRITE;
    const HRESULT hr=device_->CreateBuffer(&desc,nullptr,buffer.ReleaseAndGetAddressOf());
    if(FAILED(hr)) return fail("dynamic geometry allocation",hr);
    capacity=next;return true;
  }
  bool upload_geometry() {
    if(!dynamic_buffer(vertex_buffer_,vertex_capacity_,vertices_.size()*sizeof(detail::GpuVertex),D3D11_BIND_VERTEX_BUFFER)||
       !dynamic_buffer(index_buffer_,index_capacity_,indices_.size()*sizeof(UINT),D3D11_BIND_INDEX_BUFFER)) return false;
    const auto upload=[&](ID3D11Buffer* buffer,const void* data,std::size_t size) {
      if(!size) return true;D3D11_MAPPED_SUBRESOURCE mapped{};
      HRESULT hr=context_->Map(buffer,0,D3D11_MAP_WRITE_DISCARD,0,&mapped);
      if(FAILED(hr)) return fail("dynamic geometry map",hr);
      std::memcpy(mapped.pData,data,size);context_->Unmap(buffer,0);return true;
    };
    return upload(vertex_buffer_.Get(),vertices_.data(),vertices_.size()*sizeof(detail::GpuVertex))&&
           upload(index_buffer_.Get(),indices_.data(),indices_.size()*sizeof(UINT));
  }
  bool upload_constants(const Scene& scene) {
    detail::Constants c{};const auto& p=scene.camera;
    c.viewport[0]=static_cast<float>(p.width);c.viewport[1]=static_cast<float>(p.height);c.viewport[2]=p.cam_x;c.viewport[3]=p.cam_y;
    c.projection[0]=p.d0;c.projection[1]=p.k;c.projection[2]=p.a;c.projection[3]=p.horizon;
    c.depth[0]=p.dzp;c.depth[1]=p.near_depth;c.depth[2]=p.far_depth;c.depth[3]=std::clamp(scene.dof_strength,0.f,1.f);
    c.sky[0]=scene.sky_r;c.sky[1]=scene.sky_g;c.sky[2]=scene.sky_b;c.sky[3]=std::clamp(scene.vignette,0.f,1.f);
    c.ambient[0]=scene.ambient_r;c.ambient[1]=scene.ambient_g;c.ambient[2]=scene.ambient_b;
    int count=0;
    for(const auto& light:scene.lights) {
      for(float f:{light.x,light.y,light.elevation,light.radius,light.r,light.g,light.b,light.intensity})
        if(!std::isfinite(f)) return fail("nonfinite world light",E_INVALIDARG);
      if(light.radius<=0||light.intensity<0) return fail("invalid world light radius/intensity",E_INVALIDARG);
      const float dz=p.d0-light.y;if(dz<p.near_depth||dz>p.far_depth) continue;
      const float scale=p.k/dz;
      c.light_position[count][0]=p.width*.5f+(light.x-p.cam_x)*scale;
      c.light_position[count][1]=p.horizon+p.a/dz-light.elevation*scale;
      c.light_position[count][2]=std::max(6.f,light.radius*scale);
      c.light_position[count][3]=light.intensity;
      c.light_color[count][0]=light.r;c.light_color[count][1]=light.g;c.light_color[count][2]=light.b;++count;
    }
    c.ambient[3]=static_cast<float>(count);
    count=0;
    for(const auto& cloud:scene.clouds) {
      for(float f:{cloud.screen_x,cloud.screen_y,cloud.radius,cloud.r,cloud.g,cloud.b,cloud.strength})
        if(!std::isfinite(f)) return fail("nonfinite screen-space cloud",E_INVALIDARG);
      if(cloud.radius<=0 || cloud.radius>1.e7f || cloud.r<0 || cloud.r>1 || cloud.g<0 || cloud.g>1 ||
          cloud.b<0 || cloud.b>1 || cloud.strength<0 || cloud.strength>1)
        return fail("invalid screen-space cloud radius/tint/strength",E_INVALIDARG);
      c.cloud_position[count][0]=cloud.screen_x;c.cloud_position[count][1]=cloud.screen_y;
      c.cloud_position[count][2]=cloud.radius;c.cloud_position[count][3]=cloud.strength;
      c.cloud_color[count][0]=cloud.r;c.cloud_color[count][1]=cloud.g;c.cloud_color[count][2]=cloud.b;++count;
    }
    c.cloud_count[0]=static_cast<float>(count);
    D3D11_MAPPED_SUBRESOURCE mapped{};const HRESULT hr=context_->Map(constants_.Get(),0,D3D11_MAP_WRITE_DISCARD,0,&mapped);
    if(FAILED(hr)) return fail("frame constants map",hr);
    std::memcpy(mapped.pData,&c,sizeof(c));context_->Unmap(constants_.Get(),0);return true;
  }
  void viewport(int w,int h) { const D3D11_VIEWPORT vp{0,0,static_cast<float>(w),static_cast<float>(h),0,1};context_->RSSetViewports(1,&vp); }
  void fullscreen(ID3D11PixelShader* shader) {
    context_->IASetInputLayout(nullptr);context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context_->VSSetShader(full_vs_.Get(),nullptr,0);context_->PSSetShader(shader,nullptr,0);
    context_->OMSetDepthStencilState(depth_off_.Get(),0);context_->OMSetBlendState(opaque_.Get(),nullptr,UINT_MAX);
    context_->Draw(3,0);++stats_.draw_calls;
  }
  bool ready_=false,device_lost_=false;int width_=0,height_=0,light_width_=0,light_height_=0;
  std::string error_,adapter_;Stats stats_{};std::uint64_t clock_=0;
  detail::Com<ID3D11Device> device_;detail::Com<ID3D11DeviceContext> context_;
  detail::Com<ID3D11VertexShader> world_vs_,full_vs_;
  detail::Com<ID3D11PixelShader> world_ps_,sky_ps_,light_ps_,post_ps_;
  detail::Com<ID3D11InputLayout> input_;detail::Com<ID3D11Buffer> constants_,vertex_buffer_,index_buffer_;
  detail::Com<ID3D11SamplerState> point_,ground_,linear_;detail::Com<ID3D11RasterizerState> raster_;
  detail::Com<ID3D11DepthStencilState> depth_write_,depth_read_,depth_off_;
  detail::Com<ID3D11BlendState> opaque_,alpha_,additive_;
  detail::Target world_,output_,light_;detail::Com<ID3D11Texture2D> depth_image_,staging_;
  detail::Com<ID3D11DepthStencilView> depth_view_;
  std::size_t vertex_capacity_=0,index_capacity_=0;
  std::map<std::uint64_t,detail::Texture> textures_;
  std::vector<detail::GpuVertex> vertices_;std::vector<UINT> indices_;
  std::vector<detail::Draw> draws_;std::vector<const Sprite*> sprite_order_;
  std::vector<std::uint8_t> readback_;
};
}  // namespace fable_gpu
#endif
