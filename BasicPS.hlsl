//float4 main() : SV_TARGET
//{
//	return float4(0.5f, 0.55f, 0.5f, 1.0f);
//}
//cbuffer ConstBufferDataMaterial : register(b0)
//{
//    float4 color; // �F(RGBA)
//};
//float4 main() : SV_TARGET
//{
//    return color;
//}
//#include "Basic.hlsli"
//
//float4 main(VSOutput input) : SV_TARGET
//{
//    return float4(input.uv, 0, 1);
//}
#include "Basic.hlsli"

Texture2D<float4> tex : register(t0);      // 0�ԃX���b�g�ɐݒ肳�ꂽ�e�N�X�`��
SamplerState smp : register(s0);          // 0�ԃX���b�g�ɐݒ肳�ꂽ�T���v���[


float4 main(VSOutput input) : SV_TARGET{
	/*return float4(tex.Sample(smp, input.uv)) * color;*/
	/*return float4(1,1,1,1) * color;*/

	return float4(input.uv, 0, 1) * color;

}