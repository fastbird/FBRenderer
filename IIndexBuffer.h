#pragma once
#include "Types.h"
#include "../FBCommon/IRefCounted.h"
#include "DataFormat.h"
namespace fb
{
	FBDeclareIntrusivePointer(IIndexBuffer);
	class IIndexBuffer : public IRefCounted
	{
	public:
		virtual ~IIndexBuffer() {}
		virtual bool Initialize(const void* indexData, UINT size, EDataFormat format, bool keepData) = 0;
		virtual UINT GetSize() const = 0;
		virtual EDataFormat GetFormat() const = 0;
		virtual UINT GetElementCount() const = 0;
	};
}
