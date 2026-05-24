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

#include "ActiveGameplayEffectHandle.h"
#include "GameplayTagContainer.h"
#include "Backend/AVVMDataResolverHelper.h"
#include "Components/ActorComponent.h"
#include "StructUtils/InstancedStruct.h"

#include "ActorSkillTreeComponent.generated.h"

struct FStreamableHandle;
class UAVVMResourceManagerComponent;
class USkillTreeNodeObject;

/**
 *	Class description:
 *	
 *	FSkillTreeDataResolverHelper is a context struct that resolve backend information about an Actor Skill Tree.
 */
USTRUCT(BlueprintType)
struct SKILLSAMPLE_API FSkillTreeDataResolverHelper : public FAVVMDataResolverHelper
{
	GENERATED_BODY()

	virtual TArray<int32> GetElementDependencies(const UObject* Outer, const int32 ElementId) const override;
};

/**
 *	Class description:
 *
 *	FSkillTreeNodeToken describe a unique identifier that increment only when default construct. Can be safely
 *	passed by copy around.
 */
USTRUCT(BlueprintType)
struct SKILLSAMPLE_API FSkillTreeNodeToken
{
	GENERATED_BODY()

	static FSkillTreeNodeToken MakeToken()
	{
		static uint32 GlobalUniqueId = 0;

		FSkillTreeNodeToken NewToken;
		NewToken.UniqueId = ++GlobalUniqueId;

		return NewToken;
	}

	UPROPERTY()
	uint32 UniqueId = INDEX_NONE;
};

/**
 *	Class description:
 *	
 *	UActorSkillTreeComponent is a component object that handle granting modifiers based on a list of {FDataRegistryId} provided by its owning outer
 *	following a backend request, or Data Asset referencing. Progression is expected to be supported for USkillTreeNodeObject upgrades!
 *	
 *	Note : This component should exist on Character, and/or weapons, items, etc... any element that require data progression, and reference an ASC
 *	on itself, or it's owning Outer.
 */
UCLASS()
class SKILLSAMPLE_API UActorSkillTreeComponent : public UActorComponent
{
	GENERATED_BODY()

	DECLARE_DELEGATE(FOnAsyncSpawnRequestDeferred);

public:
	UActorSkillTreeComponent(const FObjectInitializer& ObjectInitializer);
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable)
	static UActorSkillTreeComponent* GetActorComponent(const AActor* NewActor);

	UFUNCTION(BlueprintCallable)
	void RequestSkillTree(const AActor* Outer);
	
protected:
	UFUNCTION()
	void OnSkillTreeNodeRetrieved(FSkillTreeNodeToken SkillTreeNodeToken);
	
	UFUNCTION()
	void OnRep_SkillTreeNodeCollectionChanged(const TArray<USkillTreeNodeObject*>& OldSkillTreeNodeObjects);

	void TryApplyGameplayEffect(const AActor* Outer);

	void RequestGameplayEffect(UAVVMResourceManagerComponent* ResourceManagerComponent,
	                           USkillTreeNodeObject* NewSkillTreeNode);

	UFUNCTION()
	void OnGameplayEffectClassRetrieved(const UClass* NewGameplayEffectClass,
	                                    USkillTreeNodeObject* NewSkillTreeNode);

	struct FGameplayEffectSpawnerQueuingMechanism
	{
		~FGameplayEffectSpawnerQueuingMechanism();
		bool PushDeferredItem(USkillTreeNodeObject* NewSkillTreeNode, const UActorSkillTreeComponent::FOnAsyncSpawnRequestDeferred& NewRequest);
		bool TryExecuteNextRequest(const bool bCanDequeueFrontItem = false);
		bool HasPendingRequest() const;
		USkillTreeNodeObject* PeekItem() const;

		TArray<UActorSkillTreeComponent::FOnAsyncSpawnRequestDeferred> PendingSpawnRequests;
		TArray<TWeakObjectPtr<USkillTreeNodeObject>> QueuedSkillTreeNodes;
	};
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	bool bShouldAsyncLoadOnBeginPlay = false;

	UPROPERTY(Transient, BlueprintReadOnly, ReplicatedUsing="OnRep_SkillTreeNodeCollectionChanged")
	TArray<TObjectPtr<USkillTreeNodeObject>> SkillTreeNodes;
	
	UPROPERTY(Transient, BlueprintReadOnly, meta=(ToolTip="GameplayTagContainer that define the state of the Outer Actor. Example : InTutorial, Pre-BossFight-X, etc..."))
	FGameplayTagContainer NonReplicatedComponentStateTags = FGameplayTagContainer::EmptyContainer;

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<const AActor> OwningOuter = nullptr;
	
	TMap<uint32, TSharedPtr<FStreamableHandle>> SkillTreeNodeHandleSystem;
	TSharedPtr<FGameplayEffectSpawnerQueuingMechanism> QueueingMechanism = nullptr;

private:
	void SetupSkillTreeNodeObjects(const TArray<UObject*>& NewResources);
	void SetupSkillTreeNodeEffects(const TArray<UObject*>& NewResources);
	
	// @gdemers Data Resolver for backend representation of an actor skill tree. 
	static const TInstancedStruct<FAVVMDataResolverHelper>& GetSkillTreeDataResolverHelper();

	// @gdemers cached representation of what has been attributed during the initialization
	// phase of our Skill Tree. This address the problem of uniqueness for entries with identical type, and/or owned by different entity.
	// example : Perks/Traits that are allowed to Stack. Or, shotgun +15% damage vs pistol +15% damage.
	TArray<int32> PrivateSkillTreeNodeIds;

	friend class USkillTreeResourceHandlingImpl;
};
