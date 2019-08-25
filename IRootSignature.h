#pragma once
#include "../FBCommon/IRefCounted.h"
namespace fb {
	FBDeclareIntrusivePointer(IRootSignature);
	class IRootSignature  : public IRefCounted
	{
	};
}
