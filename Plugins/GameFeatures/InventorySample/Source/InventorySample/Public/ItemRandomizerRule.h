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

#include "AVVMWorldSetting.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include "ItemRandomizerRule.generated.h"

struct FStreamableHandle;
class UItemObject;

/**
 *	Class description:
 *	
 *	UItemRandomizerImpl is an Impl Object that defines how an Actor in world should handle releasing from authority Items
 *	with support for randomization.
 *	
 *	Example : Destructible crates may have high drop rate, and can randomly release from their inventory component
 *	high value items, but over time support decreasing ratio of high value drop (or even return nothing) based on player progression.
 */
UCLASS(BlueprintType, Blueprintable)
class UItemRandomizerImpl : public UObject
{
	GENERATED_BODY()

public:
	virtual TArray<UItemObject*> GetFilteredItems(const TArray<UItemObject*>& Items) const PURE_VIRTUAL(GetFilteredItems, return {};);
	const FGameplayTag& GetImplTag() const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	FGameplayTag CategoryTag = FGameplayTag::EmptyTag;
};

/**
 *	Class description:
 *
 *	UItemRandomizerRule is a Rule referenced in AVVMWorldSettings. This system defines which elements from a collection of items
 *	is selected based on user defined impl.
 */
UCLASS()
class INVENTORYSAMPLE_API UItemRandomizerRule : public UAVVMWorldRule
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	virtual void PostInitProperties() override;
	TArray<UItemObject*> GetRandomSubset(const AActor* Outer,
	                                     const TArray<UItemObject*>& Items) const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	TArray<TSoftClassPtr<UItemRandomizerImpl>> ImplClasses;

	UPROPERTY(Transient)
	TMap<FGameplayTag, TWeakObjectPtr<const UItemRandomizerImpl>/*CDO*/> Impls;

	TSharedPtr<FStreamableHandle> StreamableHandle = nullptr;
};
