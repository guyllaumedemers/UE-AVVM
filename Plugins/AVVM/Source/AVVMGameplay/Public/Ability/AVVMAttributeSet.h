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

#include "AbilitySystemComponent.h"
#include "AttributeSet.h"

#include "AVVMAttributeSet.generated.h"

struct FStreamableHandle;
class UDataTable;

/**
 *	Class description:
 *	
 *	UAVVMAttributeSet is an abstract class used to handle property initialization
 *	based on derived type hard reference to an internal DT (FAttributeMetaData).
 */
UCLASS(Abstract, BlueprintType, NotBlueprintable)
class AVVMGAMEPLAY_API UAVVMAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Init();
	
	ATTRIBUTE_ACCESSORS_BASIC(UAVVMAttributeSet, Durability);
	ATTRIBUTE_ACCESSORS_BASIC(UAVVMAttributeSet, Weight);

protected:
	// ------------------- FAttributeSetProperties ------------------- //
	// @gdemers IMPORTANT : How we initialize our properties.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers|FAttributeSetProperties")
	TSoftObjectPtr<UDataTable> AttributeMetaDataTable = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly, Replicated, Category="Designers|FAttributeSetProperties")
	FGameplayAttributeData Durability = FGameplayAttributeData();

	UPROPERTY(Transient, BlueprintReadOnly, Replicated, Category="Designers|FAttributeSetProperties")
	FGameplayAttributeData Weight = FGameplayAttributeData();

	TSharedPtr<FStreamableHandle> AttributeMetaDataTableHandle = nullptr;
};

/**
 *	Class description:
 *	
 *	IAVVMDoesOwnAttributeSet is an interface class that allow caching of AttributeSet on Actor class
 *	that instance an AttributeSet based on its AVVMActorDefinitionAsset representation.
 */
UINTERFACE(BlueprintType)
class AVVMGAMEPLAY_API UAVVMDoesOwnAttributeSet : public UInterface
{
	GENERATED_BODY()
};

class AVVMGAMEPLAY_API IAVVMDoesOwnAttributeSet
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetAttributeSet(const UAttributeSet* NewAttributeSet);
	virtual void SetAttributeSet_Implementation(const UAttributeSet* NewAttributeSet) PURE_VIRTUAL(SetAttributeSet_Implementation, return;);
};
