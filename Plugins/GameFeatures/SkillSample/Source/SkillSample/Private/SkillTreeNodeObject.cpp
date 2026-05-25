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
#include "Ability/AVVMAbilityDefinitionDataAsset.h"
#include "Backend/AVVMOnlineEncodingUtils.h"
#include "Backend/AVVMOnlineInventory.h"
#include "Backend/AVVMOnlineSkillTree.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

void USkillTreeNodeObject::ModifyRuntimeState(const FGameplayTagContainer& AddedTags, const FGameplayTagContainer& RemovedTags)
{
	auto* ASC = ActiveGameplayEffectHandle.GetOwningAbilitySystemComponent();
	if (!IsValid(ASC))
	{
		return;
	}

	if (!AddedTags.IsEmpty())
	{
		ASC->AddLooseGameplayTags(AddedTags);
	}

	if (!RemovedTags.IsEmpty())
	{
		ASC->RemoveLooseGameplayTags(AddedTags);
	}
}

const FDataRegistryId& USkillTreeNodeObject::BP_GetSkillTreeEffectId() const
{
	return GetSkillTreeEffectId();
}

const FDataRegistryId& USkillTreeNodeObject::BP_GetSkillTreeEffectUIId() const
{
	return GetSkillTreeEffectUIId();
}

void USkillTreeNodeObject::GetGameplayEffectClassAsync(const UObject* NewGameplayEffectDefinitionDataAsset,
                                                       const FOnRequestGameplayEffectClassComplete& Callback)
{
	const auto* GameplayEffectDefinitionDataAsset = Cast<UAVVMAbilityDefinitionDataAsset>(NewGameplayEffectDefinitionDataAsset);
	if (!IsValid(GameplayEffectDefinitionDataAsset))
	{
		return;
	}

	FStreamableDelegate OnRequestItemActorClassComplete;
	OnRequestItemActorClassComplete.BindUObject(this, &USkillTreeNodeObject::OnGameplayEffectClassAcquired, Callback);
	StreamingHandle = UAssetManager::Get().LoadAssetList({GameplayEffectDefinitionDataAsset->GetGameplayEffectClass().ToSoftObjectPath()}, OnRequestItemActorClassComplete);
}

void USkillTreeNodeObject::SetActiveGameplayEffectHandle(const FActiveGameplayEffectHandle& NewActiveGameplayEffectHandle)
{
	ActiveGameplayEffectHandle = NewActiveGameplayEffectHandle;
}

void USkillTreeNodeObject::OnGameplayEffectClassAcquired(FOnRequestGameplayEffectClassComplete Callback)
{
	if (!StreamingHandle.IsValid())
	{
		Callback.ExecuteIfBound(nullptr, this);
		return;
	}

	TArray<UObject*> OutStreamableAssets;
	StreamingHandle->GetLoadedAssets(OutStreamableAssets);

	if (!OutStreamableAssets.IsEmpty())
	{
		Callback.ExecuteIfBound(Cast<UClass>(OutStreamableAssets[0]), this);
	}
	else
	{
		Callback.ExecuteIfBound(nullptr, this);
	}
}

int32 USkillTreeNodeObjectUtils::RuntimeInitStaticItem(const UObject* Outer,
                                                       const TArray<int32>& NewPrivateIds,
                                                       USkillTreeNodeObject* UnInitializedSkillTreeNodeObject)
{
	return INDEX_NONE;
}

int32 USkillTreeNodeObjectUtils::RuntimeInitOnlineItem(const UObject* Outer,
                                                       const TArray<int32>& NewPrivateIds,
                                                       const TInstancedStruct<FAVVMDataResolverHelper>& DataResolverHelper,
                                                       USkillTreeNodeObject* UnInitializedSkillTreeNodeObject)
{
	return INDEX_NONE;
}

int32 USkillTreeNodeObjectUtils::GetPrivateTreeNodeId(const USkillTreeNodeObject* SkillTreeNodeObject)
{
	return IsValid(SkillTreeNodeObject) ? SkillTreeNodeObject->PrivateTreeNodeId : INDEX_NONE;
}

int32 USkillTreeNodeObjectUtils::GetFilterTreeNode(const USkillTreeNodeObject* SkillTreeNodeObject)
{
	if (IsValid(SkillTreeNodeObject))
	{
		// @gdemers return a non-shifted version of the TreeNodeId.
		// Keep in mind that the range of the bit encoding define the item category manipulated!
		return USkillTreeNodeObjectUtils::FilterTreeNodePrivateId(SkillTreeNodeObject->PrivateTreeNodeId);
	}
	else
	{
		return INDEX_NONE;
	}
}

int32 USkillTreeNodeObjectUtils::FilterTreeNodePrivateId(const int32 EncodedBits)
{
	// TODO @gdemers Comeback to this later.
	int32 BitRange = INDEX_NONE;
	int32 BitShift = INDEX_NONE;

	if (USkillTreeNodeObjectUtils::IsAttachment(EncodedBits))
	{
		BitRange = GET_ATTACHMENT_ID_ENCODING_BIT_RANGE;
		BitShift = GET_ATTACHMENT_ID_ENCODING_RSHIFT;
	}
	else if (USkillTreeNodeObjectUtils::IsWeapon(EncodedBits))
	{
		BitRange = GET_ITEM_ID_ENCODING_BIT_RANGE;
		BitShift = GET_ITEM_ID_ENCODING_RSHIFT;
	}
	else
	{
		BitRange = GET_ITEM_ID_ENCODING_BIT_RANGE;
		BitShift = GET_ITEM_ID_ENCODING_RSHIFT;
	}

	const int32 NonShiftedTreeNodeId = UAVVMOnlineEncodingUtils::FilterInt32(EncodedBits, BitRange, BitShift);
	return NonShiftedTreeNodeId;
}

bool USkillTreeNodeObjectUtils::IsAttachment(const int32 EncodedBits)
{
	return !!(EncodedBits & CHECK_ATTACHMENT_DEPENDENT_ENCODING);
}

bool USkillTreeNodeObjectUtils::IsWeapon(const int32 EncodedBits)
{
	return !!(EncodedBits & CHECK_WEAPON_DEPENDENT_ENCODING);
}
