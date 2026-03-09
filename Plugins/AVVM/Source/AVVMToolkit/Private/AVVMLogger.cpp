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
#include "AVVMLogger.h"

#include "Engine/NetConnection.h"
#include "Subsystems/Subsystem.h"

FString UAVVMLoggerUtils::BP_PrintNetSource(const UObject* NetObject)
{
	return FString{UAVVMLoggerUtils::PrintNetSource(NetObject)};
}

FStringView UAVVMLoggerUtils::PrintNetSource(const UObject* NetObject)
{
	const auto* Actor = Cast<AActor>(NetObject);
	if (!IsValid(Actor))
	{
		const auto* Subsystem = Cast<USubsystem>(NetObject);
		return IsValid(Subsystem) ? TEXT("[Simulated|Subsystem]") : TEXT("[Unknown]");
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

FString UAVVMLoggerUtils::PrintConnectionInfo(const UNetConnection* Connection)
{
	return IsValid(Connection) ? const_cast<UNetConnection*>(Connection)->RemoteAddressToString() : TEXT("Unknown");
}
