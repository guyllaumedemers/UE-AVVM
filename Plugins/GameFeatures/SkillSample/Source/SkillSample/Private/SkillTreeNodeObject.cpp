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
#include "SkillTreeNodeObject.h"

#include "AbilitySystemComponent.h"
#include "AVVMGameplayUtils.h"
#include "AVVMSaveGame.h"
#include "AVVMToolkitUtils.h"
#include "SkillTreeUtils.h"
#include "Ability/AVVMAbilityDefinitionDataAsset.h"
#include "Backend/AVVMOnlineBackendUtils.h"
#include "Backend/AVVMOnlineEncodingUtils.h"
#include "Backend/AVVMOnlineSkillTree.h"
#include "Engine/AssetManager.h"
#include "Resources/AVVMResourceProvider.h"

// @gdemers external linkage for property FName sharing.
SKILLSAMPLE_API const FName SkillTreeProviderPayloads = TEXT("SkillTreeProviderPayloads");

FSkillTreeNodeObject::FSkillTreeNodeObject(const int32 NewPrivateTreeNodeId,
                                           const int32 NewActiveGameplayEffectHandleTypeHash)
	: ActiveGameplayEffectHandleTypeHash(NewActiveGameplayEffectHandleTypeHash),
	  PrivateTreeNodeId(NewPrivateTreeNodeId)
{
}

const int32 FSkillTreeNodeObject::GetActiveEffectHandleTypeHash() const
{
	return ActiveGameplayEffectHandleTypeHash;
}

const int32 FSkillTreeNodeObject::GetSkillTreeNodePrivateId() const
{
	return PrivateTreeNodeId;
}

int32 USkillTreeNodeObjectUtils::RuntimeInitStaticItem(const UObject* Outer,
                                                       const TArray<int32>& NewPrivateIds,
                                                       const UGameplayEffect* SkillTreeNodeEffectCDO)
{
	const bool bResult = UAVVMToolkitUtils::IsBlueprintScriptInterfaceValid<UAVVMResourceProvider>(Outer);
	if (!bResult)
	{
		return INDEX_NONE;
	}

	const int32 TargetUniqueId = IAVVMResourceProvider::Execute_GetProviderUniqueId(Outer);
	if (!ensureAlwaysMsgf(TargetUniqueId != INDEX_NONE,
	                      TEXT("Actor \"%s\" isn't referencing a valid UniqueId based on IAVVMResourceProvider::GetProviderUniqueId implementation."),
	                      *GetNameSafe(Outer)))
	{
		return INDEX_NONE;
	}

	const int32 PhysicalGlobalId = UAVVMGameplayUtils::GetGameplayEffectUniqueIdentifierByGameplayEffect(SkillTreeNodeEffectCDO);
	if (!ensureAlwaysMsgf(PhysicalGlobalId != INDEX_NONE,
	                      TEXT("Couldn't retrieve a valid TreeNodeId. Are you missing a valid FDataRegistryId reference within this Object Class definition ?")))
	{
		return INDEX_NONE;
	}

	static const auto GenerateDefaultContent = []()
	{
		return USkillTreeUtils::CreateDefaultSkillTreeProviders();
	};

	// @gdemers get-set file from disk caching all skill tree providers representation.
	const FStringView FileContent = UAVVMSaveGame::Static_GetSetFileContent(SkillTreeProviderPayloads, GenerateDefaultContent);

	// @gdemers fetch provider payload from disk representation.
	const FString SkillTreeProviderPayload = USkillTreeUtils::GetSkillTreeProviderById(FileContent.GetData(), TargetUniqueId);
	if (SkillTreeProviderPayload.IsEmpty())
	{
		return INDEX_NONE;
	}

	// @gdemers read private tree node id from payload.
	const int32 PrivateItemId = USkillTreeUtils::GetSkillTreeNodePrivateId(SkillTreeProviderPayload, NewPrivateIds, PhysicalGlobalId);
	return PrivateItemId;
}

