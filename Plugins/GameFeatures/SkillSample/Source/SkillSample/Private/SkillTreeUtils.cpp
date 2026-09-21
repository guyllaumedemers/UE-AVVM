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
#include "SkillTreeUtils.h"

#include "AVVMGameplaySettings.h"
#include "AVVMGameplayUtils.h"
#include "AVVMGameSession.h"
#include "AVVMSaveGame.h"
#include "AVVMToolkitUtils.h"
#include "DataRegistrySubsystem.h"
#include "GameplayEffect.h"
#include "SkillTreeProvider.h"
#include "SkillTreeSettings.h"
#include "Backend/AVVMOnlineEncodingUtils.h"
#include "Backend/AVVMOnlineSkillTree.h"
#include "Data/AVVMGameplayEffectIdentifierDataTableRow.h"
#include "Data/SkillTreeProviderTableRow.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "GameFramework/Actor.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

/**
 *	Class description:
 *	
 *	NSJsonSkillTree is a private namespace for class that should be hidden from project.
 */
namespace NSJsonSkillTree
{
	struct FJsonSkillTreeProvider
	{
		int32 Id{INDEX_NONE};
		// @gdemers IMPORTANT we only care about tracking the current set of tree node effects
		// applied at the given phase.
		TArray<int32> PrivateTreeNodeIds{};
	};

	void ToString(const FJsonSkillTreeProvider& NewSkillTreeProvider,
	              FString& OutFormat)
	{
		TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
		JsonData->SetNumberField(TEXT("Id"), NewSkillTreeProvider.Id);

		TArray<TSharedPtr<FJsonValue>> PrivateTreeNodeIds{};
		for (const int32 PrivateTreeNodeId : NewSkillTreeProvider.PrivateTreeNodeIds)
		{
			PrivateTreeNodeIds.Add(MakeShareable(new FJsonValueNumber(PrivateTreeNodeId)));
		}

		JsonData->SetArrayField(TEXT("PrivateTreeNodeIds"), MoveTemp(PrivateTreeNodeIds));

		FString JsonOutput{};
		auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
		if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
		{
			return;
		}

		OutFormat = MoveTemp(JsonOutput);
	}

	void FromString(const FString& NewPayload,
	                FJsonSkillTreeProvider& OutSkillTreeProvider)
	{
		if (NewPayload.IsEmpty())
		{
			return;
		}

		TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

		auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
		if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
		{
			return;
		}

		FJsonSkillTreeProvider SkillTreeProvider{};
		SkillTreeProvider.Id = JsonData->GetIntegerField(TEXT("Id"));
		for (const auto& PrivateTreeNodeId : JsonData->GetArrayField(TEXT("PrivateTreeNodeIds")))
		{
			SkillTreeProvider.PrivateTreeNodeIds.Add(PrivateTreeNodeId->AsNumber());
		}

		OutSkillTreeProvider = MoveTemp(SkillTreeProvider);
	}
}

// @gdemers external linkage for property FName sharing.
extern const FName SkillTreeProviderPayloads;

FString USkillTreeUtils::CreateDefaultSkillTreeProviders()
{
	const auto* Subsystem = UDataRegistrySubsystem::Get();
	if (!IsValid(Subsystem))
	{
		return FString{};
	}

	const UDataRegistry* DataRegistry = Subsystem->GetRegistryForType(USkillTreeSettings::GetSkillTreeProviderRegistryType());
	if (!IsValid(DataRegistry))
	{
		return FString{};
	}

	TArray<const FSkillTreeProviderTableRow*> OutRows{};
	DataRegistry->GetAllItems<FSkillTreeProviderTableRow>(TEXT(""), OutRows);

	TArray<TSharedPtr<FJsonValue>> OutModifiedPayloads{};
	for (const FSkillTreeProviderTableRow* Row : OutRows)
	{
		if (!ensureAlwaysMsgf(Row != nullptr, TEXT("Invalid Row entry.")))
		{
			continue;
		}

		const int32 ProviderId = UAVVMGameplayUtils::GetActorUniqueIdentifierByRegistryId(Row->SkillTreeProviderActorIdentifierId);
		if (!ensureAlwaysMsgf(ProviderId != INDEX_NONE,
		                      TEXT("Missing valid Id for Provider entry.")))
		{
			continue;
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

		FString OutProvider = USkillTreeUtils::CreateSkillTreeProvider(ProviderId, PrivateTreeNodeIds);
		OutModifiedPayloads.Add(MakeShareable(new FJsonValueString(MoveTemp(OutProvider))));
	}

	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetArrayField(TEXT("SkillTreeProviders"), MoveTemp(OutModifiedPayloads));

	FString JsonOutput{};
	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return FString{};
	}
	else
	{
		return JsonOutput;
	}
}

