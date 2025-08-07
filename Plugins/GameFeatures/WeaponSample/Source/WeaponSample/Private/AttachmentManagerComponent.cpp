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

#include "AbilitySystemGlobals.h"
#include "AVVMGameplayUtils.h"
#include "TriggeringAttachmentActor.h"
#include "WeaponSample.h"
#include "GameFramework/Actor.h"
#include "Resources/AVVMResourceManagerComponent.h"
#include "Resources/AVVMResourceProvider.h"

void UAttachmentManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	const auto* Outer = GetTypedOuter<AActor>();
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

UAttachmentManagerComponent* UAttachmentManagerComponent::GetActorComponent(const AActor* NewActor)
{
	return IsValid(NewActor) ? NewActor->GetComponentByClass<UAttachmentManagerComponent>() : nullptr;
}

void UAttachmentManagerComponent::Swap_Implementation(const FAttachmentSwapContextArgs& NewAttachmentSwapContext)
{
	// TODO @gdemers Handle slot swapping. Removing a Slot without reassigning should remove
	// all GE from the owning attachment.
}

void UAttachmentManagerComponent::GetAttachmentDefinition(const FGetAttachmentDefinitionRequestArgs& NewRequestArgs)
{
	QueuingMechanism.Push(NewRequestArgs.AttachmentActor);

	// @gdemers racing conditions isnt possible due to how the resource manager component system behave.
	// the above push/pop behaviour is safe.
	auto* ResourceComponent = IAVVMResourceProvider::Execute_GetResourceManagerComponent(OwningOuter.Get());
	if (ensureAlwaysMsgf(IsValid(ResourceComponent),
	                     TEXT("Outer is missing IAVVMResourceProvider::GetResourceManagerComponent impl or return nullptr.")))
	{
		ResourceComponent->RequestAsyncLoading(NewRequestArgs.AttachmentId, {});
	}
}

void UAttachmentManagerComponent::SetupAttachmentModifiers(const TArray<UObject*>& NewResources)
{
	TWeakObjectPtr<ATriggeringAttachmentActor> AttachmentActor = QueuingMechanism.Pop();
	if (AttachmentActor.IsValid())
	{
		// TODO @gdemers Outer isnt the player state that holds the triggerable actor
		// but the triggerable actor itself. maybe a recursive search of parent impose. TBD!
		auto* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwningOuter.Get());
		AttachmentActor->RegisterGameplayEffects(ASC, NewResources);
	}
}

UAttachmentManagerComponent::FAttachmentQueuingMechanism::~FAttachmentQueuingMechanism()
{
	QueuedRequest.Empty();
}

void UAttachmentManagerComponent::FAttachmentQueuingMechanism::Push(const TWeakObjectPtr<ATriggeringAttachmentActor>& NewRequest)
{
	QueuedRequest.Enqueue(NewRequest);
}

TWeakObjectPtr<ATriggeringAttachmentActor> UAttachmentManagerComponent::FAttachmentQueuingMechanism::Pop()
{
	TWeakObjectPtr<ATriggeringAttachmentActor> Out;
	QueuedRequest.Dequeue(Out);
	return Out;
}
