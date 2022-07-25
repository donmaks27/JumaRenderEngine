// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "RenderEngine_DirectX12.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX12)

#include <d3d12.h>
#include <dxgi1_6.h>
#if defined(JDEBUG)
#include <dxgidebug.h>
#endif

#include "Material_DirectX12.h"
#include "RenderPipeline_DirectX12.h"
#include "RenderTarget_DirectX12.h"
#include "Shader_DirectX12.h"
#include "Texture_DirectX12.h"
#include "VertexBuffer_DirectX12.h"
#include "DirectX12Objects/DirectX12MipGenerator.h"
#include "renderEngine/window/DirectX12/WindowControllerInfo_DirectX12.h"

namespace JumaRenderEngine
{
    IDXGIAdapter4* GetDXGIAdapter()
    {
        IDXGIFactory4* DXGIFactory = nullptr;
#if defined(JDEBUG)
        constexpr UINT createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#else
        constexpr UINT createFactoryFlags = 0;
#endif
        const HRESULT result = CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&DXGIFactory));
        if (FAILED(result))
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create DXGIFactory2"));
            return nullptr;
        }

        IDXGIAdapter4* resultAdapter = nullptr;
        IDXGIAdapter1* adapter = nullptr;
        UINT adapterIndex = 0;
        SIZE_T maxDedicatedVideoMemory = 0;
        while (DXGIFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
        {
            DXGI_ADAPTER_DESC1 adapterDescription;
            adapter->GetDesc1(&adapterDescription);
            if (((adapterDescription.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0) && (adapterDescription.DedicatedVideoMemory > maxDedicatedVideoMemory))
            {
                if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)))
                {
                    maxDedicatedVideoMemory = adapterDescription.DedicatedVideoMemory;
                    if (resultAdapter != nullptr)
                    {
                        resultAdapter->Release();
                    }
                    adapter->QueryInterface(&resultAdapter);
                }
            }
            adapter->Release();
            adapterIndex++;
        }

        DXGIFactory->Release();
        return resultAdapter;
    }

    RenderEngine_DirectX12::~RenderEngine_DirectX12()
    {
        clearDirectX();
#if defined(JDEBUG)
        {
            IDXGIDebug1* dxgiDebug;
            DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug));
            dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
            dxgiDebug->Release();
        }
#endif
    }

    bool RenderEngine_DirectX12::initInternal(const jmap<window_id, WindowProperties>& windows)
    {
#if defined(JDEBUG)
        {
            ID3D12Debug* debugInterface;
            const HRESULT result = D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface));
            if (FAILED(result))
            {
                JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to get DirectX12 debug interface"));
                return false;
            }
            debugInterface->EnableDebugLayer();
            debugInterface->Release();
        }
#endif

        if (!Super::initInternal(windows))
        {
            return false;
        }
        if (!createDirectXDevice())
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create DirectX12 device"));
            return false;
        }
        if (!createCommandQueues())
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create DirectX12 command queues"));
            return false;
        }
        if (!createOtherObjects())
        {
            return false;
        }
        if (!getWindowController<WindowController_DirectX12>()->createWindowSwapchains())
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create DirectX12 swapchains"));
            return false;
        }
        return true;
    }
    bool RenderEngine_DirectX12::createDirectXDevice()
    {
        IDXGIAdapter4* adapter = GetDXGIAdapter();
        if (adapter == nullptr)
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create DirectX12 device"));
            return false;
        }

        ID3D12Device2* device = nullptr;
        HRESULT result = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
        if (FAILED(result))
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create DirectX12 device"));
            adapter->Release();
            return false;
        }

        D3D12MA::ALLOCATOR_DESC allocatorDescription{};
        allocatorDescription.pAdapter = adapter;
        allocatorDescription.pDevice = device;
        D3D12MA::Allocator* allocator = nullptr;
        result = D3D12MA::CreateAllocator(&allocatorDescription, &allocator);
        adapter->Release();
        if (FAILED(result))
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create D3D12MA resource allocator"));
            device->Release();
            return false;
        }

