#pragma once
#include "Types.h"
#include "InputElementDesc.h"
#include "../FBCommon/Types.h"
// Pipeline State Object

namespace fb {
	struct FSODeclarationEntry
	{
		UINT Stream;
		const char* SemanticName;
		UINT SemanticIndex;
		BYTE StartComponent;
		BYTE ComponentCount;
		BYTE OutputSlot;
	};

	struct FStreamOutputDesc
	{
		const FSODeclarationEntry* pSODeclaration;
		UINT NumEntries;
		const UINT* pBufferStrides;
		UINT NumStrides;
		UINT RasterizedStream;
	};

	enum class EBlend
	{
		ZERO = 1,
		ONE = 2,
		SRC_COLOR = 3,
		INV_SRC_COLOR = 4,
		SRC_ALPHA = 5,
		INV_SRC_ALPHA = 6,
		DEST_ALPHA = 7,
		INV_DEST_ALPHA = 8,
		DEST_COLOR = 9,
		INV_DEST_COLOR = 10,
		SRC_ALPHA_SAT = 11,
		BLEND_FACTOR = 14,
		INV_BLEND_FACTOR = 15,
		SRC1_COLOR = 16,
		INV_SRC1_COLOR = 17,
		SRC1_ALPHA = 18,
		INV_SRC1_ALPHA = 19
	};

	enum class EBlendOp
	{
		ADD = 1,
		SUBTRACT = 2,
		REV_SUBTRACT = 3,
		MIN = 4,
		MAX = 5
	};

	enum class ELogicOp
	{
		CLEAR = 0,
		SET,
		COPY,
		COPY_INVERTED,
		NOOP,
		INVERT,
		AND,
		NAND,
		OR,
		NOR,
		XOR,
		EQUIV,
		AND_REVERSE,
		AND_INVERTED,
		OR_REVERSE,
		OR_INVERTED
	};

	struct FRenderTargetBlendDesc
	{
		BOOL BlendEnable;
		BOOL LogicOpEnable;
		EBlend SrcBlend;
		EBlend DestBlend;
		EBlendOp BlendOp;
		EBlend SrcBlendAlpha;
		EBlend DestBlendAlpha;
		EBlendOp BlendOpAlpha;
		ELogicOp LogicOp;
		UINT8 RenderTargetWriteMask;
	};

	struct FBlendDesc
	{
		bool AlphaToCoverageEnable;
		bool IndependentBlendEnable;

		FRenderTargetBlendDesc RenderTarget[8];
	};

	enum EFillMode
	{
		WIREFRAME = 2,
		SOLID = 3
	};

	enum class ConservativeRasterizationMode
	{
		OFF = 0,
		ON = 1
	};

	struct FRasterizerDesc
	{
		EFillMode FillMode;
		EFillMode CullMode;
		BOOL FrontCounterClockwise;
		INT DepthBias;
		FLOAT DepthBiasClamp;
		FLOAT SlopeScaledDepthBias;
		BOOL DepthClipEnable;
		BOOL MultisampleEnable;
		BOOL AntialiasedLineEnable;
		UINT ForcedSampleCount;
		ConservativeRasterizationMode ConservativeRaster;
	};

	enum class EDepthWriteMask
	{
		ZERO = 0,
		ALL = 1
	};

	enum class EComparisonFunc
	{
		NEVER = 1,
		LESS = 2,
		EQUAL = 3,
		LESS_EQUAL = 4,
		GREATER = 5,
		NOT_EQUAL = 6,
		GREATER_EQUAL = 7,
		ALWAYS = 8
	};

	enum class EStencilOp
	{
		KEEP = 1,
		ZERO = 2,
		REPLACE = 3,
		INCR_SAT = 4,
		DECR_SAT = 5,
		INVERT = 6,
		INCR = 7,
		DECR = 8
	};

	struct FDepthStencilOpDesc
	{
		EStencilOp StencilFailOp;
		EStencilOp StencilDepthFailOp;
		EStencilOp StencilPassOp;
		EComparisonFunc StencilFunc;
	};

	struct FDepthStencilDesc
	{
		BOOL DepthEnable;
		EDepthWriteMask DepthWriteMask;
		EComparisonFunc DepthFunc;
		BOOL StencilEnable;
		UINT8 StencilReadMask;
		UINT8 StencilWriteMask;
		FDepthStencilOpDesc FrontFace;
		FDepthStencilOpDesc BackFace;
	};

	enum class EIndexBufferStripCutValue
	{
		CUT_VALUE_DISABLED = 0,
		CUT_VALUE_0xFFFF = 1,
		CUT_VALUE_0xFFFFFFFF = 2
	};

	enum class EPrimitiveTopologyType
	{
		UNDEFINED = 0,
		POINT = 1,
		LINE = 2,
		TRIANGLE = 3,
		PATCH = 4
	};

	struct FSampleDesc
	{
		UINT Count;
		UINT Quality;
	};

	enum class EPipelineStateFlags
	{
		NONE = 0,
		TOOL_DEBUG = 0x1
	};

	struct PSODesc
	{
		void* pRootSignature;
		ByteArray VS;
		ByteArray PS;
		ByteArray DS;
		ByteArray HS;
		ByteArray GS;
		FStreamOutputDesc StreamOutput;
		FBlendDesc BlendState;
		UINT SampleMask;
		FRasterizerDesc RasterizerState;
		FDepthStencilDesc DepthStencilState;
		FInputLayoutDesc InputLayout;
		EIndexBufferStripCutValue IBStripCutValue;
		EPrimitiveTopologyType PrimitiveTopologyType;
		UINT NumRenderTargets;
		EDataFormat RTVFormats[8];
		EDataFormat DSVFormat;
		FSampleDesc SampleDesc;
		UINT NodeMask;
		ByteArray CachedPSO;
		EPipelineStateFlags Flags;
	};
}
