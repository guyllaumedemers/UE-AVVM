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
#include "AVVMGameState.h"

#include "Net/UnrealNetwork.h"

AAVVMGameState::AAVVMGameState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bReplicates = true;
}

void AAVVMGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// @gdemers https://dev.epicgames.com/documentation/en-us/unreal-engine/replicate-actor-properties-in-unreal-engine
	// Replicate Object References
	// Replication of object references is automatically handled by Unreal Engine's replication system.
	// If you have a UObject property that is replicated, references to that object are sent over network connections
	// as a FNetworkGUID that is assigned by the authoritative server. The server then notifies all connected clients of this assignment.

	// @gdemers object reference should be automatically handled by Unreal system according to the above but our console logs complains about the object not
	// being added for replication in this function.
	DOREPLIFETIME(AAVVMGameState, PlayerDeaths);
	DOREPLIFETIME(AAVVMGameState, PlayerKillstreaks);
	DOREPLIFETIME(AAVVMGameState, CapturedObjectives);
	DOREPLIFETIME(AAVVMGameState, DiscoveredArea);
}
