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

#include "AVVMAutomatedTestResourceValidationManager.generated.h"

/**
 *	Class description:
 *	
 *	FAVVMNonTSResourceValidationMechanism is a Non thread-safe counter for resource validation on the game thread.
 */
USTRUCT(BlueprintType)
struct AVVMTOOLKIT_API FAVVMNonTSResourceValidationMechanism
{
	GENERATED_BODY()
	
	FAVVMNonTSResourceValidationMechanism();
	
	UPROPERTY(Transient, BlueprintReadWrite)
	int32 RegistryIdRequested = INDEX_NONE;
	
	UPROPERTY(Transient, BlueprintReadWrite)
	int32 RegistryIdLoaded = INDEX_NONE;
	
	UPROPERTY(Transient, BlueprintReadWrite)
	int32 UObjectRequested = INDEX_NONE;
	
	UPROPERTY(Transient, BlueprintReadWrite)
	int32 UObjectLoaded = INDEX_NONE;
};

/**
 *	Class description:
 *	
 *	UAVVMAutomatedTestResourceValidationManager is a class that run validation on resource loading process for various system
 *	during automated testing.
 */
UCLASS()
class AVVMTOOLKIT_API UAVVMAutomatedTestResourceValidationManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static void Static_RegisterComponent(const UWorld* World, const UActorComponent* SrcComponent);
	
	UFUNCTION(BlueprintCallable)
	static void Static_UnregisterComponent(const UWorld* World, const UActorComponent* SrcComponent);
	
	UFUNCTION(BlueprintCallable)
	static void Static_IncrementRegistryIdRequested(const UWorld* World, const UActorComponent* SrcComponent);
	
	UFUNCTION(BlueprintCallable)
	static void Static_IncrementRegistryIdLoaded(const UWorld* World, const UActorComponent* SrcComponent);
	
	UFUNCTION(BlueprintCallable)
	static void Static_IncrementUObjectRequested(const UWorld* World, const UActorComponent* SrcComponent);
	
	UFUNCTION(BlueprintCallable)
	static void Static_IncrementUObjectLoaded(const UWorld* World, const UActorComponent* SrcComponent);
	
	UFUNCTION(BlueprintCallable)
	static bool Static_IsIntegral(const UWorld* World, const UActorComponent* SrcComponent);
	
protected:
	static UAVVMAutomatedTestResourceValidationManager* Get(const UWorld* World);
	void RegisterComponent(const UActorComponent* SrcComponent);
	void UnregisterComponent(const UActorComponent* SrcComponent);
	void IncrementRegistryIdRequested(const UActorComponent* SrcComponent);
	void IncrementRegistryIdLoaded(const UActorComponent* SrcComponent);
	void IncrementUObjectRequested(const UActorComponent* SrcComponent);
	void IncrementUObjectLoaded(const UActorComponent* SrcComponent);
	bool IsIntegral(const UActorComponent* SrcComponent);
	
	UPROPERTY(Transient)
	TMap<TWeakObjectPtr<const UActorComponent>, FAVVMNonTSResourceValidationMechanism> ValidationMechanisms;
};
