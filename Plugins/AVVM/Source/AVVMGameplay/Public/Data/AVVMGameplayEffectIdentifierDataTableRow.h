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

#include "Engine/DataTable.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include "AVVMGameplayEffectIdentifierDataTableRow.generated.h"

/**
 *	Class description:
 *	
 *	FAVVMGameplayEffectIdentifierDataTableRow is a table row entry thet define a unique identifier for a given GameplayEffect type. Doing so allows
 *	our Backend encoding system to reference elements, and create complex encoding scheme for tightly packing data. (See. AVVMOnline & SkillSample)
 *	
 *	Note : We do expect some rules to be followed. Make sure the RowName define for this entry match the GameplayEffect Class FName, and if you are
 *	using the SkillSample system, make sure the FDataRegistryId::ItemName is identical to the GameplayEffect Class FName referenced.
 *	
 *	IMPORTANT : This separation is for clarity of design. I understand this is identical to FAVVMActorIdentifierDataTableRow, but allowing proper
 *	separation ensure clean organization of our data, and expose editor tooling to parsing data table per unique types.
 */
USTRUCT(BlueprintType)
struct AVVMGAMEPLAY_API FAVVMGameplayEffectIdentifierDataTableRow : public FTableRowBase
{
	GENERATED_BODY()

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	// TODO @gdemers make editor tooling that allow default assignment of next value in sequence, and can filter by user class type. (example : warrior, mage, etc...)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers", meta=(ClampMin="0"))
	int32 UniqueId = INDEX_NONE;
};
