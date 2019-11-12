#pragma once
#include "../FBCommon/IRefCounted.h"
namespace fb
{
	typedef struct _SHADER_MACRO
	{
		const char* Name;
		const char* Definition;
	} 	FShaderMacro;

	enum class EShaderType
	{
		PixelShader,
		VertexShader,
		GeometryShader,
		HullShader,
		DomainShader,
		ComputeShader
	};

	FBDeclareIntrusivePointer(IShader);
	class IShader : public IRefCounted
	{
		EShaderType ShaderType;
	public:
		virtual BYTE* GetByteCode() = 0;
		virtual UINT Size() = 0;

	};
}
