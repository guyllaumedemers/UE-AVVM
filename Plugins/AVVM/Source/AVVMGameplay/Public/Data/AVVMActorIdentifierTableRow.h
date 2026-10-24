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

#include "AVVMDataTableRow.h"

#if WITH_EDITOR
#include "Engine/DataAsset.h"
#include "Misc/DataValidation.h"
#endif

#include "AVVMActorIdentifierTableRow.generated.h"

/**
 *	Class description:
 *	
 *	UAVVMActorIdentifierPreviewDataAsset is a EDITOR ONLY UObject that
 *	server as SWidget preview to the FAVVMActorIdentifierDataTableRow at Editor Time.
 */
#if WITH_EDITORONLY_DATA
UCLASS(BlueprintType, NotBlueprintable)
class AVVMGAMEPLAY_API UAVVMActorIdentifierPreviewDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	int32 UniqueId{INDEX_NONE};
};
#endif

/**
 *	Class description:
 *	
 *	FAVVMActorIdentifierDataTableRow is a table row entry thet define a unique identifier for a given Actor type. Doing so allows
 *	our Backend encoding system to reference elements, and create complex encoding scheme for tightly packing data. (See. AVVMOnline & InventorySample)
 *	
 *	Note : We do expect some rules to be followed. Make sure the RowName define for this entry match the Actor Class FName, and if you are
 *	using the InventorySample system, make sure the FDataRegistryId::ItemName is identical to the Actor Class FName referenced.
 */
USTRUCT(BlueprintType)
struct AVVMGAMEPLAY_API FAVVMActorIdentifierDataTableRow : public FAVVMDataTableRow
{
	GENERATED_BODY()

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
	
	// @gdemers IMPORTANT : We usually wouldnt define this api here, but for since AVVMEditorToolkit preview
	// all dependent asset from a given Row type, we need to address the missing api call.
	virtual TArray<FSoftObjectPath> GetResourcesPaths() const override;
#endif

	bool operator<(const FAVVMActorIdentifierDataTableRow& Rhs) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(ClampMin="0"))
	int32 UniqueId{INDEX_NONE};
};
