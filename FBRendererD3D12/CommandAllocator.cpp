#include "pch.h"
#include "CommandAllocator.h"
#include "Util.h"
using namespace fb;

void CommandAllocator::Reset()
{
	ThrowIfFailed(CommandAllocator->Reset());
}