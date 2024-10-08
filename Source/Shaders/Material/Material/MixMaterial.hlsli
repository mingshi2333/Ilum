#ifndef MIX_BSDF_HLSLI
#define MIX_BSDF_HLSLI

#include "../BSDF/BSDF.hlsli"
#include "../../Math.hlsli"
#include "../../Random.hlsli"
#include "../../Interaction.hlsli"

template<typename T1, typename T2>
struct MixMaterial
{
    T1 material1;
    T2 material2;
    float weight;

    GBufferData GetGBufferData()
    {
        GBufferData data1 = material1.GetGBufferData();
        GBufferData data2 = material2.GetGBufferData();
        GBufferData data;
        data.normal = lerp(data1.normal, data2.normal, weight);
        data.albedo = lerp(data1.albedo, data2.albedo, weight);
        data.emissive = lerp(data1.emissive, data2.emissive, weight);
        data.metallic = lerp(data1.metallic, data2.metallic, weight);
        data.roughness = lerp(data1.roughness, data2.roughness, weight);
        data.anisotropic = lerp(data1.anisotropic, data2.anisotropic, weight);
        return data;

    }

    void Init(T1 material1_, T2 material2_, float weight_)
    {
        material1 = material1_;
        material2 = material2_;
        weight = weight_;
    }

    uint Flags()
    {
        return material1.Flags() | material2.Flags();
    }

    float3 Eval(float3 woW, float3 wiW, TransportMode mode)
    {
        if(weight <= 0)
        {
            return material1.Eval(woW, wiW, mode);
        }
        else if(weight >= 1)
        {
            return material2.Eval(woW, wiW, mode);
        }
        return lerp(material1.Eval(woW, wiW, mode), material2.Eval(woW, wiW, mode), weight);
        // PCGSampler rng;
        // rng.seed = HashCombine(Hash(woW), Hash(wiW));

        // if(rng.Get1D() < weight)
        // {
        //     return material1.Eval(woW, wiW, mode);
        // }
        // else
        // {
        //     return material2.Eval(woW, wiW, mode);
        // }
    }

    float PDF(float3 woW, float3 wiW, TransportMode mode, SampleFlags flags)
    {
        if(weight <= 0)
        {
            return material1.PDF(woW, wiW, mode, flags);
        }
        else if(weight >= 1)
        {
            return material2.PDF(woW, wiW, mode, flags);
        }
        return lerp(material1.PDF(woW, wiW, mode, flags), material2.PDF(woW, wiW, mode, flags), weight);

        // PCGSampler rng;
        // rng.seed = HashCombine(Hash(woW), Hash(wiW));

        // if(rng.Get1D() < weight)
        // {
        //     return material1.PDF(woW, wiW, mode, flags);
        // }
        // else
        // {
        //     return material2.PDF(woW, wiW, mode, flags);
        // }
    }

    BSDFSample Samplef(float3 woW, float uc, float2 u, TransportMode mode, SampleFlags flags)
    {
        if(weight <= 0)
        {
            return material1.Samplef(woW, uc, u, mode, flags);
        }
        else if(weight >= 1)
        {
            return material2.Samplef(woW, uc, u, mode, flags);
        }

        BSDFSample bsdf_sample;
        bsdf_sample.Init();

        if(uc < weight)
        {
            bsdf_sample = material1.Samplef(woW, uc, u, mode, flags);
            bsdf_sample.pdf *= weight;
        }
        else
        {
            bsdf_sample = material2.Samplef(woW, uc, u, mode, flags);
            bsdf_sample.pdf *= 1 - weight;
        }

        return bsdf_sample;
    }
};

#endif