#if defined(JDEBUG)
        {
            ID3D12InfoQueue* infoQueue = nullptr;
            if (SUCCEEDED(device->QueryInterface(&infoQueue)))
            {
                infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
                infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
                infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, FALSE);

                //static constexpr D3D12_MESSAGE_CATEGORY suppressCategories[] = {};
                static D3D12_MESSAGE_SEVERITY suppressSeverities[] = { D3D12_MESSAGE_SEVERITY_INFO };
                static D3D12_MESSAGE_ID denyMessageIds[] = {
                    D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
                    D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
                    D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE
                };
                D3D12_INFO_QUEUE_FILTER filter{};
                filter.DenyList.NumCategories = 0;
                filter.DenyList.pCategoryList = nullptr;
                filter.DenyList.NumSeverities = 1;
                filter.DenyList.pSeverityList = suppressSeverities;
                filter.DenyList.NumIDs = 3;
                filter.DenyList.pIDList = denyMessageIds;
                result = infoQueue->PushStorageFilter(&filter);
                infoQueue->Release();
                if (FAILED(result))
                {
                    JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to push message filter"));
                    allocator->Release();
                    device->Release();
                    return false;
                }
            }
        }
#endif

        m_Device = device;
        m_ResourceAllocator = allocator;
        return true;
    }
    bool RenderEngine_DirectX12::createCommandQueues()
    {
        if (!registerObject(&m_CommandQueues.add(D3D12_COMMAND_LIST_TYPE_DIRECT))->init(D3D12_COMMAND_LIST_TYPE_DIRECT))
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create DirectX12 direct command queue"));
            return false;
        }
        if (!registerObject(&m_CommandQueues.add(D3D12_COMMAND_LIST_TYPE_COMPUTE))->init(D3D12_COMMAND_LIST_TYPE_COMPUTE))
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create DirectX12 compute command queue"));
            return false;
        }
        if (!registerObject(&m_CommandQueues.add(D3D12_COMMAND_LIST_TYPE_COPY))->init(D3D12_COMMAND_LIST_TYPE_COPY))
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create DirectX12 copy command queue"));
            return false;
        }
        return true;
    }
    bool RenderEngine_DirectX12::createOtherObjects()
    {
        m_CachedDescriptorSize_RTV = static_cast<uint8>(m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
        m_CachedDescriptorSize_DSV = static_cast<uint8>(m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV));
        m_CachedDescriptorSize_SRV = static_cast<uint8>(m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
        m_CachedDescriptorSize_Sampler = static_cast<uint8>(m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER));

        m_SamplersDescriptorHeap = createDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, TextureSamplerTypeCount, false);
        D3D12_CPU_DESCRIPTOR_HANDLE descriptor = m_SamplersDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
        for (uint8 samplerID = 0; samplerID < TextureSamplerTypeCount; samplerID++)
        {
            const TextureSamplerType samplerType = GetTextureSamplerType(samplerID);
            D3D12_SAMPLER_DESC samplerDescription{};
            switch (samplerType.filterType)
            {
            case TextureFilterType::Point:
                samplerDescription.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
                samplerDescription.MaxAnisotropy = 1;
                break;
            case TextureFilterType::Bilinear:
                samplerDescription.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
                samplerDescription.MaxAnisotropy = 1;
                break;
            case TextureFilterType::Trilinear:
                samplerDescription.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
                samplerDescription.MaxAnisotropy = 1;
                break;
            case TextureFilterType::Anisotropic_2:
                samplerDescription.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
                samplerDescription.MaxAnisotropy = 2;
                break;
            case TextureFilterType::Anisotropic_4:
                samplerDescription.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
                samplerDescription.MaxAnisotropy = 4;
                break;
            case TextureFilterType::Anisotropic_8:
                samplerDescription.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
                samplerDescription.MaxAnisotropy = 8;
                break;
            case TextureFilterType::Anisotropic_16:
                samplerDescription.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
                samplerDescription.MaxAnisotropy = 16;
                break;
            default: ;
            }
            switch (samplerType.wrapMode)
            {
            case TextureWrapMode::Repeat: 
                samplerDescription.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
                samplerDescription.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
                samplerDescription.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
                break;
            case TextureWrapMode::Mirror: 
                samplerDescription.AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
                samplerDescription.AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
                samplerDescription.AddressW = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
                break;
            case TextureWrapMode::Clamp: 
                samplerDescription.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
                samplerDescription.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
                samplerDescription.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
                break;
            default: ;
            }
            samplerDescription.MipLODBias = 0.0f;
            samplerDescription.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
            samplerDescription.BorderColor[0] = 1.0f;
            samplerDescription.BorderColor[1] = 1.0f;
            samplerDescription.BorderColor[2] = 1.0f;
            samplerDescription.BorderColor[3] = 1.0f;
            samplerDescription.MinLOD = -FLT_MAX;
            samplerDescription.MaxLOD = FLT_MAX;
            m_Device->CreateSampler(&samplerDescription, descriptor);
            descriptor.ptr += m_CachedDescriptorSize_Sampler;
        }

        m_TextureMipGenerator = createObject<DirectX12MipGenerator>();
        if (!m_TextureMipGenerator->init())
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to initialize texture mip generator"));
            delete m_TextureMipGenerator;
            return false;
        }
        return true;
    }

    void RenderEngine_DirectX12::clearInternal()
    {
        clearDirectX();
        Super::clearInternal();
    }
    void RenderEngine_DirectX12::clearDirectX()
    {
        for (auto& commandQueue : m_CommandQueues)
        {
            commandQueue.value.waitForFinish();
        }

        clearRenderAssets();
        {
            WindowController_DirectX12* windowController = getWindowController<WindowController_DirectX12>();
            if (windowController != nullptr)
            {
                windowController->clearWindowSwapchains();
            }
        }

        m_UnusedBuffers.clear();
        m_Buffers.clear();

        if (m_TextureMipGenerator != nullptr)
        {
            delete m_TextureMipGenerator;
            m_TextureMipGenerator = nullptr;
        }
        if (m_SamplersDescriptorHeap != nullptr)
        {
            m_SamplersDescriptorHeap->Release();
            m_SamplersDescriptorHeap = nullptr;
        }

        m_CommandQueues.clear();
        if (m_ResourceAllocator != nullptr)
        {
            m_ResourceAllocator->Release();
            m_ResourceAllocator = nullptr;
        }
        if (m_Device != nullptr)
        {
            m_Device->Release();
            m_Device = nullptr;
        }
    }

    ID3D12DescriptorHeap* RenderEngine_DirectX12::createDescriptorHeap(const D3D12_DESCRIPTOR_HEAP_TYPE heapType, const uint32 size, 
        const bool shaderVisible) const
    {
        if ((m_Device == nullptr) || (size == 0))
        {
            return nullptr;
        }

        D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDescription{};
        descriptorHeapDescription.Type = heapType;
        descriptorHeapDescription.NumDescriptors = size;
        descriptorHeapDescription.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        descriptorHeapDescription.NodeMask = 0;
        ID3D12DescriptorHeap* descriptorHeap = nullptr;
        const HRESULT result = m_Device->CreateDescriptorHeap(&descriptorHeapDescription, IID_PPV_ARGS(&descriptorHeap));
        if (FAILED(result))
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create DirectX12 descriptor heap"));
            return nullptr;
        }
        return descriptorHeap;
    }

    DirectX12Buffer* RenderEngine_DirectX12::getBuffer()
    {
        if (!m_UnusedBuffers.isEmpty())
        {
            DirectX12Buffer* buffer = m_UnusedBuffers.getLast();
            m_UnusedBuffers.removeLast();
            return buffer;
        }
        return registerObject(&m_Buffers.addDefault());
    }
    void RenderEngine_DirectX12::returnBuffer(DirectX12Buffer* buffer)
    {
        if (buffer != nullptr)
        {
            buffer->clear();
            m_UnusedBuffers.addUnique(buffer);
        }
    }

    WindowController* RenderEngine_DirectX12::createWindowController()
    {
        return registerObject(WindowControllerInfo<RenderAPI::DirectX12>::create());
    }
    VertexBuffer* RenderEngine_DirectX12::createVertexBufferInternal()
    {
        return createObject<VertexBuffer_DirectX12>();
    }
    Texture* RenderEngine_DirectX12::createTextureInternal()
    {
        return createObject<Texture_DirectX12>();
    }
    Shader* RenderEngine_DirectX12::createShaderInternal()
    {
        return createObject<Shader_DirectX12>();
    }
    Material* RenderEngine_DirectX12::createMaterialInternal()
    {
        return createObject<Material_DirectX12>();
    }
    RenderTarget* RenderEngine_DirectX12::createRenderTargetInternal()
    {
        return createObject<RenderTarget_DirectX12>();
    }
    RenderPipeline* RenderEngine_DirectX12::createRenderPipelineInternal()
    {
        return createObject<RenderPipeline_DirectX12>();
    }
}

#endif
