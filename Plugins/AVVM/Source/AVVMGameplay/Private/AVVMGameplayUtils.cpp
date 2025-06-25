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
#include "AVVMGameplayUtils.h"

bool UAVVMGameplayUtils::CheckActorAuthority(const AActor* Actor)
{
	if (!ensureAlwaysMsgf(IsValid(Actor), TEXT("Invalid Actor!")))
	{
		return false;
	}

	const ENetMode NetMode = Actor->GetNetMode();
	if (NetMode == NM_Standalone)
	{
		return true;
	}
	else if ((NetMode == NM_ListenServer) || (NetMode == NM_Client))
	{
		const ENetRole RemoteRole = Actor->GetRemoteRole();
		const ENetRole LocalRole = Actor->GetLocalRole();

		const bool bIsRunningActorOnClientWithoutControl = (RemoteRole == ROLE_Authority) && (LocalRole == ROLE_SimulatedProxy);
		if (bIsRunningActorOnClientWithoutControl)
		{
			return false;
		}

		const bool bIsRuningActorOnClientDuringStartup = (RemoteRole == ROLE_Authority) && (LocalRole == ROLE_AutonomousProxy);
		const bool bIsRuningActorOnClientPostStartup = (RemoteRole == ROLE_AutonomousProxy) && (LocalRole == ROLE_Authority);
		const bool bIsRunningActorOnServer = (RemoteRole == ROLE_SimulatedProxy) && (LocalRole == ROLE_Authority);
		return bIsRuningActorOnClientDuringStartup || bIsRuningActorOnClientPostStartup || bIsRunningActorOnServer;
	}
	else if (NetMode == NM_DedicatedServer)
	{
		return Actor->HasAuthority();
	}

	return false;
}

bool UAVVMGameplayUtils::HasNetworkAuthority(const AActor* Actor)
{
	if (!ensureAlwaysMsgf(IsValid(Actor), TEXT("Invalid Actor!")))
	{
		return false;
	}

	return Actor->HasAuthority();
}

FString UAVVMGameplayUtils::BP_PrintNetSource(const AActor* Actor)
{
	return FString{UAVVMGameplayUtils::PrintNetSource(Actor)};
}

FStringView UAVVMGameplayUtils::PrintNetSource(const AActor* Actor)
{
	if (!ensureAlwaysMsgf(IsValid(Actor), TEXT("Invalid Actor!")))
	{
		return TEXT("[Unknown]");
	}

	const ENetMode NetMode = Actor->GetNetMode();
	if (NetMode & NM_Standalone)
	{
		return TEXT("[Standalone]");
	}
	else if ((NetMode & NM_ListenServer) || (NetMode & NM_Client))
	{
		const ENetRole RemoteRole = Actor->GetRemoteRole();
		const ENetRole LocalRole = Actor->GetLocalRole();

		const bool bIsServerAndClient = (RemoteRole == ROLE_SimulatedProxy) && (LocalRole == ROLE_Authority);
		if (bIsServerAndClient)
		{
			return TEXT("[Server/Client]");
		}

		const bool bIsClientOnServer = (RemoteRole == ROLE_Authority) && (LocalRole == ROLE_SimulatedProxy);
		if (bIsClientOnServer)
		{
			return TEXT("[Client On Server]");
		}

		const bool bDoesMatchClientDuringStartup = (RemoteRole == ROLE_Authority) && (LocalRole == ROLE_AutonomousProxy);
		const bool bDoesMatchClientPostStartup = (RemoteRole == ROLE_AutonomousProxy) && (LocalRole == ROLE_Authority);
		if (bDoesMatchClientDuringStartup || bDoesMatchClientPostStartup)
		{
			return TEXT("[Client]");
		}
	}

	return TEXT("[Server]");
}

FString UAVVMGameplayUtils::PrintConnectionInfo(const UNetConnection* Connection)
{
	return IsValid(Connection) ? const_cast<UNetConnection*>(Connection)->RemoteAddressToString() : TEXT("Unknown");
}
