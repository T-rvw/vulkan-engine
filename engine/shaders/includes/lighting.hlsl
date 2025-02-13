Texture2D shadow_map : register(t3, space0);
Texture2D shadow_map_atlas : register(t4, space0);
TextureCube radiance_cubemap : register(t11, space0);
TextureCube irradiance_cubemap : register(t12, space0); // diffuse, ao
Texture2D brdf_lut : register(t13, space0); // specular
 
struct LightGridItem
{
    uint offset;
    uint lightsCount;
    uint projectorsCount;
};

struct LightItem
{
    float3 position;
    float attenuation;
    float3 color;
    float radius;
    float3 direction;
    float coneAngle;
    float4x4 projectionMatrix;
    float2 shadowmapScale;
    float2 shadowmapOffset;
    uint mask;
};

struct ProjectorItem
{
    float3 position;
    float attenuation;
    float4 color;
    float2 scale;
    float2 offset;
    float2 shadowmapScale;
    float2 shadowmapOffset;
    float4x4 projectionMatrix;
    float radius;
    uint mask;
};

cbuffer Lights : register(b5, space0)
{
    LightItem lights[100];
}

cbuffer Projectors : register(b6, space0)
{
    ProjectorItem projectors[100];
};

StructuredBuffer<LightGridItem> LightGrid : register(t7, space0);

StructuredBuffer<uint> LightIndices : register(t8, space0);;

#define CLUSTER_COUNT_X 4.0f
#define CLUSTER_COUNT_Y 4.0f
#define CLUSTER_COUNT_DEPTH 16.0f
#define CLUSTER_NEAR 0.1f
#define CLUSTER_FAR 50000.0f

float GetAttenuation(float distance, float radius)
{
    float lightInnerR = radius * 0.01;
    float d = max(distance, lightInnerR);
    return clamp(1.0 - pow(d / radius, 4.0), 0.0, 1.0) / (d * d + 1.0);
}

float GetSliceIndex(float depth)
{
    float index = LogBase(depth / CLUSTER_NEAR, (float)CLUSTER_FAR / (float)CLUSTER_NEAR) * CLUSTER_COUNT_DEPTH;
    return max(0.0f, index);
}

float calculateFragmentShadow(Texture2D shadow_map_tex, float2 uv, float fragmentDepth) {
  float shadow = 0.0;
  float bias = 0.0001;
  uint w;
  uint h;
  shadow_map_tex.GetDimensions(w, h);
  float2 texel_size = 1.0 / float2(w, h);

  [unroll]
  for(int x = -1; x <= 1; x++) {
    for(int y = -1; y <= 1; y++) {
      float closestDepth = shadow_map_tex.SampleLevel(SamplerLinearClamp, uv + float2(x, y) * texel_size, 0.0).r;
      shadow += fragmentDepth - bias > closestDepth ? 1.0 : 0.0;
    }
  }

  shadow /= 9.0;
  return shadow;
}