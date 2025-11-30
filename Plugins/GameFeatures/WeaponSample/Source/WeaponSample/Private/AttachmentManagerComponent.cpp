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

#include "AttachmentActor.h"
#include "AVVMGameplayUtils.h"
#include "TriggeringActor.h"
#include "WeaponSample.h"
#include "Ability/AVVMAbilitySystemComponent.h"
#include "Ability/AVVMAbilityUtils.h"
#include "Data/AVVMActorDefinitionDataAsset.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"

UAttachmentManagerComponent::UAttachmentManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bAllowTickBatching = false;
	PrimaryComponentTick.bAllowTickOnDedicatedServer = false;
	SetIsReplicatedByDefault(false);
}

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
	TWeakObjectPtr<AAttachmentActor> OldAttachment = nullptr;

	TWeakObjectPtr<AAttachmentActor>& SearchResult = EquippedAttachments.FindOrAdd(NewAttachmentSwapContext.SocketName);
	if (SearchResult.IsValid())
	{
		// @gdemers detach old actor from parent.
		OldAttachment = SearchResult;
	}

	// @gdemers update entry reference.
	SearchResult = NewAttachmentSwapContext.Attachment;

	// @gdemers create a context for handling socketing, and initialization of the attachment created.
	FAVVMSocketTargetingDeferralContextArgs ContextArgs;
	ContextArgs.Parent = Cast<AActor>(OwningOuter.Get());
	ContextArgs.SocketName = NewAttachmentSwapContext.SocketName;
	ContextArgs.SrcAttributeSetSoftObjectPath = NewAttachmentSwapContext.SrcAttributeSetSoftObjectPath;

	// @gdemers attach new actor to parent.
	UTriggeringUtils::Swap(OldAttachment.Get(), SearchResult.Get(), ContextArgs);

	if (BatchingMechanism.IsValid())
	{
		// @gdemers add old reference to batch destroy collection.
		BatchingMechanism->PushPendingDestroy(OldAttachment);
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

	auto* NewAttachment = Cast<AAttachmentActor>(World->SpawnActor(Cast<UClass>(OutStreamableAssets[0]), &FTransform::Identity, Params));
	if (!ensureAlwaysMsgf(IsValid(NewAttachment),
	                      TEXT("Attachment Actor Class Failed to create an instance in World!")))
	{
		return;
	}

	// @gdemers handle attachment process
	const FSoftObjectPath AttributeSetSoftObjectPath = !AttributeSoftObjectPaths.IsEmpty() ? AttributeSoftObjectPaths[0] : FSoftObjectPath();
	Swap(FAttachmentSwapContextArgs{NewAttachment, AttributeSetSoftObjectPath, NewAttachment->SocketName});

	// @gdemers adding attachment AttributeSet initialization based on owning actor creation process.
	// other alternative for this initialization is based on the inventory system, and would imply we consider the attachment a unique element in the inventory system.
	UAVVMAbilitySystemComponent* ASC = UAVVMAbilityUtils::GetAbilitySystemComponent(NewAttachment);
	if (IsValid(ASC))
	{
		ASC->SetupAttributeSet(AttributeSetSoftObjectPath, NewAttachment);
	}
}

UAttachmentManagerComponent::FAttachmentBatchingMechanism::~FAttachmentBatchingMechanism()
{
	PendingDestroy.Reset();
}

void UAttachmentManagerComponent::FAttachmentBatchingMechanism::PushPendingDestroy(const TWeakObjectPtr<AAttachmentActor>& NewAttachment)
{
	if (NewAttachment.IsValid())
	{
		PendingDestroy.Add(NewAttachment);
	}
}

void UAttachmentManagerComponent::FAttachmentBatchingMechanism::BatchDestroy()
{
	TArray<TWeakObjectPtr<AAttachmentActor>> OldAttachments = MoveTemp(PendingDestroy);
	for (auto Iterator = OldAttachments.CreateIterator(); Iterator; ++Iterator)
	{
		if (Iterator->IsValid())
		{
			Iterator->Get()->Destroy();
		}
	}
}