FString USkillTreeUtils::CreateSkillTreeProvider(const int32 ProviderId,
                                                 const TArray<int32>& NewPrivateTreeNodeIds)
{
	FString OutProvider{};
	NSJsonSkillTree::ToString(NSJsonSkillTree::FJsonSkillTreeProvider{ProviderId, NewPrivateTreeNodeIds}, OutProvider);
	return OutProvider;
}

FString USkillTreeUtils::ModifySkillTreeProvider(const FString& NewPayload,
                                                 const int32 ProviderId,
                                                 const TArray<int32>& NewPrivateTreeNodeIds)
{
	TArray<NSJsonSkillTree::FJsonSkillTreeProvider> SkillTreeProviders{};
	for (const FString& Payload : GetSkillTreeProviderPayloads(NewPayload))
	{
		NSJsonSkillTree::FJsonSkillTreeProvider OutProvider;
		NSJsonSkillTree::FromString(Payload, OutProvider);
		SkillTreeProviders.Add(OutProvider);
	}

	auto* SearchResult = SkillTreeProviders.FindByPredicate([SearchId = ProviderId](const NSJsonSkillTree::FJsonSkillTreeProvider& Provider)
	{
		return (false == (Provider.Id ^ SearchId));
	});

	if (SearchResult != nullptr)
	{
		// @gdemers overwrite all item entries within this provider.
		SearchResult->PrivateTreeNodeIds = NewPrivateTreeNodeIds;
	}

	TArray<TSharedPtr<FJsonValue>> OutModifiedPayloads{};
	for (const auto& ModifiedProvider : SkillTreeProviders)
	{
		FString OutFormat{};
		NSJsonSkillTree::ToString(ModifiedProvider, OutFormat);
		OutModifiedPayloads.Add(MakeShareable(new FJsonValueString(MoveTemp(OutFormat))));
	}

	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetArrayField(TEXT("SkillTreeProviders"), MoveTemp(OutModifiedPayloads));

	FString JsonOutput{};
	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return FString{};
	}
	else
	{
		return JsonOutput;
	}
}

TArray<FString> USkillTreeUtils::GetSkillTreeProviderPayloads(const FString& NewPayload)
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return TArray<FString>{};
	}

	TArray<FString> OutProviders{};
	for (const auto& SkillTreeProvider : JsonData->GetArrayField(TEXT("SkillTreeProviders")))
	{
		OutProviders.Add(SkillTreeProvider->AsString());
	}

	return OutProviders;
}

int32 USkillTreeUtils::CreateDefaultPrivateTreeNodeId(const FDataRegistryId& TreeNodeEffectRegistryId,
                                                      const int32 RelationshipBitMask,
                                                      const int32 InstancedId,
                                                      const int32 EffectLevel)
{
	const FDataRegistryId GameplayEffectUniqueIdentifierRegistryId = {UAVVMGameplaySettings::GetGameplayEffectIdentifierRegistryType(), TreeNodeEffectRegistryId.ItemName};
	const int32 PhysicalGlobalId = UAVVMGameplayUtils::GetGameplayEffectUniqueIdentifierByRegistryId(GameplayEffectUniqueIdentifierRegistryId);
	const int32 VirtualGlobalId = UAVVMOnlineSkillTreeUtils::TranslatePhysicalAddressing(RelationshipBitMask, PhysicalGlobalId);
	const int32 NewInstancedId = UAVVMOnlineEncodingUtils::EncodeInt32(InstancedId, GET_SKILL_TREE_NODE_INSTANCED_ID_BIT_RANGE, GET_SKILL_TREE_NODE_INSTANCED_ID_RSHIFT);
	
	const int32 NewEffectLevel = UAVVMOnlineEncodingUtils::EncodeInt32(FMath::Clamp(EffectLevel, 1/*min required level*/, INT32_MAX), GET_SKILL_TREE_NODE_LEVEL_BIT_RANGE, GET_SKILL_TREE_NODE_LEVEL_RSHIFT);
	
	// TODO @gdemers we are missing position support.
	return (RelationshipBitMask
		+ VirtualGlobalId
		+ NewInstancedId
		+ NewEffectLevel);
}

