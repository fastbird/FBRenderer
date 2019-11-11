#pragma once
#include "../FBCommon/IRefCounted.h"
namespace fb {

	static const char* RootDescriptorTableName = "DTable";
	static const char* RootDescriptorCBVName = "RootCBV";
	static const char* RootDescriptorSRVName = "RootSRV";
	static const char* RootDescriptorUAVName = "RootUAV";
	static const char* DTableRangeTypeCBV = "CBV";
	static const char* DTableRangeTypeSRV = "SRV";
	static const char* DTableRangeTypeUAV = "UAV";
	static const char* RootConstantName = "RootConstant";
	FBDeclareIntrusivePointer(IRootSignature);
	class IRootSignature  : public IRefCounted
	{
	public:
		virtual void Bind() = 0;
	};
}
