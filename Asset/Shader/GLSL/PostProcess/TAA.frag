#version 450

layout(binding = 0) uniform sampler2D last_result;
layout(binding = 1) uniform sampler2D current_result;
layout(binding = 2) uniform sampler2D motion_vector_curvature;
layout(binding = 3) uniform sampler2D linear_depth;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;

layout(push_constant) uniform PushBlock{
    vec4 jitter;    // current jitter - prev jitter
    vec4 extent;    // 1.0/width, 1.0/height, width, height
    float feedback_min;
    float feedback_max;
}push_data;

const float EPS = 0.00000001;

// Find max depth in 3x3 fragment
vec3 find_motion_vector_uv(vec2 uv)
{
    vec2 dd = push_data.extent.xy;
    vec2 du = vec2(dd.x, 0.0);
    vec2 dv = vec2(0.0, dd.y);

    vec3 dtl = vec3(-1.0, -1.0, texture(linear_depth, uv - dv - du).r);
    vec3 dtc = vec3(0.0, -1.0, texture(linear_depth, uv - dv).r);
    vec3 dtr = vec3(1.0, -1.0, texture(linear_depth, uv - dv + du).r);

    vec3 dml = vec3(-1.0, 0.0, texture(linear_depth, uv - du).r);
    vec3 dmc = vec3(0.0, 0.0, texture(linear_depth, uv).r);
    vec3 dmr = vec3(1.0, 0.0, texture(linear_depth, uv + du).r);

    vec3 dbl = vec3(-1.0, 1.0, texture(linear_depth, uv + dv - du).r);
    vec3 dbc = vec3(0.0, 1.0, texture(linear_depth, uv + dv).r);
    vec3 dbr = vec3(1.0, 1.0, texture(linear_depth, uv + dv + du).r);

    vec3 dmin = dtl;

    if(dmin.z > dtc.z) dmin = dtc;
    if(dmin.z > dtr.z) dmin = dtr;

    if(dmin.z > dml.z) dmin = dml;
    if(dmin.z > dmc.z) dmin = dmc;
    if(dmin.z > dmr.z) dmin = dmr;

    if(dmin.z > dbl.z) dmin = dbl;
    if(dmin.z > dbc.z) dmin = dbc;
    if(dmin.z > dbr.z) dmin = dbr;

    return vec3(uv + dd.xy * dmin.xy, dmin.z);
}

float luminance(vec3 rgb)
{
    return max(dot(rgb, vec3(0.299, 0.587, 0.114)), 0.0001);
}

vec3 tonemap(vec3 x)
{
    return x / (1 + luminance(x));
}

vec3 inverse_tonemap(vec3 x)
{
    return x / (1 - luminance(x));
}

vec3 RGB2YCoCg(vec3 color)
{
    // Y = R/4 + G/2+B/4
    // Co = R/2 - B/2
    // Cg = -R/4 - G/2 - B/4
    return vec3(
        color.r / 4.0 + color.g / 2.0 + color.b / 4.0,
        color.r / 2.0 - color.b / 2.0,
        -color.r / 4.0 + color.g / 2.0 - color.b / 4.0
    );
}

vec3 YCoCg2RGB(vec3 color)
{
    // R = Y + Co - Cg
    // G = Y + Cg
    // B = Y - Co - Cg
    return clamp(vec3(
        color.x + color.y - color.z,
        color.x + color.z,
        color.x - color.y - color.z), 0.0, 1.0);
}

vec3 clip_aabb(vec3 aabb_min, vec3 aabb_max, vec3 prev_sample)
{
    vec3 p_clip = 0.5 * (aabb_max + aabb_min);
    vec3 e_clip = 0.5 * (aabb_max - aabb_min);

    vec3 v_clip = prev_sample - p_clip;
    vec3 v_unit = v_clip.xyz / e_clip;
    vec3 a_unit = abs(v_unit);
    float ma_unit = max(a_unit.x, max(a_unit.y, a_unit.z));
    if(ma_unit > 1.0)
    {
        return p_clip + v_clip / ma_unit;
    }
    else
    {
        return prev_sample;
    }
}

vec3 temporal_reprojection(vec2 motion_vector)
{
    vec2 uv = inUV;

    vec3 current_color = RGB2YCoCg(texture(current_result, uv).rgb);
    vec3 last_color = RGB2YCoCg(texture(last_result, inUV - motion_vector).rgb);

    // Clipping
    vec2 du = vec2(push_data.extent.x, 0.0);
    vec2 dv = vec2(0.0, push_data.extent.y);

    vec3 ctl = RGB2YCoCg(texture(current_result, uv - du - dv).rgb);
    vec3 ctc = RGB2YCoCg(texture(current_result, uv - dv).rgb);
    vec3 ctr = RGB2YCoCg(texture(current_result, uv + du - dv).rgb);

    vec3 cml = RGB2YCoCg(texture(current_result, uv - du).rgb);
    vec3 cmc = RGB2YCoCg(texture(current_result, uv).rgb);
    vec3 cmr = RGB2YCoCg(texture(current_result, uv + du).rgb);

    vec3 cbl = RGB2YCoCg(texture(current_result, uv - du + dv).rgb);
    vec3 cbc = RGB2YCoCg(texture(current_result, uv + dv).rgb);
    vec3 cbr = RGB2YCoCg(texture(current_result, uv + du + dv).rgb);

    // Variance clipping
    vec3 m1 =ctl + ctc + ctr + cml + cmc + cmr + cbl + cbc + cbr;
    vec3 m2 =ctl * ctl + ctc * ctc + ctr * ctr + cml * cml + cmc * cmc + cmr * cmr + cbl * cbl + cbc * cbc + cbr * cbr;
    
    vec3 mu = m1 / 9.0;
    vec3 sigma = sqrt(m2 / 9.0 - mu * mu);

    vec3 cmin = mu - 1.0 * sigma;
    vec3 cmax = mu + 1.0 * sigma;

    vec2 chroma_extent = 0.25 * 0.5 * vec2(cmax.r - cmin.r);
    vec2 chroma_center = current_color.gb;
    cmin.yz = chroma_center - chroma_extent;
    cmax.yz = chroma_center + chroma_extent;

    last_color = clip_aabb(cmin, cmax, last_color);

    // Sharpen
    current_color *= 5.0;
    current_color += -1.0 * cml;
    current_color += -1.0 * cmr;
    current_color += -1.0 * ctc;
    current_color += -1.0 * cbc;

    current_color = tonemap(current_color);
    last_color = tonemap(last_color);

    // Calculate mix weight
    float lum0 = current_color.r;
    float lum1 = last_color.r;

    float unbiased_diff = abs(lum0 - lum1) / max(lum0, max(lum1, 0.2));
    float unbiased_weight = 1.0 - unbiased_diff;
    float unbiased_weight_sqr = unbiased_weight * unbiased_weight;
    float k_feedback = mix(push_data.feedback_min, push_data.feedback_max, unbiased_weight_sqr);

    vec3 blended = mix(current_color.rgb, last_color.rgb, k_feedback);

    blended = inverse_tonemap(blended);
    blended = YCoCg2RGB(blended);

    return blended;
}

void main()
{
    // TODO: Unjitter?
    vec2 uv = inUV;

    // Find motion vector sample uv
    vec3 mv_uv = find_motion_vector_uv(uv);
    
    // Sample motion vector
    vec2 motion_vector = texture(motion_vector_curvature, mv_uv.xy).xy;
    float depth = mv_uv.z;

    vec3 color_temporal = temporal_reprojection(motion_vector);


    outColor = vec4(clamp(color_temporal, 0.0, 1.0), 1.0);
}