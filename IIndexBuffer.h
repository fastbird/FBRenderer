#pragma once
#include "Types.h"
#include "../FBCommon/IRefCounted.h"
#include "DataFormat.h"
namespace fb
{
	enum class EIndexBufferFormat
	{
		R16,
		R32,
	};
	FBDeclareIntrusivePointer(IIndexBuffer);
	class IIndexBuffer : public IRefCounted
	{
	public:
		virtual ~IIndexBuffer() {}
		virtual bool Initialize(const void* indexData, UINT size, EIndexBufferFormat format, bool keepData) = 0;
		virtual UINT GetSize() const = 0;
		virtual EIndexBufferFormat GetFormat() const = 0;
		virtual UINT GetElementCount() const = 0;
		virtual void Bind() = 0;
	};
}
