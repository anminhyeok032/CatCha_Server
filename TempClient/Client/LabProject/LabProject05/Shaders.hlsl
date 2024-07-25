
cbuffer cbCameraInfo : register(b0)
{
	matrix		gmtxView : packoffset(c0);
	matrix		gmtxProjection : packoffset(c4);
	float3		gvCameraPosition : packoffset(c8);
};

cbuffer cbGameObjectInfo : register(b1)
{
	matrix		gmtxGameObject : packoffset(c0);
};

#include "Light.hlsl"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
struct VS_DIFFUSED_INPUT
{
	float3 position : POSITION;
    //float4 color : COLOR;
};

struct VS_DIFFUSED_OUTPUT
{
	float4 position : SV_POSITION;
};

VS_DIFFUSED_OUTPUT VSDiffused(VS_DIFFUSED_INPUT input)
{
	VS_DIFFUSED_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);

	return(output);
}

float4 PSDiffused(VS_DIFFUSED_OUTPUT input) : SV_TARGET
{
    return (float4(1.0, 0.0, 0.0, 1.0));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define MAX_VERTEX_INFLUENCES 4
#define SKINNED_ANIMATION_BONES 128

cbuffer cbBoneOffsets : register(b5)
{
    float4x4 gpmtxBoneOffsets[SKINNED_ANIMATION_BONES];
};

cbuffer cbBoneTransforms : register(b6)
{
    float4x4 gpmtxBoneTransforms[SKINNED_ANIMATION_BONES];
};

struct VS_SKIN_ANIMATION_DIFFUSED_INPUT
{
    float3 position : POSITION;
    int4 indices : BONEINDEX;
    float4 weights : BONEWEIGHT;
};

struct VS_SKIN_ANIMATION_DIFFUSED_OUTPUT
{
    float4 position : SV_POSITION;
};

VS_SKIN_ANIMATION_DIFFUSED_OUTPUT VSSkinAnimationDiffused(VS_SKIN_ANIMATION_DIFFUSED_INPUT input)
{
    VS_SKIN_ANIMATION_DIFFUSED_OUTPUT output;

    float3 positionW = float3(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < MAX_VERTEX_INFLUENCES; i++)
    {
        positionW += input.weights[i] * mul(mul(float4(input.position, 1.0f), gpmtxBoneOffsets[input.indices[i]]), gpmtxBoneTransforms[input.indices[i]]).xyz;
    }
    
    output.position = mul(mul(float4(positionW, 1.0f), gmtxView), gmtxProjection);

    return (output);
}

float4 PSSkinAnimationDiffused(VS_SKIN_ANIMATION_DIFFUSED_OUTPUT input) : SV_TARGET
{
    return (float4(0.0, 1.0, 0.0, 1.0));
}

//|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

Texture2D gtxtDiffuseColor : register(t0);
SamplerState gssWrap : register(s0);

struct VS_STANDARD_INPUT
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    int4 indices : BONEINDEX;
    float4 weights : BONEWEIGHT;

};

struct VS_STANDARD_OUTPUT
{
    float4 position : SV_POSITION; 
    float2 uv : TEXCOORD;
};

VS_STANDARD_OUTPUT VSStandard(VS_STANDARD_INPUT input)
{
    VS_STANDARD_OUTPUT output;

    float3 positionW = float3(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < MAX_VERTEX_INFLUENCES; i++)
    {
        positionW += input.weights[i] * mul(mul(float4(input.position, 1.0f), gpmtxBoneOffsets[input.indices[i]]), gpmtxBoneTransforms[input.indices[i]]).xyz;
    }
    
    output.position = mul(mul(float4(positionW, 1.0f), gmtxView), gmtxProjection);
    output.uv = input.uv;
    
    return (output);
}

float4 PSStandard(VS_STANDARD_OUTPUT input) : SV_TARGET
{
    float4 Color = gtxtDiffuseColor.Sample(gssWrap, input.uv);
        
    return (Color);
}

//|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||


struct VS_STATICMESH_INPUT
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
};

struct VS_STATICMESH_OUTPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