FString USkillTreeUtils::GetSkillTreeProviderById(const FString& NewPayload,
                                                  const int32 NewProviderId)
{
	const TArray<FString> SkillTreeProviders = GetSkillTreeProviderPayloads(NewPayload);
	if (SkillTreeProviders.IsEmpty())
	{
		return FString{};
	}

	const FString* SearchResult = SkillTreeProviders.FindByPredicate([SearchId = NewProviderId](const FString& Payload)
	{
		NSJsonSkillTree::FJsonSkillTreeProvider OutProvider;
		NSJsonSkillTree::FromString(Payload, OutProvider);
		return (false == (OutProvider.Id ^ SearchId));
	});

	if (SearchResult != nullptr)
	{
		return *SearchResult;
	}
	else
	{
		return FString{};
	}
}

void USkillTreeUtils::GetSkillTreeProvider(const FString& NewPayload,
                                           int32& OutProviderId,
                                           TArray<int32>& OutPrivateTreeNodeIds)
{
	NSJsonSkillTree::FJsonSkillTreeProvider OutProvider{};
	NSJsonSkillTree::FromString(NewPayload, OutProvider);

	OutProviderId = OutProvider.Id;
	OutPrivateTreeNodeIds = OutProvider.PrivateTreeNodeIds;
}

int32 USkillTreeUtils::GetSkillTreeNodePrivateId(const FString& NewPayload,
                                                 const TArray<int32>& NewPrivateIds,
                                                 const int32 PhysicalGlobalId)
{
	NSJsonSkillTree::FJsonSkillTreeProvider OutProvider{};
	NSJsonSkillTree::FromString(NewPayload, OutProvider);

	TArray<int32> FilteredSet = OutProvider.PrivateTreeNodeIds;
	for (const int32 PrivateId : NewPrivateIds)
	{
		FilteredSet.Remove(PrivateId);
	}

	const int32* SearchResult = FilteredSet.FindByPredicate([SearchId = PhysicalGlobalId](const int32 Value)
	{
		// @gdemers filter the PrivateItemId that represent our complex encoding, and translate the virtual id parsed
		// from the integer into a physical id for comparison.
		const int32 OutPhysicalGlobalId = UAVVMOnlineSkillTreeUtils::GetPhysicalGlobalId(Value);
		return (false == (OutPhysicalGlobalId ^ SearchId))/*if both bits are identical, return 0.*/;
	});

	if (SearchResult != nullptr)
	{
		return *SearchResult;
	}
	else
	{
		return INDEX_NONE;
	}
}

TArray<FDataRegistryId> USkillTreeUtils::TranslatePrivateItemId(const TArray<int32>& NewPrivateItemIds)
{
	static const auto GetRegistryId = [](const TArray<FDataRegistryId>& NewRegistryIds,
										 const TWeakObjectPtr<const UDataRegistrySubsystem>& DataRegistrySubsystem,
										 const int32 NewPrivateItemId)
	{
		if (!DataRegistrySubsystem.IsValid())
		{
			return FDataRegistryId{};
		}

		const int32 PhysicalGlobalId = UAVVMOnlineSkillTreeUtils::GetPhysicalGlobalId(NewPrivateItemId);
		for (const auto& RegistryId : NewRegistryIds)
		{
			const auto* Row = DataRegistrySubsystem->GetCachedItem<FAVVMGameplayEffectIdentifierDataTableRow>(RegistryId);
			if (ensureAlwaysMsgf(Row != nullptr, TEXT("Invalid Row.")) && (Row->UniqueId == PhysicalGlobalId))
			{
				return RegistryId;
			}
		}

		return FDataRegistryId{};
	};

	const auto* Subsystem = UDataRegistrySubsystem::Get();
	if (!IsValid(Subsystem))
	{
		return TArray<FDataRegistryId>{};
	}

	TArray<FDataRegistryId> OutRegistryIds{};
	Subsystem->GetPossibleDataRegistryIdList(UAVVMGameplaySettings::GetGameplayEffectIdentifierRegistryType(), OutRegistryIds);

	TArray<FDataRegistryId> OutResults{};
	for (const int32 PrivateItemId : NewPrivateItemIds)
	{
		const FDataRegistryId ItemRegistryId = GetRegistryId(OutRegistryIds, Subsystem, PrivateItemId);
		if (!ensureAlwaysMsgf(ItemRegistryId.IsValid(), TEXT("Invalid Registry Id.")))
		{
			continue;
		}

		OutResults.Add(FDataRegistryId
		               {
				               USkillTreeSettings::GetSkillTreeNodeRegistryType(),
				               ItemRegistryId.ItemName
		               });
	}

	return OutResults;
}

