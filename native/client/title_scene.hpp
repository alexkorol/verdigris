#pragma once

// Native presentation adapter for WIZARD/verdigris_splash. The authored GLB
// remains a real, depth-tested mesh; no game state or browser runtime lives here.
// D3D renders a bounded offscreen target, then the existing Framekit compositor
// places its accessible menu over it. Resources are created once, not per input.
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <filesystem>
#include <stdexcept>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

namespace verdigris::client {

struct TitleOrbit {
  float yaw = 0, pitch = 0, zoom = 1;
  bool dragging = false;
  POINT previous{};
  void reset() { yaw = pitch = 0; zoom = 1; dragging = false; }
  void drag(POINT point) {
    if (dragging) {
      yaw = std::remainder(yaw + (point.x - previous.x) * .004f, 6.2831853f);
      pitch = std::clamp(pitch + (point.y - previous.y) * .003f, -.7f, .6f);
    }
    previous = point;
  }
  void wheel(int delta) {
    zoom = std::clamp(zoom * std::pow(.9f, static_cast<float>(delta) / WHEEL_DELTA), .78f, 1.35f);
  }
};

class TitleScene {
  template<class T> using Ptr = Microsoft::WRL::ComPtr<T>;
  Ptr<ID3D11Device> device_;
  Ptr<ID3D11DeviceContext> context_;
  Ptr<ID3D11VertexShader> mesh_vs_, sky_vs_;
  Ptr<ID3D11PixelShader> mesh_ps_, sky_ps_, upscale_ps_;
  Ptr<ID3D11InputLayout> layout_;
  Ptr<ID3D11Buffer> vertices_, indices_, constants_;
  Ptr<ID3D11SamplerState> sampler_;
  Ptr<ID3D11RasterizerState> raster_;
  Ptr<ID3D11DepthStencilState> depth_on_, depth_off_;
  Ptr<ID3D11ShaderResourceView> top_, underside_, lights_;
  Ptr<ID3D11Texture2D> color_, depth_, staging_, output_;
  Ptr<ID3D11RenderTargetView> target_, output_target_;
  Ptr<ID3D11ShaderResourceView> canvas_view_;
  Ptr<ID3D11DepthStencilView> depth_view_;
  std::vector<unsigned char> pixels_;
  int width_ = 0, height_ = 0;
  int output_width_ = 0, output_height_ = 0;
  UINT index_count_ = 0;
  bool attempted_ = false;
  std::string error_;
  struct Vertex { float position[3], normal[3]; };
  struct Constants {
    DirectX::XMFLOAT4X4 transform;
    DirectX::XMFLOAT4 eye_time;
    DirectX::XMFLOAT4 viewport;
  };
  static void check(HRESULT hr, const char* step) {
    if (FAILED(hr)) throw std::runtime_error(std::string(step) + " (" + std::to_string(hr) + ")");
  }
  static std::vector<unsigned char> read(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary | std::ios::ate);
    if (!input) throw std::runtime_error("Missing title asset: " + path.filename().string());
    const auto length = input.tellg();
    if (length < 20 || length > 32000000) throw std::runtime_error("Invalid title asset size");
    std::vector<unsigned char> bytes(static_cast<size_t>(length));
    input.seekg(0);
    if (!input.read(reinterpret_cast<char*>(bytes.data()), length)) throw std::runtime_error("Title asset read failed");
    return bytes;
  }
  static unsigned word(const std::vector<unsigned char>& b, size_t at) {
    if (at > b.size() || b.size() - at < 4) throw std::runtime_error("Truncated island mesh");
    unsigned result; std::memcpy(&result, b.data() + at, 4); return result;
  }
  void load_mesh(const std::filesystem::path& path) {
    // This adapter deliberately supports the checked-in, identity-transformed
    // tapered export only, not arbitrary glTF skins/materials/transforms.
    const auto bytes = read(path);
    if (word(bytes, 0) != 0x46546c67 || word(bytes, 4) != 2 || word(bytes, 8) != bytes.size() || word(bytes, 16) != 0x4e4f534a)
      throw std::runtime_error("Unsupported island GLB header");
    const size_t json_size = word(bytes, 12), bin_header = 20 + json_size;
    if (bin_header + 8 > bytes.size() || word(bytes, bin_header + 4) != 0x004e4942)
      throw std::runtime_error("Missing island geometry chunk");
    const size_t bin = bin_header + 8, bin_size = word(bytes, bin_header);
    if (bin_size != bytes.size() - bin) throw std::runtime_error("Truncated island geometry");
    networking::JsonValue json;
    if (!networking::parse_json(std::string(reinterpret_cast<const char*>(bytes.data() + 20), json_size), json))
      throw std::runtime_error("Invalid island metadata");
    const auto* accessors = json["accessors"].array();
    const auto* views = json["bufferViews"].array();
    if (!accessors || accessors->size() != 4 || !views || views->size() != 4)
      throw std::runtime_error("Unsupported island export layout");
    auto number = [](const networking::JsonValue& v, const char* key) {
      return v[key].number().value_or(-1);
    };
    auto source = [&](size_t index, int type, int count, size_t stride) {
      const auto& a = (*accessors)[index]; const auto& v = (*views)[index];
      const double offset = number(v, "byteOffset");
      if (number(a, "bufferView") != index || number(a, "componentType") != type ||
          number(a, "count") != count || number(v, "buffer") != 0 ||
          a["byteOffset"].number().value_or(0) != 0 ||
          v["byteStride"].number().value_or(static_cast<double>(stride)) != stride ||
          offset < 0 || offset != std::floor(offset) || offset > bin_size ||
          number(v, "byteLength") != count * stride || count * stride > bin_size - static_cast<size_t>(offset))
        throw std::runtime_error("Unexpected island vertex/index layout");
      return bytes.data() + bin + static_cast<size_t>(offset);
    };
    const auto* positions = source(0, 5126, 61645, 12);
    const auto* normals = source(1, 5126, 61645, 12);
    const auto* indices = source(3, 5123, 357777, 2);
    std::vector<Vertex> vertices(61645);
    for (size_t i = 0; i < vertices.size(); ++i) {
      std::memcpy(vertices[i].position, positions + i * 12, 12);
      std::memcpy(vertices[i].normal, normals + i * 12, 12);
      for (float f : vertices[i].position) if (!std::isfinite(f) || std::abs(f) > 20) throw std::runtime_error("Invalid island vertex");
      for (float f : vertices[i].normal) if (!std::isfinite(f) || std::abs(f) > 1.01f) throw std::runtime_error("Invalid island normal");
    }
    for (size_t i = 0; i < 357777; ++i) {
      unsigned short index; std::memcpy(&index, indices + i * 2, 2);
      if (index >= vertices.size()) throw std::runtime_error("Invalid island triangle");
    }
    D3D11_BUFFER_DESC desc{}; desc.Usage = D3D11_USAGE_IMMUTABLE; desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    desc.ByteWidth = static_cast<UINT>(vertices.size() * sizeof(Vertex));
    D3D11_SUBRESOURCE_DATA data{}; data.pSysMem = vertices.data();
    check(device_->CreateBuffer(&desc, &data, &vertices_), "Island vertices");
    desc.BindFlags = D3D11_BIND_INDEX_BUFFER; desc.ByteWidth = 357777 * 2; data.pSysMem = indices;
    check(device_->CreateBuffer(&desc, &data, &indices_), "Island triangles");
    index_count_ = 357777;
  }
  void texture(const std::filesystem::path& path, Ptr<ID3D11ShaderResourceView>& view) {
    skin::ensure_started();
    Gdiplus::Bitmap image(path.c_str());
    if (image.GetLastStatus() != Gdiplus::Ok || image.GetWidth() > 4096 || image.GetHeight() > 4096 || !image.GetWidth() || !image.GetHeight())
      throw std::runtime_error("Cannot decode title texture: " + path.filename().string());
    Gdiplus::Rect rect(0, 0, image.GetWidth(), image.GetHeight());
    Gdiplus::BitmapData locked{};
    if (image.LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &locked) != Gdiplus::Ok)
      throw std::runtime_error("Cannot lock title texture");
    std::vector<unsigned char> rgba(static_cast<size_t>(rect.Width) * rect.Height * 4);
    for (int y = 0; y < rect.Height; ++y)
      std::memcpy(rgba.data() + static_cast<size_t>(y) * rect.Width * 4, static_cast<unsigned char*>(locked.Scan0) + y * locked.Stride, static_cast<size_t>(rect.Width) * 4);
    image.UnlockBits(&locked);
    D3D11_TEXTURE2D_DESC desc{}; desc.Width = rect.Width; desc.Height = rect.Height;
    desc.MipLevels = 1; desc.ArraySize = 1; desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count = 1; desc.Usage = D3D11_USAGE_IMMUTABLE; desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    D3D11_SUBRESOURCE_DATA data{}; data.pSysMem = rgba.data(); data.SysMemPitch = rect.Width * 4;
    Ptr<ID3D11Texture2D> tex;
    check(device_->CreateTexture2D(&desc, &data, &tex), "Island texture");
    check(device_->CreateShaderResourceView(tex.Get(), nullptr, &view), "Island texture view");
  }
  static constexpr const char* shader_ = R"hlsl(
