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
#pragma once

#include "CoreMinimal.h"

#include "DataRegistryId.h"
#include "GameplayTagContainer.h"
#include "Data/AVVMDataTableRow.h"
#include "Engine/DataAsset.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include "AttachmentDefinitionDataAsset.generated.h"

/**
 *	Class description:
 *
 *	UAttachmentDefinitionDataAsset is a POD asset that defines the properties of an attachment.
 */
UCLASS(BlueprintType, NotBlueprintable)
class WEAPONSAMPLE_API UAttachmentDefinitionDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	const FDataRegistryId& GetAttachmentActorId() const;
	
	bool CanAccessItem(const FGameplayTagContainer& RequirementTags,
	                   const FGameplayTagContainer& BlockingTags) const;

protected:
	// @gdemers required for supporting AttributeSet initialization of our actor.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(ItemStruct="AVVMActorDefinitionDataTableRow"))
	FDataRegistryId AttachmentActorId = FDataRegistryId();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	FGameplayTagContainer RequiredTagsForItemAccess = FGameplayTagContainer::EmptyContainer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	FGameplayTagContainer BlockingTagsForItemAccess = FGameplayTagContainer::EmptyContainer;
};

/**
 *	Class description:
 *
 *	FAttachmentDefinitionDataTableRow is an entry in a DataTableRow for a unique UAttachmentDefinitionDataAsset.
 */
USTRUCT(BlueprintType)
struct WEAPONSAMPLE_API FAttachmentDefinitionDataTableRow : public FAVVMDataTableRow
{
	GENERATED_BODY()

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	virtual TArray<FSoftObjectPath> GetResourcesPaths() const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	TSoftObjectPtr<UAttachmentDefinitionDataAsset> AttachmentDefinition = nullptr;
};
