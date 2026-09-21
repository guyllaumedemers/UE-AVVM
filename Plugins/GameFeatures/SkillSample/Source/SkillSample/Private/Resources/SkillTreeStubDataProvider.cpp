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
#include "Resources/SkillTreeStubDataProvider.h"

#include "AVVMGameplaySettings.h"
#include "AVVMGameplayUtils.h"
#include "DataRegistrySubsystem.h"
#include "GameplayEffect.h"
#include "SkillTreeSettings.h"
#include "SkillTreeUtils.h"
#include "Backend/AVVMOnlineEncodingUtils.h"
#include "Backend/AVVMOnlinePlayer.h"
#include "Backend/AVVMOnlineSkillTree.h"
#include "Data/SkillTreeProviderTableRow.h"
#include "Data/SkillTreeStubDataProviderTableRow.h"

USkillTreeStubDataProvider::USkillTreeStubDataProvider(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (IsTemplate(RF_ClassDefaultObject))
	{
		UAVVMOnlineStubDataHelper::Static_RegisterPropertyProvider(TAG_AVVMONLINE_BACKEND_STUB_SKILLS, GetClass());
	}
}

TArray<int32> USkillTreeStubDataProvider::MakePropertyStubData() const
{
	const auto* Subsystem = UDataRegistrySubsystem::Get();
	if (!IsValid(Subsystem))
	{
		return TArray<int32>{};
	}

	const auto* Row = Subsystem->GetCachedItem<FSkillTreeProviderTableRow>(USkillTreeSettings::GetStubDataProviderSkillTreeId());
	if (!ensureAlwaysMsgf(Row != nullptr, TEXT("Invalid Stub Data Provider.")))
	{
		return TArray<int32>{};
	}

	TArray<int32> PrivateTreeNodeIds{};
	for (const auto& SkillTreeNodeDefinition : Row->SkillTreeNodeDefinitions)
	{
		const int32 RelationshipBitmask = FSkillTreeNodeDefinition::Static_GetRelationshipBitmask(SkillTreeNodeDefinition);
		const int32 PrivateTreeNodeId = USkillTreeUtils::CreateDefaultPrivateTreeNodeId(SkillTreeNodeDefinition.SkillTreeNodeId,
		                                                                                RelationshipBitmask,
		                                                                                SkillTreeNodeDefinition.InstancedId,
		                                                                                SkillTreeNodeDefinition.EffectLevel);

		PrivateTreeNodeIds.Add(PrivateTreeNodeId);
	}

	return PrivateTreeNodeIds;
}

USkillDependencyGraphStubDataProvider::USkillDependencyGraphStubDataProvider(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (IsTemplate(RF_ClassDefaultObject))
	{
		UAVVMOnlineStubDataHelper::Static_RegisterPropertyProvider(TAG_AVVMONLINE_BACKEND_STUB_SKILL_DEPENDENCY_GRAPH, GetClass());
	}
}