bool USkillTreeUtils::GetOuterSourceType(const AActor* Outer, ESkillTreeSrcType& OutSrcType)
{
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")) ||
		!UAVVMToolkitUtils::IsBlueprintScriptInterfaceValid<const USkillTreeProvider>(Outer))
	{
		return false;
	}

	OutSrcType = ISkillTreeProvider::Execute_GetSkillTreeSrcType(Outer);
	const bool bIsNone = EnumHasAnyFlags(OutSrcType, ESkillTreeSrcType::None);
	if (!ensureAlwaysMsgf(!bIsNone, TEXT("ISkillTreeProvider::GetSkillTreeSrcType is None. Check if it was properly overriden.")))
	{
		return false;
	}

	return true;
}

TArray<FDataRegistryId> USkillTreeUtils::GetProviderSkillTreeRegistryIds(const int32 NewProviderId)
{
	// TODO @gdemers These require rework!
	// // @gdemers lambda to conditionally generate our default provider content
	// // for serialization to disk.
	// static const auto GenerateDefaultContent = []()
	// {
	// 	return USkillTreeUtils::CreateDefaultSkillTreeProviders();
	// };
	//
	// const FStringView FileContent = UAVVMSaveGame::Static_GetSetFileContent(SkillTreeProviderPayloads, GenerateDefaultContent);
	// const FString SearchPayload = USkillTreeUtils::GetSkillTreeProviderById(FileContent.GetData(), NewProviderId);
	//
	// NSJsonSkillTree::FJsonSkillTreeProvider OutProvider{};
	// NSJsonSkillTree::FromString(SearchPayload, OutProvider);
	// return TranslatePrivateItemId(OutProvider.PrivateTreeNodeIds);
	return {};
}

TArray<FDataRegistryId> USkillTreeUtils::GetBackendProviderSkillTreeRegistryIds(const UObject* WorldContextObject,
                                                                                const int32 NewProfileId)
{
	TArray<int32> PrivateItemIds = AAVVMGameSession::Static_GetPlayerSkillTreeNodes(WorldContextObject, NewProfileId);
	PrivateItemIds.RemoveAll([](const int32 PrivateItemId)
	{
		const int32 RelationshipBitmask = UAVVMOnlineEncodingUtils::DecodeInt32(PrivateItemId, GET_SKILL_TREE_NODE_RELATIONSHIP_BIT_RANGE, GET_SKILL_TREE_NODE_RELATIONSHIP_RSHIFT);
		return (false != (RelationshipBitmask ^ FILTER_CHARACTER_RELATIONSHIP_BIT)/*XOR 1 for elements that arent Character dependent*/);
	});

	return TranslatePrivateItemId(PrivateItemIds);
}

