//float4 main() : SV_TARGET
//{
//    return float4(0.5f, 0.55f, 0.5f, 1.0f);
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
     /*return float4(tex.Sample(smp, input.uv)) color; /*/
     float4 texcolor = float4(tex.Sample(smp, input.uv));
    /*return float4(input.normal, 1);*///RGB�����ꂼ��@����XYZ,A��1�ŏo��
    // return float4(1,1,1,1); */
    float3 light = normalize(float3(1,-1,1)); //�E�����@�����̃��C�g
    float difuse = saturate(dot(-light, input.normal)); //�����ւ̃x�N�g���Ɩ@���x�N�g���̓���
    float brightness = difuse + 0.3f;            //�A�h�r�G���g����0.3�Ƃ��Čv�Z
    return float4(texcolor.rgb * brightness, texcolor.a);// *color;//�P�x��RGB�ɑ�����ďo��
}