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

#include "BatchingSubsystem.generated.h"

class UBatchingRule;

/**
 *	Class description:
 *
 *	UBatchingSubsystem is a Server Authoritative Subsystem that gather Actors implementing the IBatchable interface,
 *	and that destroy them in sequence.
 *
 *	Mostly used for candidates that require replication based on instantiation, and as such cannot be pooled easily. These
 *	actors are usually expected to remain in world or for given laps of time which can be handled via this system.
 *
 *	Example : Pickups in Multiplayer game.
 */
UCLASS()
class BATCHSAMPLE_API UBatchingSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	UFUNCTION(BlueprintCallable)
	static UBatchingSubsystem* Get(const UWorld* World);

	UFUNCTION(BlueprintCallable)
	void UnRegister(AActor* Actor);

	UFUNCTION(BlueprintCallable)
	void Register(AActor* Actor);

protected:
	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<const UBatchingRule> BatchingRule = nullptr;

	struct FIBSFBatchContext
	{
		// @gdemers default ctor called first, then we reserve size. imply double initialization
		// of properties, but allow for preallocation of collection type to avoid resizing.
		FIBSFBatchContext(AActor* Actors, const int32 MaxSize)
		{
			Candidates.Reserve(MaxSize);
			Candidates.Add(Actors);
		};

		bool DoesQualifyForBatchDestroy(const float MaxSize) const;
		void Obliterate();
		void Push(AActor* Actor);
		void ForceRemove(AActor* Actor);

	private:
		TArray<TWeakObjectPtr<AActor>> Candidates;
	};

	UPROPERTY(Transient, BlueprintReadOnly)
	float Interval = 0.f;

	UPROPERTY(Transient, BlueprintReadOnly)
	float MaxLifetimeAllowedToUndersizeBatch = 0.f;

	UPROPERTY(Transient, BlueprintReadOnly)
	int32 MaxSizePerBatchDestroy = INDEX_NONE;

	UPROPERTY(Transient, BlueprintReadOnly)
	float Timestamp = 0.f;

	TArray<FIBSFBatchContext> PendingDestroy;
};
