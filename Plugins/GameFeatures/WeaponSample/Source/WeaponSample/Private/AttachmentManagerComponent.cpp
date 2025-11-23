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
#include "AttachmentManagerComponent.h"

#include "AVVMGameplayUtils.h"
#include "TriggeringAttachmentActor.h"
#include "WeaponSample.h"
#include "Ability/AVVMAbilitySystemComponent.h"
#include "Ability/AVVMAbilityUtils.h"
#include "Data/AVVMActorDefinitionDataAsset.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"

void UAttachmentManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	BatchingMechanism = MakeShared<FAttachmentBatchingMechanism>();

	auto* Outer = GetTypedOuter<AActor>();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	OwningOuter = Outer;

	UE_LOG(LogWeaponSample,
	       Log,
	       TEXT("Executed from \"%s\". Adding \"%s\" on Outer \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *UAttachmentManagerComponent::StaticClass()->GetName(),
	       *Outer->GetName());
}

void UAttachmentManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	BatchingMechanism.Reset();

	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	UE_LOG(LogWeaponSample,
	       Log,
	       TEXT("Executed from \"%s\". Removing \"%s\" on Outer \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *UAttachmentManagerComponent::StaticClass()->GetName(),
	       *Outer->GetName());
}

void UAttachmentManagerComponent::Swap_Implementation(const FAttachmentSwapContextArgs& NewAttachmentSwapContext)
{
	TWeakObjectPtr<ATriggeringAttachmentActor>& SearchResult = EquippedAttachments.FindOrAdd(NewAttachmentSwapContext.TargetSlotTag);
	if (SearchResult.IsValid())
	{
		// @gdemers detach old actor from parent.
		SearchResult->Detach();
	}

	if (BatchingMechanism.IsValid())
	{
		// @gdemers add old reference to batch destroy collection.
		BatchingMechanism->PushPendingDestroy(SearchResult.Get());
	}

	// @gdemers update entry reference.
	SearchResult = NewAttachmentSwapContext.Attachment;
	if (SearchResult.IsValid())
	{
		// @gdemers attach new actor to parent.
		SearchResult->Attach(Cast<AActor>(OwningOuter.Get()));
	}
}

void UAttachmentManagerComponent::SetupAttachments(const TArray<UObject*>& NewResources)
{
	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	if (!ensureAlwaysMsgf(!NewResources.IsEmpty(),
	                      TEXT("Attempting to load invalid Attachment set on Outer \"%s\"."),
	                      *Outer->GetName()))
	{
		return;
	}

	const auto* IsServerOrClientString = UAVVMGameplayUtils::PrintNetSource(Outer).GetData();

	TArray<FSoftObjectPath> DeferredItems;
	TArray<FSoftObjectPath> DeferredItemsAttributeSet;
	
	for (const UObject* Resource : NewResources)
	{
		const auto* ActorDefinitionAsset = Cast<UAVVMActorDefinitionDataAsset>(Resource);
		if (!IsValid(ActorDefinitionAsset))
		{
			continue;
		}

		UE_LOG(LogWeaponSample,
		       Log,
		       TEXT("Executed from \"%s\". New \"%s\" Recorded."),
		       IsServerOrClientString,
		       *ActorDefinitionAsset->GetName());

		DeferredItems.Add(ActorDefinitionAsset->GetActorClassSoftObjectPath());
		DeferredItemsAttributeSet.Add(ActorDefinitionAsset->GetActorAttributeSetClassSoftObjectPath());
	}

	if (!DeferredItems.IsEmpty())
	{
		FAttachmentToken Token;
		FStreamableDelegate Callback;
		Callback.BindUObject(this, &UAttachmentManagerComponent::OnAttachmentActorClassRetrieved, Token, DeferredItemsAttributeSet);

		TSharedPtr<FStreamableHandle>& OutResult = AttachmentHandleSystem.FindOrAdd(Token.UniqueId);
		OutResult = UAssetManager::Get().LoadAssetList(DeferredItems, Callback);
	}
}

void UAttachmentManagerComponent::OnAttachmentActorClassRetrieved(FAttachmentToken AttachmentToken, TArray<FSoftObjectPath> AttributeSoftObjectPaths)
{
	const TSharedPtr<FStreamableHandle>* OutResult = AttachmentHandleSystem.Find(AttachmentToken.UniqueId);
	if (!ensure(OutResult != nullptr && OutResult->IsValid()))
	{
		return;
	}

	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Owning Actor invalid!")))
	{
		return;
	}

	TArray<UObject*> OutStreamableAssets;
	(*OutResult)->GetLoadedAssets(OutStreamableAssets);

	if (OutStreamableAssets.IsEmpty())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	FActorSpawnParameters Params;
	Params.Owner = const_cast<AActor*>(Outer);

	auto* NewAttachment = Cast<ATriggeringAttachmentActor>(World->SpawnActor(Cast<UClass>(OutStreamableAssets[0]), &FTransform::Identity, Params));
	if (!ensureAlwaysMsgf(IsValid(NewAttachment), TEXT("Attachment Actor Class Failed to create an instance in World!")))
	{
		return;
	}

	// @gdemers adding attachment AttributeSet initialization based on owning actor creation process.
	// other alternative for this initialization is based on the inventory system, and would imply we consider the attachment
	// a unique element in the inventory system.
	UAVVMAbilitySystemComponent* ASC = UAVVMAbilityUtils::GetAbilitySystemComponent(NewAttachment);
	if (IsValid(ASC) && !AttributeSoftObjectPaths.IsEmpty())
	{
		ASC->SetupAttributeSet(AttributeSoftObjectPaths[0], NewAttachment);
	}

	Swap(FAttachmentSwapContextArgs{NewAttachment, NewAttachment->SlotTag});
}

UAttachmentManagerComponent::FAttachmentBatchingMechanism::~FAttachmentBatchingMechanism()
{
	PendingDestroy.Reset();
}

void UAttachmentManagerComponent::FAttachmentBatchingMechanism::PushPendingDestroy(const TWeakObjectPtr<ATriggeringAttachmentActor>& NewAttachment)
{
	if (NewAttachment.IsValid())
	{
		PendingDestroy.Add(NewAttachment);
	}
}

void UAttachmentManagerComponent::FAttachmentBatchingMechanism::BatchDestroy()
{
	for (auto Iterator = PendingDestroy.CreateIterator(); Iterator; ++Iterator)
	{
		if (Iterator->IsValid())
		{
			Iterator->Get()->Destroy();
			Iterator.RemoveCurrentSwap();
		}
	}
}