TArray<int32> USkillDependencyGraphStubDataProvider::MakePropertyStubData() const
{
	const auto* Subsystem = UDataRegistrySubsystem::Get();
	if (!IsValid(Subsystem))
	{
		return TArray<int32>{};
	}

	const auto* Row = Subsystem->GetCachedItem<FStubData_SkillDependencyGraphTableRow>(USkillTreeSettings::GetStubDataSkillTreeDependencyGraphId());
	if (!ensureAlwaysMsgf(Row != nullptr, TEXT("Invalid Stub Data Provider Complex Lookup.")))
	{
		return TArray<int32>{};
	}

	// @gdemers allow assigning proper instance id to dependent element of the GE.
	TMap<const UGameplayEffect*/*CDO*/, int32/*Counter*/> GameplayEffectInstanceCount{};
	TMap<const AActor*/*CDO*/, int32/*Counter*/> DependentActorInstanceCount{};

	TArray<int32> OutComplexDependencies{};
	for (const auto& [GameplayEffectClass, SkillDependencyGraphElements] : Row->SkillDependencyGraph)
	{
		if (GameplayEffectClass.IsNull())
		{
			continue;
		}

		// TODO @gdemers Improve on this. I dont like that its synchronous.
		const UClass* EffectClass = GameplayEffectClass.LoadSynchronous();
		if (!IsValid(EffectClass))
		{
			continue;
		}

		const auto* EffectCDO = EffectClass->GetDefaultObject<UGameplayEffect>();
		if (!IsValid(EffectCDO))
		{
			continue;
		}

		const int32 EffectPhysicalGlobalId = UAVVMGameplayUtils::GetGameplayEffectUniqueIdentifierByGameplayEffect(EffectCDO);
		for (const auto& SkillDependencyGraphElement : SkillDependencyGraphElements.Dependencies)
		{
			if (SkillDependencyGraphElement.DependentActorClass.IsNull())
			{
				continue;
			}

			// TODO @gdemers Improve on this. I dont like that its synchronous.
			const UClass* ActorClass = SkillDependencyGraphElement.DependentActorClass.LoadSynchronous();
			if (!IsValid(ActorClass))
			{
				continue;
			}

			// @gdemers IMPORTANT - PhysicalGlobalId & VirtualGlobalId are identical in Skill Sample due to flexibility requirements.
			// We want design to be able to reuse gameplay effect on ANY actor they want.
			const auto* ActorCDO = ActorClass->GetDefaultObject<AActor>();
			const int32 DependentVirtualGlobalId = UAVVMGameplayUtils::GetActorUniqueIdentifierByActor(ActorCDO);
			
			const int32 RelationshipBitmask = FStubData_SkillDependencyGraphElement::Static_GetRelationshipBitmask(SkillDependencyGraphElement);
			const int32 EffectVirtualGlobalId = EffectPhysicalGlobalId;

			int32& OutGameplayEffectCount = GameplayEffectInstanceCount.FindOrAdd(EffectCDO);
			++OutGameplayEffectCount;
			
			int32& OutDependentActorCount = DependentActorInstanceCount.FindOrAdd(ActorCDO);
			++OutDependentActorCount;

			// @gdemers IMPORTANT - Both bit encoding are different. virtual address translation
			// is required to generate the proper lookup.
			const int32 DependencyBitmask = (
				RelationshipBitmask +
				UAVVMOnlineEncodingUtils::EncodeInt32(EffectVirtualGlobalId, GET_SKILL_TREE_NODE_LOOKUP_VIRTUAL_GLOBAL_ID_BIT_RANGE, GET_SKILL_TREE_NODE_LOOKUP_VIRTUAL_GLOBAL_ID_RSHIFT) +
				UAVVMOnlineEncodingUtils::EncodeInt32(OutGameplayEffectCount, GET_SKILL_TREE_NODE_LOOKUP_INSTANCED_ID_BIT_RANGE, GET_SKILL_TREE_NODE_LOOKUP_INSTANCED_ID_RSHIFT) +
				UAVVMOnlineEncodingUtils::EncodeInt32(DependentVirtualGlobalId, GET_SKILL_TREE_NODE_LOOKUP_OWNER_VIRTUAL_GLOBAL_ID_BIT_RANGE, GET_SKILL_TREE_NODE_LOOKUP_OWNER_VIRTUAL_GLOBAL_ID_RSHIFT) +
				UAVVMOnlineEncodingUtils::EncodeInt32(OutDependentActorCount, GET_SKILL_TREE_NODE_LOOKUP_OWNER_INSTANCED_ID_BIT_RANGE, GET_SKILL_TREE_NODE_LOOKUP_OWNER_INSTANCED_ID_RSHIFT)
			);

			OutComplexDependencies.Add(DependencyBitmask);
		}
	}

	return OutComplexDependencies;
}
