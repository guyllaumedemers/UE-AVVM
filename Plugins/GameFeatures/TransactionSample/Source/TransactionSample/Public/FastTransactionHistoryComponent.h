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

#include "AVVMGameplayTagStackObject.h"
#include "Components/ActorComponent.h"
#include "GameFramework/OnlineReplStructs.h"

#include "FastTransactionHistoryComponent.generated.h"

class AAVVMGameState;
class AAVVMPlayerState;

/**
 *	Class description:
 *
 *	UFastTransactionHistoryComponent is a component system that capture gameplay statistics tied to player state in-play.
 *	
 *	Note : Use this system for transient statistics with simple payload. example : Scoreboard.
 */
UCLASS(ClassGroup=("Transaction"), Blueprintable, meta=(BlueprintSpawnableComponent))
class TRANSACTIONSAMPLE_API UFastTransactionHistoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFastTransactionHistoryComponent(const FObjectInitializer& ObjectInitializer);
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable)
	static const TArray<FAVVMGameplayTagStackObjectFastArray>& Static_GetFastTransactionObjects(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable)
	static FAVVMGameplayTagStackObjectFastArray Static_GetFastLookupFromPlayerState(const UObject* WorldContextObject,
	                                                                                const AAVVMPlayerState* PlayerState);

	UFUNCTION(BlueprintCallable)
	static FAVVMGameplayTagStackObject Static_GetFastLookupTagStateFromPlayerState(const UObject* WorldContextObject,
	                                                                               const AAVVMPlayerState* PlayerState,
	                                                                               const FGameplayTag& Tag);

	static const FAVVMGameplayTagStackObjectFastArray* Static_GetFastLookup(const UObject* WorldContextObject,
	                                                                        const FUniqueNetIdRepl& UniqueNetId);

	static const FAVVMGameplayTagStackObject* Static_GetFastLookupTag(const UObject* WorldContextObject,
	                                                                  const FUniqueNetIdRepl& UniqueNetId,
	                                                                  const FGameplayTag& Tag);

	UFUNCTION(BlueprintCallable)
	static int32 Static_ModifyTransaction(const UObject* WorldContextObject,
	                                      const FUniqueNetIdRepl& PlayerUniqueId,
	                                      const FGameplayTag& Tag,
	                                      const int32 DeltaValue/* +/- */,
	                                      const bool bHasAuthority);

protected:
	static UFastTransactionHistoryComponent* GetActorComponent(const UObject* WorldContextObject);
	const TArray<FAVVMGameplayTagStackObjectFastArray>& GetFastTransactionObjects() const;
	const FAVVMGameplayTagStackObjectFastArray* GetFastLookup(const FUniqueNetIdRepl& UniqueNetId) const;
	const FAVVMGameplayTagStackObject* GetFastLookupTag(const FUniqueNetIdRepl& UniqueNetId, const FGameplayTag& Tag) const;

	int32 ModifyTransaction(const FUniqueNetIdRepl& PlayerUniqueId,
	                        const FGameplayTag& Tag,
	                        const int32 DeltaValue/* +/- */,
	                        const bool bHasAuthority);

	UFUNCTION()
	void OnPlayerArrayResized(AAVVMPlayerState* NewPlayerState,
	                          const bool bExpanded);

	UFUNCTION()
	void OnPlayerUniqueNetIdReady(const FUniqueNetIdRepl& UniqueNetIdRepl);
	
	UFUNCTION()
	void OnRep_OnPerPlayerTransactionsResize();

	// @gdemers manage broadcasting to the AVVMNotificationSubsystem 
	void OnGameplayTagStackObjectChanged(const FAVVMGameplayTagStackObjectFastArray* FastArray, const FGameplayTag& Tag, const int32 NewValue) const;
	void OnGameplayTagStackObjectRemoved(const FAVVMGameplayTagStackObjectFastArray* FastArray, const FGameplayTag& Tag) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bShouldReleasePlayerStatisticsOnEarlyLeave{false};

	UPROPERTY(Transient, BlueprintReadOnly, Replicated)
	TArray<FAVVMGameplayTagStackObjectFastArray> PerPlayerTransactions{};

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<const AAVVMGameState> OwningOuter = nullptr;

	// @gdemers fast access lookup into the replicated array of a player transactions
	TMap<const FAVVMGameplayTagStackObjectFastArray*, const FUniqueNetIdRepl> ReverseFastLookup{};
	TMap<FUniqueNetIdRepl, FAVVMGameplayTagStackObjectFastArray*> FastLookup{};
};
