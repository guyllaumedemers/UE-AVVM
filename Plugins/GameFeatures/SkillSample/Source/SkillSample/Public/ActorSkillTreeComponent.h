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
#include "ExecutionContextParams.h"
#include "ExecutionContextRule.h"
#include "GameplayTagContainer.h"
#include "SkillTreeNodeObject.h"
#include "Backend/AVVMDataResolverHelper.h"
#include "Components/ActorComponent.h"
#include "StructUtils/InstancedStruct.h"

#include "ActorSkillTreeComponent.generated.h"

struct FStreamableHandle;
class UAVVMResourceManagerComponent;

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

public:
	UActorSkillTreeComponent(const FObjectInitializer& ObjectInitializer);
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable)
	static UActorSkillTreeComponent* GetActorComponent(const AActor* NewActor);

	UFUNCTION(BlueprintCallable)
	void RequestSkillTree(const AActor* Outer);

	UFUNCTION(BlueprintCallable)
	void ModifyRuntimeState(const int32 SkillTreeNodeTypeHash,
	                        const FGameplayTagContainer& AddedTags,
	                        const FGameplayTagContainer& RemovedTags);

	UFUNCTION(BlueprintCallable)
	void ModifyRuntimeLevel(const int32 SkillTreeNodeTypeHash,
	                        const int32 NewLevel);

	UFUNCTION(BlueprintCallable)
	void GrantTreeNodeObject(const FSkillTreeNodeObject& NewTreeNodeObject);

	UFUNCTION(BlueprintCallable)
	void RevokeTreeNodeObject(const int32 SkillTreeNodeTypeHash);

protected:
	UFUNCTION()
	void OnSkillTreeNodeRetrieved(FSkillTreeNodeToken SkillTreeNodeToken);

	FActiveGameplayEffectHandle TryApplyGameplayEffect(const UClass* NewGameplayEffectClass,
	                                                   const int32 PrivateTreeNodeId);

	bool CanExecute(const TInstancedStruct<FExecutionContextParams>& Params,
	                const TInstancedStruct<FExecutionContextRule>& Rule) const;

	UFUNCTION(Server, Reliable)
	void Server_GrantTreeNodeObject(const FSkillTreeNodeObject& NewTreeNodeObject);

	UFUNCTION(Server, Reliable)
	void Server_RevokeTreeNodeObject(const int32 SkillTreeNodeTypeHash);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	bool bShouldAsyncLoadOnBeginPlay = false;

	UPROPERTY(Transient)
	TMap<uint32/*FActiveGameplayEffectHandle::GetTypeHash*/, FActiveGameplayEffectHandle> NonReplicatedActiveGameplayEffectHandles;

	UPROPERTY(Transient, BlueprintReadOnly, meta=(ToolTip="GameplayTagContainer that define the state of the Outer Actor. Example : InTutorial, Pre-BossFight-X, etc..."))
	FGameplayTagContainer NonReplicatedComponentStateTags = FGameplayTagContainer::EmptyContainer;

	UPROPERTY(Transient, BlueprintReadOnly, Replicated)
	FSkillTreeNodeObjectFastArray SkillTree;

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<const AActor> OwningOuter = nullptr;

	TMap<uint32, TSharedPtr<FStreamableHandle>> SkillTreeNodeHandleSystem;

private:
	void SetupSkillTreeNodeObjects(const TArray<UObject*>& NewResources);

	// @gdemers Data Resolver for backend representation of an actor skill tree. 
	static const TInstancedStruct<FAVVMDataResolverHelper>& GetSkillTreeDataResolverHelper();

	virtual TInstancedStruct<FExecutionContextRule> GetGrantRule() const;
	virtual TInstancedStruct<FExecutionContextRule> GetRevokeRule() const;

	// @gdemers virtual overrides are available. respect property access modifiers.
	virtual void OnGrant(const FSkillTreeNodeObject& NewTreeNodeObject);
	virtual void OnRevoke(const int32 SkillTreeNodeTypeHash);

	// @gdemers cached representation of what has been attributed during the initialization
	// phase of our Skill Tree. This address the problem of uniqueness for entries with identical type, and/or owned by different entity.
	// example : Perks/Traits that are allowed to Stack. Or, shotgun +15% damage vs pistol +15% damage.
	TArray<int32> PrivateSkillTreeNodeIds;

	friend class USkillTreeResourceHandlingImpl;
};
