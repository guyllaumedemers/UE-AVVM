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
#include "Backend/AVVMOnlineBackendUtils.h"
#include "Backend/AVVMOnlineEncodingUtils.h"
#include "Backend/AVVMOnlineSkillTree.h"
#include "Engine/AssetManager.h"
#include "Resources/AVVMResourceProvider.h"

// @gdemers external linkage for property FName sharing.
extern const FName SkillTreeProviderPayloads;

FSkillTreeNodeObject::FSkillTreeNodeObject(const int32 NewPrivateTreeNodeId,
                                           const FGameplayEffectSpec& NewGameplayEffectSpec)
	: GameplayEffectSpec(NewGameplayEffectSpec),
	  PrivateTreeNodeId(NewPrivateTreeNodeId)
{
}

const FGameplayEffectSpec& FSkillTreeNodeObject::GetGameplayEffectSpec() const
{
	return GameplayEffectSpec;
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

	// @gdemers filter the backend set to ensure we dont reallocate an item that was already configured.
	TArray<int32> FilteredSet = UAVVMOnlineBackendUtils::GetElementDependencies(Outer, TargetUniqueId, DataResolverHelper);
	for (const int32 ReservedItemId : NewPrivateIds)
	{
		FilteredSet.Remove(ReservedItemId);
	}

	const int32* SearchResult = FilteredSet.FindByPredicate([SearchId = PhysicalGlobalId](const int32 Value)
	{
		// @gdemers IMPORTANT - PhysicalGlobalId & VirtualGlobalId are identical in Skill Sample due to flexibility requirements.
		// We want design to be able to reuse gameplay effect on ANY actor they want.
		const int32 OutVirtualGlobalId = UAVVMOnlineEncodingUtils::DecodeInt32(Value, GET_SKILL_TREE_NODE_VIRTUAL_GLOBAL_ID_BIT_RANGE, GET_SKILL_TREE_NODE_VIRTUAL_GLOBAL_ID_RSHIFT);
		return (false == (OutVirtualGlobalId ^ SearchId))/*if both bits are identical, return 0.*/;
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

FGameplayTag USkillTreeNodeObjectUtils::GetPrivateIdBlockingTag(const int32 EncodedBits)
{
	return FGameplayTag::EmptyTag;
}
