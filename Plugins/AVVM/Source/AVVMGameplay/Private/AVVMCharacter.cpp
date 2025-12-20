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
#include "AVVMCharacter.h"

#include "AVVMGameSession.h"
#include "AVVMNotificationSubsystem.h"
#include "AVVMOnlineInterfaceUtils.h"
#include "AVVMUtils.h"
#include "Ability/AVVMAbilitySystemComponent.h"
#include "Ability/AVVMAbilityUtils.h"
#include "Data/AVVMActorPayload.h"
#include "GameFramework/PlayerState.h"
#include "Resources/AVVMResourceManagerComponent.h"

AAVVMCharacter::AAVVMCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ResourceManagerComponent = ObjectInitializer.CreateDefaultSubobject<UAVVMResourceManagerComponent>(this, TEXT("ResourceManagerComponent"));

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bAllowTickBatching = true;
	PrimaryActorTick.bAllowTickOnDedicatedServer = true;
	SetReplicateMovement(true);
	bReplicates = true;
}

void AAVVMCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void AAVVMCharacter::BeginPlay()
{
	Super::BeginPlay();

	OwningActor = GetPlayerState();

#if WITH_EDITOR
	if (!IsNetMode(NM_DedicatedServer))
#endif
	{
		UE_AVVM_NOTIFY(this,
		               RegisteredChannels.PostCharacterBeginTag,
		               this,
		               FAVVMNotificationPayload::Make<FAVVMActorPayload>(TScriptInterface<const IAVVMCanExposeActorPayload>(this)));
	}
}

void AAVVMCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	OwningActor.Reset();

#if WITH_EDITOR
	if (!IsNetMode(NM_DedicatedServer))
#endif
	{
		UE_AVVM_NOTIFY(this,
		               RegisteredChannels.PostCharacterEndTag,
		               this,
		               FAVVMNotificationPayload::Make<FAVVMActorPayload>(TScriptInterface<const IAVVMCanExposeActorPayload>(this)));
	}
}

UAVVMAbilitySystemComponent* AAVVMCharacter::BP_GetAbilitySystemComponent() const
{
	return Cast<UAVVMAbilitySystemComponent>(GetAbilitySystemComponent());
}

UAbilitySystemComponent* AAVVMCharacter::GetAbilitySystemComponent() const
{
	// @gdemers Note : Override for AI derived class and provide ASC directly.
	return UAVVMAbilityUtils::GetAbilitySystemComponent(OwningActor.Get());
}

TInstancedStruct<FAVVMActorContext> AAVVMCharacter::GetExposedActorContext_Implementation() const
{
	const bool bIsValid = UAVVMUtils::IsNativeScriptInterfaceValid<const IAVVMCanExposeActorPayload>(OwningActor.Get());
	return bIsValid ? IAVVMCanExposeActorPayload::Execute_GetExposedActorContext(OwningActor.Get()) : IAVVMCanExposeActorPayload::GetExposedActorContext_Implementation();
}

void AAVVMCharacter::NotifyAvailableSocketParent(AActor* SocketTarget)
{
	OnParentSocketAvailable.Broadcast(this, SocketTarget);
}

int32 AAVVMCharacter::GetProviderUniqueId_Implementation() const
{
#if WITH_SERVER_CODE
	if (HasAuthority())
	{
		const APlayerState* NewPlayerState = GetPlayerState();
		return AAVVMGameSession::GetUserUniqueId(GetWorld(), NewPlayerState);
	}
	else
#endif
	{
		return INDEX_NONE;
	}
}

TArray<FDataRegistryId> AAVVMCharacter::GetResourceDefinitionResourceIds_Implementation() const
{
	return {ActorDefinitionId};
}

UAVVMResourceManagerComponent* AAVVMCharacter::GetResourceManagerComponent_Implementation() const
{
	return ResourceManagerComponent;
}

void AAVVMCharacter::OnPlayerStateChanged(APlayerState* NewPlayerState, APlayerState* OldPlayerState)
{
	Super::OnPlayerStateChanged(NewPlayerState, OldPlayerState);
	
	// TODO @gdemers if ever server need to configure something based on PlayerState assignment.
}

void AAVVMCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();

#if WITH_EDITOR
	if (!IsNetMode(NM_DedicatedServer))
#endif
	{
		UE_AVVM_NOTIFY(this,
		               RegisteredChannels.PostPlayerControllerReplicationTag,
		               this,
		               FAVVMNotificationPayload::Make<FAVVMActorPayload>(TScriptInterface<const IAVVMCanExposeActorPayload>(this)));
	}
}

void AAVVMCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

#if WITH_EDITOR
	if (!IsNetMode(NM_DedicatedServer))
#endif
	{
		UE_AVVM_NOTIFY(this,
		               RegisteredChannels.PostPlayerStateReplicationTag,
		               this,
		               FAVVMNotificationPayload::Make<FAVVMActorPayload>(TScriptInterface<const IAVVMCanExposeActorPayload>(this)));
	}
}