VS_STATICMESH_OUTPUT VSStaticMesh(VS_STATICMESH_INPUT input)
{
    VS_STATICMESH_OUTPUT output;

    output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
    output.uv = input.uv;
    return (output);
}

float4 PSStaticMesh(VS_STATICMESH_OUTPUT input) : SV_TARGET
{
    float4 Color = gtxtDiffuseColor.Sample(gssWrap, input.uv);
        
    return (Color);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Texture2D gtxtTerrainTexture : register(t17);
Texture2D gtxtDetailTexture : register(t18);

struct VS_TERRAIN_INPUT
{
    float3 position : POSITION;
    float4 color : COLOR;
    float2 uv0 : TEXCOORD0;
    float2 uv1 : TEXCOORD1;
};

struct VS_TERRAIN_OUTPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 uv0 : TEXCOORD0;
    float2 uv1 : TEXCOORD1;
};

VS_TERRAIN_OUTPUT VSTerrain(VS_TERRAIN_INPUT input)
{
    VS_TERRAIN_OUTPUT output;

    output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
    output.color = input.color;
    output.uv0 = input.uv0;
    output.uv1 = input.uv1;

    return (output);
}

float4 PSTerrain(VS_TERRAIN_OUTPUT input) : SV_TARGET
{
    float4 cBaseTexColor = gtxtTerrainTexture.Sample(gssWrap, input.uv0);
    float4 cDetailTexColor = gtxtDetailTexture.Sample(gssWrap, input.uv1);
	//	float fAlpha = gtxtTerrainTexture.Sample(gssWrap, input.uv0);

    float4 cColor = cBaseTexColor * 0.7f + cDetailTexColor * 0.3f;
	//	float4 cColor = saturate(lerp(cBaseTexColor, cDetailTexColor, fAlpha));
    //cColor = float4(0.0, 1.0, 0.0, 1.0);
    return (cColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
struct VS_SKYBOX_CUBEMAP_INPUT
{
    float3 position : POSITION;
};

struct VS_SKYBOX_CUBEMAP_OUTPUT
{
    float3 positionL : POSITION;
    float4 position : SV_POSITION;
};

VS_SKYBOX_CUBEMAP_OUTPUT VSSkyBox(VS_SKYBOX_CUBEMAP_INPUT input)
{
    VS_SKYBOX_CUBEMAP_OUTPUT output;

    output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection).xyww;
    output.positionL = input.position;

    return (output);
}

TextureCube gtxtSkyCubeTexture : register(t19);
SamplerState gssClamp : register(s1);

float4 PSSkyBox(VS_SKYBOX_CUBEMAP_OUTPUT input) : SV_TARGET
{
    float4 cColor = gtxtSkyCubeTexture.Sample(gssClamp, input.positionL);

    return (cColor);
}

//|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//#define _WITH_VERTEX_LIGHTING


struct VS_LIGHTING_INPUT
{
    float3 position : POSITION;
    float3 normal : NORMAL;
};

struct VS_LIGHTING_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float3 normalW : NORMAL;
//	nointerpolation float3 normalW : NORMAL;
#ifdef _WITH_VERTEX_LIGHTING
	float4 color : COLOR;
#endif
};

VS_LIGHTING_OUTPUT VSLighting(VS_LIGHTING_INPUT input)
{
    VS_LIGHTING_OUTPUT output;

    output.normalW = mul(input.normal, (float3x3) gmtxGameObject);
    output.positionW = (float3) mul(float4(input.position, 1.0f), gmtxGameObject);
    output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
#ifdef _WITH_VERTEX_LIGHTING
	output.normalW = normalize(output.normalW);
	output.color = Lighting(output.positionW, output.normalW);
#endif
    return (output);
}

float4 PSLighting(VS_LIGHTING_OUTPUT input) : SV_TARGET
{
#ifdef _WITH_VERTEX_LIGHTING
	return(input.color);
#else
    input.normalW = normalize(input.normalW);
    float4 color = Lighting(input.positionW, input.normalW);
    return (color);
#endif
}
