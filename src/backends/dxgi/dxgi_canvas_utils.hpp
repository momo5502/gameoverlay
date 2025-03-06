#pragma once

#include "dxgi_utils.hpp"

namespace gameoverlay::dxgi
{
    struct vertex
    {
        vertex() = default;
        vertex(const float x, const float y, const float z, const float u, const float v, const COLORREF col)
            : pos(x, y, z),
              tex_coord(u, v),
              color(col)
        {
        }

        DirectX::XMFLOAT3 pos{};
        DirectX::XMFLOAT2 tex_coord{};
        COLORREF color{};
    };

    constexpr std::string_view vertex_shader_src = R"(
            struct VertexIn {
                float3 Pos : POSITION;
                float2 Tex : TEXCOORD;
                float4 Color : COLOR;
            };

            struct VertexOut {
                float4 Pos : SV_POSITION;
                float2 Tex : TEXCOORD;
                float4 Color : COLOR;
            };

            VertexOut VS(VertexIn vin) {
                VertexOut vout;
                vout.Pos = float4(vin.Pos, 1.0f);
                vout.Tex = vin.Tex;
                vout.Color = vin.Color;
                return vout;
            }
        )";

    constexpr std::string_view pixel_shader_src = R"(
            Texture2D SpriteTex;
            SamplerState samLinear;

            struct VertexOut {
                float4 Pos : SV_POSITION;
                float2 Tex : TEXCOORD;
                float4 Color : COLOR;
            };

            float4 PS(VertexOut pin) : SV_Target {
                return pin.Color * SpriteTex.Sample(samLinear, pin.Tex);
            }
        )";

    inline void translate_vertices(vertex* v, const int32_t x, const int32_t y, const COLORREF color,
                                   const dimensions dim)
    {
        const auto f_x = static_cast<float>(x);
        const auto f_y = static_cast<float>(y);

        const auto f_width = static_cast<float>(dim.width);
        const auto f_height = static_cast<float>(dim.height);

        const auto w1 = 2.0f * f_x / f_width - 1.0f;
        const auto w2 = 2.0f * (f_x + f_width) / f_width - 1.0f;

        const auto h1 = 1.0f - 2.0f * f_y / f_height;
        const auto h2 = 1.0f - 2.0f * (f_y + f_height) / f_height;

        v[0] = vertex(w1, h1, 0.5f, 0.0f, 0.0f, color);
        v[1] = vertex(w2, h1, 0.5f, 1.0f, 0.0f, color);
        v[2] = vertex(w2, h2, 0.5f, 1.0f, 1.0f, color);
        v[3] = vertex(w1, h2, 0.5f, 0.0f, 1.0f, color);
    }
}
