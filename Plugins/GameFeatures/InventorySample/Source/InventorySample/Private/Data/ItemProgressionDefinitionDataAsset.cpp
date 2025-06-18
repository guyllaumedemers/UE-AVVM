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
#include "Data/ItemProgressionDefinitionDataAsset.h"

#include "Engine/AssetManager.h"

const TSoftClassPtr<AActor>& UItemProgressionStageDefinitionDataAsset::GetOverrideItemActorClass() const
{
	return OverrideItemActorClass;
}

bool UItemProgressionStageDefinitionDataAsset::CanOverrideItemActorClass() const
{
	return bDoesOverrideItemActorClass;
}

void UItemProgressionDefinitionDataAsset::GetItemActorClassAsync(const int32 ProgressionStageIndex,
                                                                 const FOnRequestItemActorClassComplete& Callback)
{
	if (!ItemProgressionStageDataAssets.IsEmpty() &&
		ensureAlwaysMsgf((ItemProgressionStageDataAssets.Num() > ProgressionStageIndex) && (ProgressionStageIndex >= 0),
		                 TEXT("UItemProgressionDefinitionDataAsset Invalid Index Access!")))
	{
		FStreamableDelegate OnProgressionStageDefinitionLoaded;
		OnProgressionStageDefinitionLoaded.BindUObject(this, &UItemProgressionDefinitionDataAsset::OnSoftObjectAcquired, Callback);
		ItemProgressionStageHandle = UAssetManager::Get().LoadAssetList({ItemProgressionStageDataAssets[ProgressionStageIndex].ToSoftObjectPath()}, OnProgressionStageDefinitionLoaded);
	}
	else
	{
		Callback.ExecuteIfBound(DefaultItemActorClass);
	}
}

const TSoftClassPtr<AActor>& UItemProgressionDefinitionDataAsset::GetDefaultItemActorClass() const
{
	return DefaultItemActorClass;
}

void UItemProgressionDefinitionDataAsset::OnSoftObjectAcquired(FOnRequestItemActorClassComplete OnRequestItemActorClassComplete)
{
	if (!ensureAlwaysMsgf(ItemProgressionStageHandle.IsValid(),
	                      TEXT("UItemProgressionDefinitionDataAsset trying to access invalid FStreamableHandle")))
	{
		OnRequestItemActorClassComplete.ExecuteIfBound(DefaultItemActorClass);
		return;
	}

	TArray<UObject*> OutStreamedAssets;
	ItemProgressionStageHandle->GetLoadedAssets(OutStreamedAssets);

	if (!OutStreamedAssets.IsEmpty())
	{
		const auto* ProgressionStageItemDefinitionDataAsset = Cast<UItemProgressionStageDefinitionDataAsset>(OutStreamedAssets[0]);
		if (ensureAlwaysMsgf(IsValid(ProgressionStageItemDefinitionDataAsset),
		                     TEXT("UItemProgressionDefinitionDataAsset trying to Cast object to UItemProgressionStageDefinitionDataAsset failed!")) &&
			ProgressionStageItemDefinitionDataAsset->CanOverrideItemActorClass())
		{
			OnRequestItemActorClassComplete.ExecuteIfBound(ProgressionStageItemDefinitionDataAsset->GetOverrideItemActorClass());
		}
		else
		{
			OnRequestItemActorClassComplete.ExecuteIfBound(DefaultItemActorClass);
		}
	}
	else
	{
		OnRequestItemActorClassComplete.ExecuteIfBound(DefaultItemActorClass);
	}
}

#if WITH_EDITOR
EDataValidationResult FItemProgressionDefinitionDataTableRow::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);
	if (ItemProgressionDefinition.IsNull())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(NSLOCTEXT("FAVVMItemProgressionDefinitionDataTableRow", "", "UItemProgressionDefinitionDataAsset missing. No valid TSoftObjectPtr specified!"));
	}

	return Result;
}
#endif

TArray<FSoftObjectPath> FItemProgressionDefinitionDataTableRow::GetResourcesPaths() const
{
	return {ItemProgressionDefinition.ToSoftObjectPath()};
}
