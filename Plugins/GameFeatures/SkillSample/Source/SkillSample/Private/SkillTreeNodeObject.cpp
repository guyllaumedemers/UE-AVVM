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

#include "Ability/AVVMAbilityDefinitionDataAsset.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

const FDataRegistryId& USkillTreeNodeObject::BP_GetSkillTreeEffectId() const
{
	return GetSkillTreeEffectId();
}

const FDataRegistryId& USkillTreeNodeObject::BP_GetSkillTreeEffectUIId() const
{
	return GetSkillTreeEffectUIId();
}

void USkillTreeNodeObject::SetActiveGameplayEffectHandle(const FActiveGameplayEffectHandle& NewActiveGameplayEffectHandle)
{
	ActiveGameplayEffectHandle = NewActiveGameplayEffectHandle;
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
