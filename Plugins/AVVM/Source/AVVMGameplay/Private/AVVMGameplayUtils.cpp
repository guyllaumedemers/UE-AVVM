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

bool UAVVMGameplayUtils::IsAuthoritativeActor(const AActor* Actor)
{
	if (!ensureAlwaysMsgf(IsValid(Actor), TEXT("Invalid Actor!")))
	{
		return false;
	}

	const ENetMode NetMode = Actor->GetNetMode();
	if (NetMode == NM_DedicatedServer || NetMode == NM_ListenServer || NetMode == NM_Standalone)
	{
		// @gdemers if HasAuthority return true in PIE, this means the Actor exist on the Server
		// or the authoritative instance of the game.
		return Actor->HasAuthority();
	}

	return false;
}

bool UAVVMGameplayUtils::IsLocallyControlled(const AActor* Actor)
{
	if (ensureAlwaysMsgf(IsValid(Actor), TEXT("Invalid Actor!")))
	{
		const auto* PC = Actor->GetOwner<APlayerController>();
		return IsValid(PC) && PC->IsLocalPlayerController();
	}

	return false;
}

FString UAVVMGameplayUtils::BP_PrintIsLocallyControlled(const AActor* Actor)
{
	return FString{UAVVMGameplayUtils::PrintIsLocallyControlled(Actor)};
}

FString UAVVMGameplayUtils::BP_PrintNetMode(const AActor* Actor)
{
	return FString{UAVVMGameplayUtils::PrintIsLocallyControlled(Actor)};
}

FStringView UAVVMGameplayUtils::PrintIsLocallyControlled(const AActor* Actor)
{
	return UAVVMGameplayUtils::IsLocallyControlled(Actor) ? TEXT("True") : TEXT("False");
}

FStringView UAVVMGameplayUtils::PrintNetMode(const AActor* Actor)
{
	if (!ensureAlwaysMsgf(IsValid(Actor), TEXT("Invalid Actor!")))
	{
		return TEXT("Unknown");
	}

	const ENetMode NetMode = Actor->GetNetMode();
	const bool bIsServerOrClient = UAVVMGameplayUtils::IsAuthoritativeActor(Actor);

	if (NetMode == NM_Standalone)
	{
		return TEXT("NM_Standalone");
	}
	else if (NetMode == NM_ListenServer)
	{
		return bIsServerOrClient ? TEXT("NM_ListenServer") : TEXT("NM_Clients");
	}
	else
	{
		return bIsServerOrClient ? TEXT("NM_DedicatedServer") : TEXT("NM_Clients");
	}
}

FString UAVVMGameplayUtils::PrintConnectionInfo(const UNetConnection* Connection)
{
	return IsValid(Connection) ? const_cast<UNetConnection*>(Connection)->RemoteAddressToString() : TEXT("Unknown");
}
