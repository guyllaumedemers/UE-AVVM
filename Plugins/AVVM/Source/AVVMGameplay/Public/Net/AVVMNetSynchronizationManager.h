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

#include "AVVMNetSynchronizationManager.generated.h"

class IAVVMDoesImplNetSynchronization;

/**
 *	Class description:
 *	
 *	UAVVMNetSynchronizationManager is a server-sided subsystem handling tracking of replicated system
 *	in their final state.
 *	
 *	Net synchronization is hard to accomplish. Initialization of visuals based on server-client state
 *	often suffer from property replication affected by race conditions, or packet loss. By handling registration of finalized state for replicated
 *	system, we can ensure clients initialization through RPC request.
 */
UCLASS()
class AVVMGAMEPLAY_API UAVVMNetSynchronizationManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	static TArray<TScriptInterface<IAVVMDoesImplNetSynchronization>> Static_GetAllNetFinalized(const UWorld* World);
	static void Static_Register(const UWorld* World, const UActorComponent* ReplicatedComponent);

protected:
	static UAVVMNetSynchronizationManager* Get(const UWorld* World);
	const TArray<TScriptInterface<IAVVMDoesImplNetSynchronization>>& GetAllNetFinalized() const;
	void Register(const UActorComponent* ReplicatedComponent);

	// @gdemers a collection of systems that are fully initialized on the server. ready to RPC
	// to local PC.
	UPROPERTY(Transient, BlueprintReadOnly)
	TArray<TScriptInterface<IAVVMDoesImplNetSynchronization>> NetFinalized;
};
