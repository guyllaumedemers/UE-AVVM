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

#include "Subsystems/WorldSubsystem.h"

#include "AutomatedTestInventoryValidationManager.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct INVENTORYSAMPLE_API FInventoryValidationMechanism
{
	GENERATED_BODY()

	UPROPERTY(Transient, BlueprintReadOnly)
	int32 ResourceProviderId = INDEX_NONE;

	UPROPERTY(Transient, BlueprintReadOnly)
	TArray<int32> PrivateItemIds;
};

/**
 *	Class description:
 *	
 *	UAutomatedTestInventoryValidationManager is a class that run validation on resource loading/initialization process for the inventory system
 *	during automated testing.
 */
UCLASS()
class INVENTORYSAMPLE_API UAutomatedTestInventoryValidationManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	UFUNCTION(BlueprintCallable)
	static void Static_RegisterComponent(const UWorld* World,
	                                     const UActorInventoryComponent* SrcComponent);

	UFUNCTION(BlueprintCallable)
	static void Static_UnregisterComponent(const UWorld* World,
	                                       const UActorInventoryComponent* SrcComponent);

	UFUNCTION(BlueprintCallable)
	static void Static_PushPrivateItemId(const UWorld* World,
	                                     const UActorInventoryComponent* SrcComponent,
	                                     const int32 NewPrivateItemId);
	
protected:
	static UAutomatedTestInventoryValidationManager* Get(const UWorld* World);
	void RegisterComponent(const UActorInventoryComponent* SrcComponent);
	void UnregisterComponent(const UActorInventoryComponent* SrcComponent);
	void PushPrivateItemId(const UActorInventoryComponent* SrcComponent, const int32 NewPrivateItemId);
	
	UPROPERTY(Transient)
	TMap<TWeakObjectPtr<const UActorInventoryComponent>, FInventoryValidationMechanism> ValidationMechanisms;
};
