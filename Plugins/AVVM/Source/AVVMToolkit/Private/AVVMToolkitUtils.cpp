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
#include "AVVMToolkitUtils.h"

#include "AVVMViewModelFNameHelper.h"
#include "CommonUserWidget.h"
#include "MVVMSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "View/MVVMView.h"

ULocalPlayer* UAVVMToolkitUtils::GetFirstOrTargetLocalPlayer(const UObject* WorldContextObject)
{
	if (!IsValid(WorldContextObject))
	{
		return nullptr;
	}

	ULocalPlayer* TargetLocalPlayer = UAVVMToolkitUtils::GetTargetLocalPlayer(WorldContextObject);
	if (IsValid(TargetLocalPlayer))
	{
		return TargetLocalPlayer;
	}

	const UWorld* World = WorldContextObject->GetWorld();
	if (!IsValid(World))
	{
		return nullptr;
	}

	const UGameInstance* GameInstance = World->GetGameInstance();
	if (IsValid(GameInstance))
	{
		return GameInstance->GetFirstGamePlayer();
	}
	else
	{
		return nullptr;
	}
}

ULocalPlayer* UAVVMToolkitUtils::GetTargetLocalPlayer(const UObject* WorldContextObject)
{
	auto* PlayerState = Cast<APlayerState>(WorldContextObject);
	if (IsValid(PlayerState))
	{
		return UAVVMToolkitUtils::GetTargetLocalPlayer(PlayerState->GetPlayerController());
	}

	auto* PC = Cast<APlayerController>(WorldContextObject);
	if (IsValid(PC))
	{
		return PC->GetLocalPlayer();
	}

	return nullptr;
}

double UAVVMToolkitUtils::GetServerWorldTime(const UObject* WorldContextObject)
{
	const auto* GameState = UGameplayStatics::GetGameState(WorldContextObject);
	return ensureAlwaysMsgf(IsValid(GameState), TEXT("Missing GameState. Cannot retrieve Server time.")) ? GameState->GetServerWorldTimeSeconds() : 0.f;
}

bool UAVVMToolkitUtils::CheckActorAuthority(const AActor* Actor)
{
	if (!ensureAlwaysMsgf(IsValid(Actor), TEXT("Invalid Actor!")))
	{
		return false;
	}

	const ENetRole RemoteRole = Actor->GetRemoteRole();
	const ENetRole LocalRole = Actor->GetLocalRole();

	const ENetMode NetMode = Actor->GetNetMode();
	if (NetMode == NM_Standalone)
	{
		return true;
	}
	else if ((NetMode == NM_ListenServer) || (NetMode == NM_Client))
	{
		const bool bIsRunningActorOnClientWithoutControl = (RemoteRole == ROLE_Authority) && (LocalRole == ROLE_SimulatedProxy);
		if (bIsRunningActorOnClientWithoutControl)
		{
			return false;
		}

		const bool bIsRunningActorClientOnServer = (RemoteRole == ROLE_AutonomousProxy) && (LocalRole == ROLE_Authority);
		const bool bIsRunningActorOnClient = (RemoteRole == ROLE_Authority) && (LocalRole == ROLE_AutonomousProxy);
		const bool bIsRunningActorOnServer = (RemoteRole == ROLE_SimulatedProxy) && (LocalRole == ROLE_Authority);
		return bIsRunningActorOnClient || bIsRunningActorClientOnServer || bIsRunningActorOnServer;
	}
	else if (NetMode == NM_DedicatedServer)
	{
		return Actor->HasAuthority();
	}

	return false;
}

bool UAVVMToolkitUtils::HasNetworkAuthority(const AActor* Actor)
{
	if (!ensureAlwaysMsgf(IsValid(Actor), TEXT("Invalid Actor!")))
	{
		return false;
	}

	return Actor->HasAuthority();
}

void UAVVMToolkitUtils::BindViewModel(const TScriptInterface<IAVVMViewModelFNameHelper>& ViewModelFNameHelper, UCommonUserWidget* Target)
{
	const bool bIsValid = UAVVMToolkitUtils::IsNativeScriptInterfaceValid<IAVVMViewModelFNameHelper>(ViewModelFNameHelper);
	if (!bIsValid)
	{
		return;
	}

	UMVVMView* MVVMView = UMVVMSubsystem::GetViewFromUserWidget(Target);
	if (IsValid(MVVMView))
	{
		const FName ViewModelFName = ViewModelFNameHelper->GetViewModelFName();
		UObject* ViewModel = ViewModelFNameHelper.GetObject();
		MVVMView->SetViewModel(ViewModelFName, ViewModel);
	}
}
