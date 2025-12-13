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

#include "GameFeatureAction.h"
#include "GameFeaturesSubsystem.h"
#include "AVVMGameFrameworkInputMappingContextManager.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include "AVVMGameFeatureAction_AddInputMappingContext.generated.h"

class UInputMappingContext;

/**
 *	Class description:
 *	
 *	UAVVMGameFeatureAction_AddInputMappingContext is a GameFeatureAction that push IMCs context onto locally controlled PC.
 */
UCLASS(meta=(DisplayName="Add Input Mapping Context"))
class AVVMGAMEPLAY_API UAVVMGameFeatureAction_AddInputMappingContext : public UGameFeatureAction
{
	GENERATED_BODY()

	/**
	 *	Class description:
	 *	
	 *	FContextHandles is a context struct that cache Delegate Handle for GameInstance Started.
	 */
	struct FContextHandles
	{
		FDelegateHandle GameInstanceStartHandle;
	};

public:
	//~UGameFeatureAction interface
	virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override;
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
	//~End of UGameFeatureAction interface

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

protected:
	void HandleGameInstanceStart(UGameInstance* GameInstance, FGameFeatureStateChangeContext ChangeContext);
	void AddToWorld(const FWorldContext& WorldContext, FContextHandles& Handles);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Designers")
	TArray<TSoftObjectPtr<UInputMappingContext>> IMCs;

	TMap<FGameFeatureStateChangeContext, FContextHandles> ContextHandles;
};
