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
#include "Engine/StreamableManager.h"

#include "AVVMAbilitySystemComponent.generated.h"

/**
 *	Class description:
 *
 *	FAbilityToken describe a unique identifier that increment only when default construct. Can be safely
 *	passed by copy around.
 */
USTRUCT(BlueprintType)
struct AVVMGAMEPLAY_API FAbilityToken
{
	GENERATED_BODY()

	explicit FAbilityToken()
	{
		static uint32 GlobalUniqueId = 0;
		UniqueId = ++GlobalUniqueId;
	}

	UPROPERTY()
	uint32 UniqueId = 0;
};

/**
 *	Class description:
 *
 *	UAVVMAbilitySystemComponent extend base behaviour from Unreal Ability system component and support
 *	default initialization of Actor Abilities based on data received.
 */
UCLASS(ClassGroup=("AVVMGameplay"), Blueprintable, meta=(BlueprintSpawnableComponent))
class AVVMGAMEPLAY_API UAVVMAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void SetupAbilities(const TArray<UObject*>& Resources);
	void SetupAttributeSet(const FSoftObjectPath& AttributeSetSoftObjectPath);

protected:
	UFUNCTION()
	void OnAbilityGrantingDeferred(FAbilityToken AbilityToken);

	virtual void OnTagUpdated(const FGameplayTag& Tag, bool TagExists) override;

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<const AActor> OwningOuter = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly)
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

	TMap<uint32, TSharedPtr<FStreamableHandle>> AbilityHandleSystem;
	TSharedPtr<FStreamableHandle> AttributeSetHandle = nullptr;
};