cbuffer Camera : register(b0) { row_major float4x4 transform; float4 eyeTime; float4 viewport; };
Texture2D terrain : register(t0); Texture2D underside : register(t1); Texture2D illumination : register(t2);
Texture2D canvas : register(t3);
SamplerState surfaceSampler : register(s0);
struct Pixel { float4 position : SV_POSITION; float3 local : TEXCOORD0; float3 normal : TEXCOORD1; };
Pixel meshVS(float3 position : POSITION, float3 normal : NORMAL) {
  Pixel o; o.position = mul(float4(position, 1), transform); o.local = position; o.normal = normal; return o;
}
float hash(float2 p) { return frac(sin(dot(p,float2(127.1,311.7))) * 43758.5453); }
float noise(float2 p) {
 float2 i=floor(p), f=frac(p); f=f*f*(3-2*f);
 return lerp(lerp(hash(i),hash(i+float2(1,0)),f.x),lerp(hash(i+float2(0,1)),hash(i+1),f.x),f.y);
}
float3 finish(float3 c, float2 uv) {
 float vignette = saturate(1.15 - length((uv-.5)*float2(1.2,1))*.62);
 // Authored readable left menu space, not a panel covering the island.
 return c * vignette * lerp(.3,1,smoothstep(0,.46,uv.x));
}
float4 meshPS(Pixel i) : SV_TARGET {
 float2 uv=(i.local.xz+7.4)/14.8; uv.y=1-uv.y;
 float3 n=normalize(i.normal);
 float topMask=smoothstep(.05,.32,n.y)*smoothstep(.08,.12,i.local.y);
 float3 atlas=terrain.Sample(surfaceSampler,uv).rgb;
 float3 rock=underside.Sample(surfaceSampler,uv).rgb;
 float3 base=lerp(rock,atlas,topMask);
 float diffuse=max(0,dot(n,normalize(float3(-9,10.5,7.5))));
 float rim=max(0,dot(n,normalize(float3(10,2.8,-7))));
 float under=max(0,dot(n,normalize(float3(-5,-11,4))));
 float3 light=float3(.36,.4,.4)+diffuse*float3(.95,.79,.59)+rim*float3(.14,.26,.29)+under*float3(.28,.32,.37);
 float water=smoothstep(.03,.13,min(atlas.g,atlas.b)-atlas.r)*topMask;
 float ripple=sin(i.local.x*13+eyeTime.w*.8)*sin(i.local.z*17-eyeTime.w*.65)*.5+.5;
 float3 view=normalize(eyeTime.xyz-i.local);
 float glint=pow(saturate(dot(reflect(-normalize(float3(-9,10.5,7.5)),n),view)),32);
 float3 city=illumination.Sample(surfaceSampler,uv).rgb * topMask;
 float3 c=base*light + water*(ripple*.045+glint*.32)*float3(.4,.8,.85)+city*.25;
 c=c/(.7+c)*1.18;
 return float4(finish(c,i.position.xy/viewport.xy),1);
}
Pixel skyVS(uint id : SV_VertexID) {
 Pixel o; float2 uv=float2((id<<1)&2,id&2); o.position=float4(uv*float2(2,-2)+float2(-1,1),1,1); o.local=float3(uv,0); o.normal=0; return o;
}
float4 skyPS(Pixel i) : SV_TARGET {
 float2 uv=i.local.xy; float2 p=uv*float2(viewport.x/viewport.y,1);
 float band=exp(-pow((uv.y-.34+sin(uv.x*4)*.1)*5,2));
 float nebula=noise(p*5+eyeTime.w*.001)*.6+noise(p*13)*.25+noise(p*34)*.15;
 float3 c=lerp(float3(.014,.025,.04),float3(.09,.16,.18),band*nebula);
 float2 cells=p*650; float star=step(.9988,hash(floor(cells)))*pow(saturate(1-length(frac(cells)-.5)*2),5);
 c+=star*float3(.5,.65,.65);
 float cloud=pow(saturate(noise(p*4+float2(eyeTime.w*.005,0))*.6+noise(p*10)*.4),3)*smoothstep(.48,1,uv.y);
 c+=cloud*float3(.1,.18,.18);
 return float4(finish(c,uv),1);
}
float4 upscalePS(Pixel i) : SV_TARGET { return canvas.Sample(surfaceSampler,i.local.xy); }
)hlsl";
  Ptr<ID3DBlob> compile(const char* entry, const char* profile) {
    Ptr<ID3DBlob> code, errors;
    const HRESULT hr = D3DCompile(shader_, std::strlen(shader_), "wizard-title", nullptr, nullptr, entry, profile, D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &code, &errors);
    if (FAILED(hr)) throw std::runtime_error(errors ? std::string(static_cast<const char*>(errors->GetBufferPointer()), errors->GetBufferSize()) : "Title shader compile failed");
    return code;
  }
  void initialize(const std::filesystem::path& root) {
    D3D_FEATURE_LEVEL level;
    const D3D_FEATURE_LEVEL requested[] = { D3D_FEATURE_LEVEL_11_0 };
    HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, requested, 1, D3D11_SDK_VERSION, &device_, &level, &context_);
    if (FAILED(hr)) hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0, requested, 1, D3D11_SDK_VERSION, &device_, &level, &context_);
    check(hr, "Title graphics device");
    const auto vs = compile("meshVS", "vs_5_0"), ps = compile("meshPS", "ps_5_0"), skyvs = compile("skyVS", "vs_5_0"), skyps = compile("skyPS", "ps_5_0");
    check(device_->CreateVertexShader(vs->GetBufferPointer(), vs->GetBufferSize(), nullptr, &mesh_vs_), "Island shader");
    check(device_->CreatePixelShader(ps->GetBufferPointer(), ps->GetBufferSize(), nullptr, &mesh_ps_), "Island lighting");
    check(device_->CreateVertexShader(skyvs->GetBufferPointer(), skyvs->GetBufferSize(), nullptr, &sky_vs_), "Sky shader");
    check(device_->CreatePixelShader(skyps->GetBufferPointer(), skyps->GetBufferSize(), nullptr, &sky_ps_), "Sky lighting");
    const auto upscale = compile("upscalePS", "ps_5_0");
    check(device_->CreatePixelShader(upscale->GetBufferPointer(),upscale->GetBufferSize(),nullptr,&upscale_ps_), "Title upscale");
    const D3D11_INPUT_ELEMENT_DESC elements[] = { {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0}, {"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0} };
    check(device_->CreateInputLayout(elements,2,vs->GetBufferPointer(),vs->GetBufferSize(),&layout_), "Island layout");
    D3D11_BUFFER_DESC buffer{}; buffer.ByteWidth=sizeof(Constants); buffer.Usage=D3D11_USAGE_DEFAULT; buffer.BindFlags=D3D11_BIND_CONSTANT_BUFFER;
    check(device_->CreateBuffer(&buffer,nullptr,&constants_), "Title camera");
    D3D11_SAMPLER_DESC sample{}; sample.Filter=D3D11_FILTER_MIN_MAG_MIP_LINEAR; sample.AddressU=sample.AddressV=sample.AddressW=D3D11_TEXTURE_ADDRESS_CLAMP; sample.MaxLOD=D3D11_FLOAT32_MAX;
    check(device_->CreateSamplerState(&sample,&sampler_), "Island sampler");
    D3D11_RASTERIZER_DESC raster{}; raster.FillMode=D3D11_FILL_SOLID; raster.CullMode=D3D11_CULL_NONE; raster.DepthClipEnable=TRUE;
    check(device_->CreateRasterizerState(&raster,&raster_), "Title rasterizer");
    D3D11_DEPTH_STENCIL_DESC ds{}; ds.DepthEnable=TRUE; ds.DepthWriteMask=D3D11_DEPTH_WRITE_MASK_ALL; ds.DepthFunc=D3D11_COMPARISON_LESS;
    check(device_->CreateDepthStencilState(&ds,&depth_on_), "Island depth");
    ds.DepthEnable=FALSE; ds.DepthWriteMask=D3D11_DEPTH_WRITE_MASK_ZERO;
    check(device_->CreateDepthStencilState(&ds,&depth_off_), "Sky depth");
    load_mesh(root / "world/celestial_world_runtime_tapered.glb");
    texture(root / "world/celestial_world_top_texture_4k_detail.png", top_);
    texture(root / "world/celestial_world_underside_texture.png", underside_);
    texture(root / "world/celestial_world_illumination_map_4k.png", lights_);
  }
  void resize(int width, int height, int output_width, int output_height) {
    if (width_ == width && height_ == height && output_width_ == output_width && output_height_ == output_height) return;
    context_->OMSetRenderTargets(0,nullptr,nullptr);
    ID3D11ShaderResourceView* empty=nullptr; context_->PSSetShaderResources(3,1,&empty);
    target_.Reset(); depth_view_.Reset(); color_.Reset(); staging_.Reset(); depth_.Reset(); canvas_view_.Reset(); output_.Reset(); output_target_.Reset();
    width_=height_=0;
    D3D11_TEXTURE2D_DESC desc{}; desc.Width=width; desc.Height=height; desc.MipLevels=1; desc.ArraySize=1; desc.Format=DXGI_FORMAT_B8G8R8A8_UNORM; desc.SampleDesc.Count=1; desc.BindFlags=D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE;
    check(device_->CreateTexture2D(&desc,nullptr,&color_), "Title canvas");
    check(device_->CreateRenderTargetView(color_.Get(),nullptr,&target_), "Title canvas view");
    check(device_->CreateShaderResourceView(color_.Get(),nullptr,&canvas_view_), "Title upscale source");
    desc.Width=output_width; desc.Height=output_height; desc.BindFlags=D3D11_BIND_RENDER_TARGET;
    check(device_->CreateTexture2D(&desc,nullptr,&output_), "Title output");
    check(device_->CreateRenderTargetView(output_.Get(),nullptr,&output_target_), "Title output view");
    desc.BindFlags=0; desc.Usage=D3D11_USAGE_STAGING; desc.CPUAccessFlags=D3D11_CPU_ACCESS_READ;
    check(device_->CreateTexture2D(&desc,nullptr,&staging_), "Title readback");
    desc.Width=width; desc.Height=height;
    desc.BindFlags=D3D11_BIND_DEPTH_STENCIL; desc.Format=DXGI_FORMAT_D24_UNORM_S8_UINT; desc.Usage=D3D11_USAGE_DEFAULT; desc.CPUAccessFlags=0;
    check(device_->CreateTexture2D(&desc,nullptr,&depth_), "Title depth buffer");
    check(device_->CreateDepthStencilView(depth_.Get(),nullptr,&depth_view_), "Title depth view");
    pixels_.resize(static_cast<size_t>(output_width)*output_height*4); width_=width; height_=height;
    output_width_=output_width; output_height_=output_height;
  }
 public:
  const std::string& error() const { return error_; }
  unsigned triangles() const { return index_count_/3; }
  bool draw(HDC dc, const RECT& bounds, const std::filesystem::path& root, const TitleOrbit& orbit, float seconds) {
    if (!error_.empty()) return false;
    try {
      if (!attempted_) { attempted_=true; initialize(root); }
      const int output_w=bounds.right-bounds.left, output_h=bounds.bottom-bounds.top;
      if (output_w<=0 || output_h<=0) return false;
      const float scale=std::min({1.f, 1600.f/output_w, 900.f/output_h});
      resize(std::max(1,static_cast<int>(output_w*scale)),std::max(1,static_cast<int>(output_h*scale)),output_w,output_h);
      using namespace DirectX;
      const float aspect=static_cast<float>(width_)/height_;
      const float yaw=.52f+orbit.yaw+std::sin(seconds*.17453f)*.018f;
      const float pitch=std::clamp(.46f+orbit.pitch,-.26f,1.18f);
      const float distance=(aspect<1.25f?29.f:24.f)*orbit.zoom;
      const XMVECTOR eye=XMVectorSet(std::sin(yaw)*std::cos(pitch)*distance,std::sin(pitch)*distance,std::cos(yaw)*std::cos(pitch)*distance,1);
      const XMVECTOR target=XMVectorSet(0,-.5f,0,1);
      const auto view=XMMatrixLookAtRH(eye,target,XMVectorSet(0,1,0,0));
      auto projection=XMMatrixPerspectiveFovRH(XMConvertToRadians(37),aspect,.08f,150);
      // Shift the actual world toward the free right-hand canvas.
      projection.r[2]=XMVectorSet(-.30f,0,XMVectorGetZ(projection.r[2]),XMVectorGetW(projection.r[2]));
      Constants c{}; XMStoreFloat4x4(&c.transform,view*projection); XMStoreFloat4(&c.eye_time,eye); c.eye_time.w=seconds; c.viewport={static_cast<float>(width_),static_cast<float>(height_),0,0};
      context_->UpdateSubresource(constants_.Get(),0,nullptr,&c,0,0);
      ID3D11Buffer* cb=constants_.Get(); context_->VSSetConstantBuffers(0,1,&cb); context_->PSSetConstantBuffers(0,1,&cb);
      const D3D11_VIEWPORT vp{0,0,static_cast<float>(width_),static_cast<float>(height_),0,1}; context_->RSSetViewports(1,&vp); context_->RSSetState(raster_.Get());
      ID3D11ShaderResourceView* empty=nullptr; context_->PSSetShaderResources(3,1,&empty);
      ID3D11RenderTargetView* rtv=target_.Get(); context_->OMSetRenderTargets(1,&rtv,depth_view_.Get());
      context_->ClearDepthStencilView(depth_view_.Get(),D3D11_CLEAR_DEPTH,1,0);
      context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
      context_->IASetInputLayout(nullptr); context_->OMSetDepthStencilState(depth_off_.Get(),0);
      context_->VSSetShader(sky_vs_.Get(),nullptr,0); context_->PSSetShader(sky_ps_.Get(),nullptr,0); context_->Draw(3,0);
      context_->IASetInputLayout(layout_.Get()); context_->OMSetDepthStencilState(depth_on_.Get(),0);
      ID3D11Buffer* vb=vertices_.Get(); const UINT stride=sizeof(Vertex), offset=0; context_->IASetVertexBuffers(0,1,&vb,&stride,&offset); context_->IASetIndexBuffer(indices_.Get(),DXGI_FORMAT_R16_UINT,0);
      ID3D11ShaderResourceView* textures[]={top_.Get(),underside_.Get(),lights_.Get()}; context_->PSSetShaderResources(0,3,textures);
      ID3D11SamplerState* sampler=sampler_.Get(); context_->PSSetSamplers(0,1,&sampler);
      context_->VSSetShader(mesh_vs_.Get(),nullptr,0); context_->PSSetShader(mesh_ps_.Get(),nullptr,0); context_->DrawIndexed(index_count_,0,0);
      // Scale with GPU bilinear filtering. GDI HALFTONE scaling of a full
      // ultrawide frame cost more than the entire gameplay frame budget.
      ID3D11RenderTargetView* output_target=output_target_.Get(); context_->OMSetRenderTargets(1,&output_target,nullptr);
      const D3D11_VIEWPORT output_vp{0,0,static_cast<float>(output_w),static_cast<float>(output_h),0,1}; context_->RSSetViewports(1,&output_vp);
      context_->OMSetDepthStencilState(depth_off_.Get(),0); context_->IASetInputLayout(nullptr);
      ID3D11ShaderResourceView* canvas=canvas_view_.Get(); context_->PSSetShaderResources(3,1,&canvas);
      context_->VSSetShader(sky_vs_.Get(),nullptr,0); context_->PSSetShader(upscale_ps_.Get(),nullptr,0); context_->Draw(3,0);
      context_->CopyResource(staging_.Get(),output_.Get());
      D3D11_MAPPED_SUBRESOURCE mapped{}; check(context_->Map(staging_.Get(),0,D3D11_MAP_READ,0,&mapped), "Title frame readback");
      for (int y=0;y<output_h;++y) std::memcpy(pixels_.data()+static_cast<size_t>(y)*output_w*4,static_cast<const unsigned char*>(mapped.pData)+static_cast<size_t>(y)*mapped.RowPitch,static_cast<size_t>(output_w)*4);
      context_->Unmap(staging_.Get(),0);
      BITMAPINFO info{}; info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER); info.bmiHeader.biWidth=output_w; info.bmiHeader.biHeight=-output_h; info.bmiHeader.biPlanes=1; info.bmiHeader.biBitCount=32; info.bmiHeader.biCompression=BI_RGB;
      SetDIBitsToDevice(dc,bounds.left,bounds.top,output_w,output_h,0,0,0,output_h,pixels_.data(),&info,DIB_RGB_COLORS);
      return true;
    } catch (const std::exception& error) { error_=error.what(); return false; }
  }
};
} // namespace verdigris::client
