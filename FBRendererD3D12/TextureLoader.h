#pragma once

struct DDS_PIXELFORMAT
{
	uint32_t    size;
	uint32_t    flags;
	uint32_t    fourCC;
	uint32_t    RGBBitCount;
	uint32_t    RBitMask;
	uint32_t    GBitMask;
	uint32_t    BBitMask;
	uint32_t    ABitMask;
};

struct DDS_HEADER
{
	uint32_t        size;
	uint32_t        flags;
	uint32_t        height;
	uint32_t        width;
	uint32_t        pitchOrLinearSize;
	uint32_t        depth; // only if DDS_HEADER_FLAGS_VOLUME is set in flags
	uint32_t        mipMapCount;
	uint32_t        reserved1[11];
	DDS_PIXELFORMAT ddspf;
	uint32_t        caps;
	uint32_t        caps2;
	uint32_t        caps3;
	uint32_t        caps4;
	uint32_t        reserved2;
};

struct DDS_HEADER_DXT10
{
	DXGI_FORMAT     dxgiFormat;
	uint32_t        resourceDimension;
	uint32_t        miscFlag; // see D3D11_RESOURCE_MISC_FLAG
	uint32_t        arraySize;
	uint32_t        miscFlags2;
};

enum DDS_ALPHA_MODE
{
	DDS_ALPHA_MODE_UNKNOWN = 0,
	DDS_ALPHA_MODE_STRAIGHT = 1,
	DDS_ALPHA_MODE_PREMULTIPLIED = 2,
	DDS_ALPHA_MODE_OPAQUE = 3,
	DDS_ALPHA_MODE_CUSTOM = 4,
};

static HRESULT LoadTextureDataFromFile(_In_z_ const wchar_t* fileName,
	std::unique_ptr<uint8_t[]>& ddsData,
	DDS_HEADER** header,
	uint8_t** bitData,
	size_t* bitSize
);

HRESULT CreateDDSTextureFromFile12(_In_ ID3D12Device* device,
	_In_ ID3D12GraphicsCommandList* cmdList,
	_In_z_ const wchar_t* szFileName,
	_Out_ Microsoft::WRL::ComPtr<ID3D12Resource>& texture,
	_Out_ Microsoft::WRL::ComPtr<ID3D12Resource>& textureUploadHeap,
	_In_ size_t maxsize,
	_Out_opt_ DDS_ALPHA_MODE* alphaMode);