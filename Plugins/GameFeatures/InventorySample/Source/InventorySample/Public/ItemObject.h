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
// ReSharper disable CppIncompleteSwitchStatement
// ReSharper disable CppDefaultCaseNotHandledInSwitchStatement
#pragma once

#include "CoreMinimal.h"

#include "DataRegistryId.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"

#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#endif // UE_WITH_IRIS

#include "ItemObject.generated.h"

/**
 *	Class description:
 *
 *	FItemStatus is the status of the Item using Tags.
 */
USTRUCT(BlueprintType)
struct INVENTORYSAMPLE_API FItemStatus
{
	GENERATED_BODY()

	// @gdemers Complex state of the item. Example : CanBeConsumed & InBackupStorage & PendingForTrade
	UPROPERTY(Transient, BlueprintReadOnly)
	FGameplayTagContainer RuntimeItemDefinitionTags = FGameplayTagContainer::EmptyContainer;

	UPROPERTY(Transient, BlueprintReadOnly, meta=(ClampMin=0, ClampMax=999))
	int32 Counter = 1;
};

/**
 *	Class description:
 *
 *	FItemLayout is the Item UI layout space.
 */
USTRUCT(BlueprintType)
struct INVENTORYSAMPLE_API FItemLayout
{
	GENERATED_BODY()

	UPROPERTY(Transient, BlueprintReadOnly, meta=(ClampMin=0, ClampMax=999))
	int32 RowHeight = 1;

	UPROPERTY(Transient, BlueprintReadOnly, meta=(ClampMin=0, ClampMax=999))
	int32 ColWidth = 1;
};

/**
 *	Class description:
 *	
 *	UItemObject is any object that exist in your project LORE. Note: When a UItemObject state becomes InWorld, an AActor is instanced on the server
 *	for player pick up.
 *
 *	This object type can be created based on data assets referenced on an AActor Class (holding UAVVMResourceManagerComponent)
 *	or based on a FDataRegistryId collection returned from backend services.
 */
UCLASS(BlueprintType, Blueprintable)
class INVENTORYSAMPLE_API UItemObject : public UObject
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool IsSupportedForNetworking() const override;

#if UE_WITH_IRIS
	virtual void RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;
#endif // UE_WITH_IRIS

	void TrySpawnEquippedItem(const AActor* Target);
	void TrySpawnDroppedItem(const AActor* Target);

protected:
	// @gdemers define a set of tags that default initialize the status of the item when created.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagContainer ItemDefinitionTags = FGameplayTagContainer::EmptyContainer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FDataRegistryId ItemProgressionId = FDataRegistryId();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(InlineEditConditionToggle))
	bool bCanAttachToSocket = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(EditCondition="bCanAttachToSocket"))
	FName SocketName = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(InlineEditConditionToggle))
	bool bSupportGridLayout = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(EditCondition="bSupportGridLayout"))
	FItemLayout ItemLayout = FItemLayout();

	UPROPERTY(Transient, BlueprintReadOnly, Replicated)
	TObjectPtr<AActor> ItemActor = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly, Replicated)
	FItemStatus ItemStatus = FItemStatus();
};
