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
#include "AVVMPlayerState.h"

#include "Ability/AVVMAbilitySystemComponent.h"

FAVVMPlayerStatePayload::FAVVMPlayerStatePayload(const APlayerState* NewPlayerState,
                                                 const bool bNewAddOrRemove)
	: PlayerState(NewPlayerState), bWasAddedOrRemoved(bNewAddOrRemove)
{
}

AAVVMPlayerState::AAVVMPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UAVVMAbilitySystemComponent>(this, TEXT("ASC"));
	
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.bAllowTickBatching = false;
	PrimaryActorTick.bAllowTickOnDedicatedServer = false;
	// TODO @gdemers PC movement are replicated, but disabling this may cause issue later. be aware!
	SetReplicateMovement(false);
	bReplicates = true;
}

void AAVVMPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void AAVVMPlayerState::BeginPlay()
{
	Super::BeginPlay();
}

void AAVVMPlayerState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AAVVMPlayerState::ClientInitialize(class AController* C)
{
	Super::ClientInitialize(C);

	// @gdemers ASC has this issue on Client PIE where the ActorInfo initialize before the Owner of the Player State
	// is set. This prevent local predicted Abilities from executing.
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (IsValid(ASC))
	{
		ASC->InitAbilityActorInfo(this, this);
	}

	if (!IsNetMode(NM_DedicatedServer))
	{
		const auto PayloadOwner = TScriptInterface<const IAVVMCanExposeActorPayload>(this);
		UE_AVVM_NOTIFY(this, RegisteredChannels.PostPlayerControllerClientInitializedTag, this, FAVVMNotificationPayload::Make<FAVVMActorPayload>(PayloadOwner));
	}
}

UAbilitySystemComponent* AAVVMPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

TInstancedStruct<FAVVMActorContext> AAVVMPlayerState::GetExposedActorContext_Implementation() const
{
	// @gdemers Define the PlayerState representation for it's Actor Context.
	// example : GamerTag, level, ranking, etc... (such as Nameplate information).
	return IAVVMCanExposeActorPayload::GetExposedActorContext_Implementation();
}
