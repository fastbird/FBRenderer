#pragma once
#include "../FBCommon/Types.h"
namespace fb
{
	FBDeclareIntrusivePointer(ICommandAllocator);
	class ICommandAllocator : public IRefCounted
	{
	public:
		virtual void Reset() = 0;
	};
}
