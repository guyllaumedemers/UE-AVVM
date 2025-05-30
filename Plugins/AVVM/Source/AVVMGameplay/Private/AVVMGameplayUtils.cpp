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

FStringView UAVVMGameplayUtils::PrintIsLocallyControlled(const AActor* Actor)
{
	return UAVVMGameplayUtils::IsLocallyControlled(Actor) ? TEXT("True") : TEXT("False");
}

FStringView UAVVMGameplayUtils::PrintIsServerOrClient(const AActor* Actor)
{
	return UAVVMGameplayUtils::IsExecutingFromServerOrClient(Actor) ? TEXT("Server") : TEXT("Client");
}

bool UAVVMGameplayUtils::IsExecutingFromServerOrClient(const AActor* Actor)
{
	if (!ensureAlwaysMsgf(IsValid(Actor), TEXT("Invalid Actor!")))
	{
		return false;
	}

	const ENetMode NetMode = Actor->GetNetMode();
	if (NetMode == NM_DedicatedServer || NetMode == NM_ListenServer || NetMode || NetMode == NM_Standalone)
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
