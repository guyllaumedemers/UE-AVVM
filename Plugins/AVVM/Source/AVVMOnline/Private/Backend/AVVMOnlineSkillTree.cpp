//Copyright(c) 2025 gdemers
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files(the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions :
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.
#include "Backend/AVVMOnlineSkillTree.h"

#include "Backend/AVVMOnlineEncodingUtils.h"

int32 UAVVMOnlineSkillTreeUtils::GetPhysicalGlobalId(const int32 EncodedBits)
{
	constexpr int32 BitRange = GET_SKILL_TREE_NODE_VIRTUAL_GLOBAL_ID_BIT_RANGE;
	constexpr int32 BitShift = GET_SKILL_TREE_NODE_VIRTUAL_GLOBAL_ID_RSHIFT;
	int32 PhysicalOffset = 0;

	const int32 OutRelationshipBitmask = UAVVMOnlineEncodingUtils::DecodeInt32(EncodedBits, GET_SKILL_TREE_NODE_RELATIONSHIP_BIT_RANGE, GET_SKILL_TREE_NODE_RELATIONSHIP_RSHIFT);
	if ((OutRelationshipBitmask & (1 << 0/*attachment bit-index*/)))
	{
		PhysicalOffset = GET_ATTACHMENT_DEPENDENT_PHYSICAL_ADDRESSING_OFFSET;
	}
	else if ((OutRelationshipBitmask & (1 << 1/*character bit-index*/)))
	{
		PhysicalOffset = GET_CHARACTER_DEPENDENT_PHYSICAL_ADDRESSING_OFFSET;
	}
	else if ((OutRelationshipBitmask & (1 << 2/*item bit-index*/)))
	{
		PhysicalOffset = GET_ITEM_DEPENENT_PHYSICAL_ADDRESSING_OFFSET;
	}

	// @gdemers translate the virtual id stored in the encoded bits into globally defined physical id
	const int32 BaseId = UAVVMOnlineEncodingUtils::DecodeInt32(EncodedBits, BitRange, BitShift);
	return (BaseId + PhysicalOffset);
}

int32 UAVVMOnlineSkillTreeUtils::TranslatePhysicalAddressing(const int32 RelationshipBitMask,
                                                             const int32 PhysicalGlobalId)
{
	constexpr int32 BitRange = GET_SKILL_TREE_NODE_VIRTUAL_GLOBAL_ID_BIT_RANGE;
	constexpr int32 BitShift = GET_SKILL_TREE_NODE_VIRTUAL_GLOBAL_ID_RSHIFT;
	int32 VirtualId = INT32_MAX;

	if ((RelationshipBitMask & (1 << 0/*attachment bit-index*/)))
	{
		VirtualId = (PhysicalGlobalId & ~GET_ATTACHMENT_DEPENDENT_PHYSICAL_ADDRESSING_OFFSET);
	}
	else if ((RelationshipBitMask & (1 << 1/*character bit-index*/)))
	{
		VirtualId = (PhysicalGlobalId & ~GET_CHARACTER_DEPENDENT_PHYSICAL_ADDRESSING_OFFSET);
	}
	else if ((RelationshipBitMask & (1 << 2/*item bit-index*/)))
	{
		VirtualId = (PhysicalGlobalId & ~GET_ITEM_DEPENENT_PHYSICAL_ADDRESSING_OFFSET);
	}

	return UAVVMOnlineEncodingUtils::EncodeInt32(VirtualId, BitRange, BitShift);
}