TArray<FDataRegistryId> USkillTreeUtils::GetProviderDependentSkillTreeRegistryIds(const int32 NewProviderId,
                                                                                  const int32 NewPrivateItemId)
{
	// TODO @gdemers These require rework!
	// // @gdemers lambda to conditionally generate our default provider content
	// // for serialization to disk.
	// static const auto GenerateDefaultContent = []()
	// {
	// 	return USkillTreeUtils::CreateDefaultSkillTreeProviders();
	// };
	//
	// const FStringView FileContent = UAVVMSaveGame::Static_GetSetFileContent(SkillTreeProviderPayloads, GenerateDefaultContent);
	// const FString SearchPayload = USkillTreeUtils::GetSkillTreeProviderById(FileContent.GetData(), NewProviderId);
	//
	// NSJsonSkillTree::FJsonSkillTreeProvider OutProvider{};
	// NSJsonSkillTree::FromString(SearchPayload, OutProvider);
	//
	// const TArray<int32> FilteredSet = USkillTreeUtils::FilterSkillIds(OutProvider.PrivateTreeNodeIds, NewPrivateItemId);
	// return TranslatePrivateItemId(FilteredSet);
	return {};
}

TArray<FDataRegistryId> USkillTreeUtils::GetBackendProviderDependentSkillTreeRegistryIds(const UObject* WorldContextObject,
                                                                                         const int32 NewProfileId,
                                                                                         const int32 NewPrivateItemId)
{
	// @gdemers Parse the dependency graph to retrieved skills tied to the given target actor.
	TArray<int32> SkillDependencyGraphElements = AAVVMGameSession::Static_GetPlayerSkillDependencyGraph(WorldContextObject, NewProfileId);
	SkillDependencyGraphElements.RemoveAll([OwnerPrivateItemId = NewPrivateItemId](const int32 SkillDependencyGraphElementId)
	{
		const int32 OwnerRelationshipBitmask = UAVVMOnlineEncodingUtils::DecodeInt32(OwnerPrivateItemId, GET_SKILL_TREE_NODE_RELATIONSHIP_BIT_RANGE, GET_SKILL_TREE_NODE_RELATIONSHIP_RSHIFT);
		const int32 ElementOwnerRelationshipBitmask = UAVVMOnlineEncodingUtils::DecodeInt32(OwnerPrivateItemId, GET_SKILL_TREE_NODE_LOOKUP_RELATIONSHIP_BIT_RANGE, GET_SKILL_TREE_NODE_LOOKUP_RELATIONSHIP_RSHIFT);
		const bool bDoesShareRelationship = (ElementOwnerRelationshipBitmask/*Attachment*/ & OwnerRelationshipBitmask/*example : Attachment+Item*/);
		if (!bDoesShareRelationship)
		{
			return false;
		}

		// @gdemers we use DecodeInt32 instead of FilterInt32 due to the encoding scheme being different between both entity.
		const int32 OwnerVirtualId = UAVVMOnlineEncodingUtils::DecodeInt32(OwnerPrivateItemId, GET_SKILL_TREE_NODE_VIRTUAL_GLOBAL_ID_BIT_RANGE, GET_SKILL_TREE_NODE_VIRTUAL_GLOBAL_ID_RSHIFT);
		const int32 ElementOwnerVirtualId = UAVVMOnlineEncodingUtils::DecodeInt32(SkillDependencyGraphElementId, GET_SKILL_TREE_NODE_LOOKUP_OWNER_VIRTUAL_GLOBAL_ID_BIT_RANGE, GET_SKILL_TREE_NODE_LOOKUP_OWNER_VIRTUAL_GLOBAL_ID_RSHIFT);
		const bool bAreSameActorType = (false == (OwnerVirtualId ^ ElementOwnerVirtualId));
		if (bAreSameActorType)
		{
			const int32 DependantInstancedId = UAVVMOnlineEncodingUtils::DecodeInt32(OwnerPrivateItemId, GET_SKILL_TREE_NODE_INSTANCED_ID_BIT_RANGE, GET_SKILL_TREE_NODE_INSTANCED_ID_RSHIFT);
			const int32 TargetInstancedId = UAVVMOnlineEncodingUtils::DecodeInt32(SkillDependencyGraphElementId, GET_SKILL_TREE_NODE_LOOKUP_OWNER_INSTANCED_ID_BIT_RANGE, GET_SKILL_TREE_NODE_LOOKUP_OWNER_INSTANCED_ID_RSHIFT);
			return (false == (DependantInstancedId ^ TargetInstancedId));
		}
		else
		{
			return false;
		}
	});

	return TranslatePrivateItemId(SkillDependencyGraphElements);
}
