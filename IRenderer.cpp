#include  "IRenderer.h"
#include "../FBCommon/Utility.h"

using namespace fb;

UINT IRenderer::CalcConstantBufferByteSize(UINT beforeAligned) const
{
	return fb::CalcAligned(beforeAligned, 256);
}