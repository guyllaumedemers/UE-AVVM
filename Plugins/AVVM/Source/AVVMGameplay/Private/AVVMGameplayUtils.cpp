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

#include "AVVMGameplaySettings.h"
#include "Data/AVVMActorDefinitionDataAsset.h"
#include "Data/AVVMActorIdentifierTableRow.h"
#include "Engine/NetConnection.h"
#include "GameFramework/Actor.h"

bool UAVVMGameplayUtils::CheckActorAuthority(const AActor* Actor)
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
	if (NetMode == NM_Standalone)
	{
		return TEXT("[Standalone]");
	}
	else if ((NetMode == NM_ListenServer) || (NetMode == NM_Client))
	{
		const ENetRole RemoteRole = Actor->GetRemoteRole();
		const ENetRole LocalRole = Actor->GetLocalRole();

		const bool bIsRunningActorOnServer = (RemoteRole == ROLE_SimulatedProxy) && (LocalRole == ROLE_Authority);
		if (bIsRunningActorOnServer)
		{
			return TEXT("[Server|Authority]");
		}

		const bool bIsRunningActorOnClientWithoutControl = (RemoteRole == ROLE_Authority) && (LocalRole == ROLE_SimulatedProxy);
		if (bIsRunningActorOnClientWithoutControl)
		{
			return TEXT("[ClientOnServer|NoAuthority]");
		}

		const bool bIsRunningActorClientOnServer = (RemoteRole == ROLE_AutonomousProxy) && (LocalRole == ROLE_Authority);
		if (bIsRunningActorClientOnServer)
		{
			return TEXT("[ClientOnServer|Authority]");
		}

		const bool bIsRunningActorOnClient = (RemoteRole == ROLE_Authority) && (LocalRole == ROLE_AutonomousProxy);
		if (bIsRunningActorOnClient)
		{
			return TEXT("[Client|Authority]");
		}
	}
	else if (NetMode == NM_DedicatedServer)
	{
		return TEXT("[DedicatedServer]");
	}

	return TEXT("[Unknown]");
}

FString UAVVMGameplayUtils::PrintConnectionInfo(const UNetConnection* Connection)
{
	return IsValid(Connection) ? const_cast<UNetConnection*>(Connection)->RemoteAddressToString() : TEXT("Unknown");
}

int32 UAVVMGameplayUtils::GetUniqueIdentifier(const AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return INDEX_NONE;
	}

	const TSoftObjectPtr<UDataTable>& ActorIdentifierDataTable = UAVVMGameplaySettings::GetActorIdentifierDataTable();
	if (ActorIdentifierDataTable.IsNull())
	{
		return INDEX_NONE;
	}

	// @gdemers since entries are TSoftClassPtr themselves, this should be fairly quick to load
	// and not create any hitches during gameplay.
	const UDataTable* DataTable = ActorIdentifierDataTable.LoadSynchronous();
	if (!IsValid(DataTable))
	{
		return INDEX_NONE;
	}

	const UClass* ActorClass = Actor->GetClass();
	if (!IsValid(ActorClass))
	{
		return INDEX_NONE;
	}

	const auto* RowValue = DataTable->FindRow<FAVVMActorIdentifierDataTableRow>(ActorClass->GetFName(), TEXT(""));
	if (ensureAlwaysMsgf(RowValue != nullptr,
	                     TEXT("Invalid Row Entry. Make sure FAVVMActorIdentifierDataTableRow match the Data Table.")))
	{
		return RowValue->UniqueId;
	}
	else
	{
		return INDEX_NONE;
	}
}
