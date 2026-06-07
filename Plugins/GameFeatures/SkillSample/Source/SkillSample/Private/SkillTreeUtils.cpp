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

#include "AVVMToolkitUtils.h"
#include "DataRegistrySubsystem.h"
#include "GameplayEffect.h"
#include "SkillTreeNodeObject.h"
#include "SkillTreeProvider.h"
#include "SkillTreeSettings.h"
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
		int32 Id = INDEX_NONE;
		TArray<int32> PrivateTreeNodeIds;
	};

	void ToString(const FJsonSkillTreeProvider& NewSkillTreeProvider,
	              FString& OutFormat)
	{
		TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
		JsonData->SetNumberField(TEXT("Id"), NewSkillTreeProvider.Id);

		TArray<TSharedPtr<FJsonValue>> PrivateTreeNodeIds;
		for (const int32 PrivateTreeNodeId : NewSkillTreeProvider.PrivateTreeNodeIds)
		{
			PrivateTreeNodeIds.Add(MakeShareable(new FJsonValueNumber(PrivateTreeNodeId)));
		}

		JsonData->SetArrayField(TEXT("PrivateTreeNodeIds"), PrivateTreeNodeIds);

		FString JsonOutput;

		auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
		if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
		{
			return;
		}

		OutFormat = JsonOutput;
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

		FJsonSkillTreeProvider SkillTreeProvider;
		SkillTreeProvider.Id = JsonData->GetIntegerField(TEXT("Id"));

		const TArray<TSharedPtr<FJsonValue>> PrivateTreeNodeIds = JsonData->GetArrayField(TEXT("PrivateTreeNodeIds"));
		for (const auto& PrivateTreeNodeId : PrivateTreeNodeIds)
		{
			SkillTreeProvider.PrivateTreeNodeIds.Add(PrivateTreeNodeId->AsNumber());
		}

		OutSkillTreeProvider = SkillTreeProvider;
	}
}

FString USkillTreeUtils::CreateDefaultSkillTreeProviders()
{
	const TSoftObjectPtr<UDataTable>& ProviderDataTable = USkillTreeSettings::GetDefaultProviderSkillTrees();
	if (!ensureAlwaysMsgf(!ProviderDataTable.IsNull(),
	                      TEXT("Project doesn't reference a valid Data Table to initialize the Provider Skill Tree on Disk.")))
	{
		return FString();
	}

	// TODO @gdemers Improve on this. I dont like that its synchronous.
	const UDataTable* DataTable = ProviderDataTable.LoadSynchronous();
	if (!IsValid(DataTable))
	{
		return FString();
	}

	TArray<FSkillTreeProviderTableRow*> OutRows;
	DataTable->GetAllRows<FSkillTreeProviderTableRow>(TEXT(""), OutRows);

	TArray<TSharedPtr<FJsonValue>> OutModifiedPayloads;
	for (const FSkillTreeProviderTableRow* Row : OutRows)
	{
		if (!ensureAlwaysMsgf(Row != nullptr, TEXT("Invalid Row entry.")))
		{
			continue;
		}

		const int32 ProviderId = USkillTreeUtils::GetSkillTreeNodeUniqueIdentifier(Row->SkillTreeProviderActorIdentifierId);
		if (!ensureAlwaysMsgf(ProviderId != INDEX_NONE,
		                      TEXT("Missing valid Id for Provider entry.")))
		{
			continue;
		}

		const FString OutProvider = USkillTreeUtils::CreateSkillTreeProvider(ProviderId, Row->SkillTreeNodePerPhases);
		OutModifiedPayloads.Add(MakeShareable(new FJsonValueString(OutProvider)));
	}

	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetArrayField(TEXT("SkillTreeProviders"), OutModifiedPayloads);

	FString JsonOutput;

	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return FString();
	}
	else
	{
		return JsonOutput;
	}
}

FString USkillTreeUtils::CreateSkillTreeProvider(const int32 ProviderId,
                                                 const TArray<FSkillTreeNodePhase>& SkillTreeNodePhases)
{
	return FString();
}

TArray<FString> USkillTreeUtils::GetSkillTreeProviderPayloads(const FString& NewPayload)
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return TArray<FString>();
	}

	TArray<FString> OutProviders;

	const TArray<TSharedPtr<FJsonValue>> SkillTreeProviders = JsonData->GetArrayField(TEXT("SkillTreeProviders"));
	for (const auto& SkillTreeProvider : SkillTreeProviders)
	{
		OutProviders.Add(SkillTreeProvider->AsString());
	}

	return OutProviders;
}

FString USkillTreeUtils::GetSkillTreeProviderById(const FString& NewPayload,
                                                  const int32 NewProviderId)
{
	const TArray<FString> SkillTreeProviders = GetSkillTreeProviderPayloads(NewPayload);
	if (SkillTreeProviders.IsEmpty())
	{
		return FString();
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
		return TEXT("");
	}
}

int32 USkillTreeUtils::GetSkillTreeNodePrivateId(const FString& NewPayload,
                                                 const TArray<int32>& NewPrivateIds,
                                                 const int32 SkillTreeId)
{
	NSJsonSkillTree::FJsonSkillTreeProvider OutProvider;
	NSJsonSkillTree::FromString(NewPayload, OutProvider);

	TArray<int32> FilteredSet = OutProvider.PrivateTreeNodeIds;
	for (const int32 PrivateId : NewPrivateIds)
	{
		FilteredSet.Remove(PrivateId);
	}

	const int32* SearchResult = FilteredSet.FindByPredicate([SearchId = SkillTreeId](const int32 Value)
	{
		// @gdemers filter Value (PrivateItemId) of the disk representation of the item, and parse it's type, returning an output value
		// that respect our initial bit encoding defined under AVVMOnlineSkillTree.h
		const int32 OutValue = USkillTreeNodeObjectUtils::FilterTreeNodePrivateId(Value);
		return (false == (OutValue ^ SearchId))/*if both bits are identical, return 0.*/;
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

int32 USkillTreeUtils::GetObjectUniqueIdentifier(const UGameplayEffect* SkillTreeNodeEffect)
{
	if (!IsValid(SkillTreeNodeEffect))
	{
		return INDEX_NONE;
	}
	else
	{
		return USkillTreeUtils::GetSkillTreeNodeUniqueIdentifier({USkillTreeSettings::GetGameplayEffectIdentifierRegistryType(), SkillTreeNodeEffect->GetFName()});
	}
}

int32 USkillTreeUtils::GetSkillTreeNodeUniqueIdentifier(const FDataRegistryId& SkillTreeNodeId)
{
	if (!ensureAlwaysMsgf(SkillTreeNodeId.IsValid(),
						  TEXT("Composed RegistryId isn't valid. You may be missing a reference in DeveloperSettings for the TreeNode Identifier RegistryType.")))
	{
		return INDEX_NONE;
	}

	auto* Subsystem = UDataRegistrySubsystem::Get();
	if (!IsValid(Subsystem))
	{
		return INDEX_NONE;
	}

	// @gdemers imply we pre-cache our DT (which is fine! we can set that in editor, and is lightweight)
	const auto* RowValue = Subsystem->GetCachedItem<FAVVMGameplayEffectIdentifierDataTableRow>(SkillTreeNodeId);
	if (ensureAlwaysMsgf(RowValue != nullptr,
						 TEXT("Invalid Row Entry. Make sure FAVVMGameplayEffectIdentifierDataTableRow match the Data Table.")))
	{
		return RowValue->UniqueId;
	}
	else
	{
		return INDEX_NONE;
	}
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
