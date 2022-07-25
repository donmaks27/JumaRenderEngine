// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX12)

#include <d3d12.h>

namespace JumaRenderEngine
{
    template <typename InnerStructType, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE Type>
    class alignas(void*) DirectX12_PipelineStateStreamSubobject
    {
    public:
        DirectX12_PipelineStateStreamSubobject() : type(Type), data() {}
        DirectX12_PipelineStateStreamSubobject(InnerStructType const& i) : type(Type), data(i) {}

        DirectX12_PipelineStateStreamSubobject& operator=(InnerStructType const& i) { data = i; return *this; }
        operator InnerStructType() const { return data; }
        operator InnerStructType&() { return data; }

    private:
        D3D12_PIPELINE_STATE_SUBOBJECT_TYPE type;
    public:
        InnerStructType data;
    };
    using DirectX12_PipelineStateStream_FLAGS                 = DirectX12_PipelineStateStreamSubobject<D3D12_PIPELINE_STATE_FLAGS,         D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_FLAGS>;
    using DirectX12_PipelineStateStream_NODE_MASK             = DirectX12_PipelineStateStreamSubobject<UINT,                               D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_NODE_MASK>;
    using DirectX12_PipelineStateStream_ROOT_SIGNATURE        = DirectX12_PipelineStateStreamSubobject<ID3D12RootSignature*,               D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE>;
    using DirectX12_PipelineStateStream_INPUT_LAYOUT          = DirectX12_PipelineStateStreamSubobject<D3D12_INPUT_LAYOUT_DESC,            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_INPUT_LAYOUT>;
    using DirectX12_PipelineStateStream_IB_STRIP_CUT_VALUE    = DirectX12_PipelineStateStreamSubobject<D3D12_INDEX_BUFFER_STRIP_CUT_VALUE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_IB_STRIP_CUT_VALUE>;
    using DirectX12_PipelineStateStream_PRIMITIVE_TOPOLOGY    = DirectX12_PipelineStateStreamSubobject<D3D12_PRIMITIVE_TOPOLOGY_TYPE,      D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PRIMITIVE_TOPOLOGY>;
    using DirectX12_PipelineStateStream_VS                    = DirectX12_PipelineStateStreamSubobject<D3D12_SHADER_BYTECODE,              D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VS>;
    using DirectX12_PipelineStateStream_GS                    = DirectX12_PipelineStateStreamSubobject<D3D12_SHADER_BYTECODE,              D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_GS>;
    using DirectX12_PipelineStateStream_STREAM_OUTPUT         = DirectX12_PipelineStateStreamSubobject<D3D12_STREAM_OUTPUT_DESC,           D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_STREAM_OUTPUT>;
    using DirectX12_PipelineStateStream_HS                    = DirectX12_PipelineStateStreamSubobject<D3D12_SHADER_BYTECODE,              D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_HS>;
    using DirectX12_PipelineStateStream_DS                    = DirectX12_PipelineStateStreamSubobject<D3D12_SHADER_BYTECODE,              D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DS>;
    using DirectX12_PipelineStateStream_PS                    = DirectX12_PipelineStateStreamSubobject<D3D12_SHADER_BYTECODE,              D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PS>;
    using DirectX12_PipelineStateStream_CS                    = DirectX12_PipelineStateStreamSubobject<D3D12_SHADER_BYTECODE,              D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_CS>;
    using DirectX12_PipelineStateStream_BLEND                 = DirectX12_PipelineStateStreamSubobject<D3D12_BLEND_DESC,                   D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_BLEND>;
    using DirectX12_PipelineStateStream_DEPTH_STENCIL         = DirectX12_PipelineStateStreamSubobject<D3D12_DEPTH_STENCIL_DESC,           D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL>;
    using DirectX12_PipelineStateStream_DEPTH_STENCIL1        = DirectX12_PipelineStateStreamSubobject<D3D12_DEPTH_STENCIL_DESC1,          D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL1>;
    using DirectX12_PipelineStateStream_DEPTH_STENCIL_FORMAT  = DirectX12_PipelineStateStreamSubobject<DXGI_FORMAT,                        D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL_FORMAT>;
    using DirectX12_PipelineStateStream_RASTERIZER            = DirectX12_PipelineStateStreamSubobject<D3D12_RASTERIZER_DESC,              D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RASTERIZER>;
    using DirectX12_PipelineStateStream_RENDER_TARGET_FORMATS = DirectX12_PipelineStateStreamSubobject<D3D12_RT_FORMAT_ARRAY,              D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RENDER_TARGET_FORMATS>;
    using DirectX12_PipelineStateStream_SAMPLE_DESC           = DirectX12_PipelineStateStreamSubobject<DXGI_SAMPLE_DESC,                   D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_DESC>;
    using DirectX12_PipelineStateStream_SAMPLE_MASK           = DirectX12_PipelineStateStreamSubobject<UINT,                               D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_MASK>;
    using DirectX12_PipelineStateStream_CACHED_PSO            = DirectX12_PipelineStateStreamSubobject<D3D12_CACHED_PIPELINE_STATE,        D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_CACHED_PSO>;
    using DirectX12_PipelineStateStream_VIEW_INSTANCING       = DirectX12_PipelineStateStreamSubobject<D3D12_VIEW_INSTANCING_DESC,         D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VIEW_INSTANCING>;
}

#endif