int32 USkillTreeNodeObjectUtils::RuntimeInitOnlineItem(const UObject* Outer,
                                                       const TArray<int32>& NewPrivateIds,
                                                       const TInstancedStruct<FAVVMDataResolverHelper>& DataResolverHelper,
                                                       const UGameplayEffect* SkillTreeNodeEffectCDO)
{
	const bool bResult = UAVVMToolkitUtils::IsBlueprintScriptInterfaceValid<UAVVMResourceProvider>(Outer);
	if (!bResult)
	{
		return INDEX_NONE;
	}

	// @gdemers target any actor type referencing this inventory component that may have a backend representation.
	const int32 TargetUniqueId = IAVVMResourceProvider::Execute_GetProviderUniqueId(Outer);
	if (!ensureAlwaysMsgf(TargetUniqueId != INDEX_NONE,
	                      TEXT("Actor \"%s\" isn't referencing a valid UniqueId based on IAVVMResourceProvider::GetProviderUniqueId implementation."),
	                      *GetNameSafe(Outer)))
	{
		return INDEX_NONE;
	}

	const TArray<int32> OuterDependencies = UAVVMOnlineBackendUtils::GetElementDependencies(Outer, TargetUniqueId, DataResolverHelper);
	const int32 PhysicalGlobalId = UAVVMGameplayUtils::GetGameplayEffectUniqueIdentifierByGameplayEffect(SkillTreeNodeEffectCDO);

	if (OuterDependencies.IsEmpty() || !ensureAlwaysMsgf(PhysicalGlobalId != INDEX_NONE,
	                                                     TEXT("Couldn't retrieve a valid TreeNodeId. Are you missing a valid FDataRegistryId reference within this Object Class definition ?")))
	{
		return INDEX_NONE;
	}

	// @gdemers filter the backend set to ensure we dont reallocate an item that was already configured.
	TArray<int32> FilteredSet = OuterDependencies;
	for (const int32 ReservedItemId : NewPrivateIds)
	{
		FilteredSet.Remove(ReservedItemId);
	}

	const int32* SearchResult = FilteredSet.FindByPredicate([SearchId = PhysicalGlobalId](const int32 Value)
	{
		// @gdemers filter the PrivateItemId that represent our complex encoding, and translate the virtual id parsed
		// from the integer into a physical id for comparison.
		const int32 OutPhysicalGlobalId = USkillTreeNodeObjectUtils::FilterTreeNodePrivateId(Value);
		return (false == (OutPhysicalGlobalId ^ SearchId))/*if both bits are identical, return 0.*/;
	});

	if (ensureAlwaysMsgf(SearchResult != nullptr, TEXT("Couldn't retrieve the TreeNodeId.")))
	{
		// @gdemers your backend private id that represent the allocated USkillTreeNodeObject.
		const int32 PrivateItemId = (*SearchResult);
		return PrivateItemId;
	}
	else
	{
		return INDEX_NONE;
	}
}

int32 USkillTreeNodeObjectUtils::FilterTreeNodePrivateId(const int32 EncodedBits/*PrivateTreeNodeId*/)
{
	constexpr int32 BitRange = GET_SKILL_TREE_NODE_VIRTUAL_GLOBAL_ID_BIT_RANGE;
	constexpr int32 BitShift = GET_SKILL_TREE_NODE_VIRTUAL_GLOBAL_ID_RSHIFT;
	int32 PhysicalOffset = 0;

	// @gdemers translate the virtual id stored in the encoded bits into globally defined physical id
	const int32 BaseId = UAVVMOnlineEncodingUtils::DecodeInt32(EncodedBits, BitRange, BitShift);
	return (BaseId + PhysicalOffset);
}

FGameplayTag USkillTreeNodeObjectUtils::GetPrivateIdBlockingTag(const int32 EncodedBits)
{
	return FGameplayTag::EmptyTag;
}
