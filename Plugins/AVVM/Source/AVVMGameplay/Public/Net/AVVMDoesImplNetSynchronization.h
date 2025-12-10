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
#pragma once

#include "CoreMinimal.h"

#include "UObject/Interface.h"

#include "AVVMDoesImplNetSynchronization.generated.h"

class AAVVMPlayerState;

/**
 *	Class description:
 *	
 *	IAVVMDoesImplNetSynchronization is an interface class that streamline the initialization sequence
 *	of a system that is replicated. As explained in the UAVVMNetSynchronizationManager class description, net synchronization
 *	is a hard topic, and as such proper infrastructure should be put in place to allow clients with bad network
 *	connectivity to always present the correct state of the game.
 *	
 *	Such systems, may initialize once during the gameplay loop, replicate properties, be compared against with the client
 *	local PC/or PlayerState/or Character (which may or may not be accessible at time for any reason), etc...
 *	
 *	With that in mind, we expect systems to register themselves once finalized, ensuring access by latent initialized clients
 *	to such system, and RPC through a force update.
 */
UINTERFACE(BlueprintType)
class AVVMGAMEPLAY_API UAVVMDoesImplNetSynchronization : public UInterface
{
	GENERATED_BODY()
};

class AVVMGAMEPLAY_API IAVVMDoesImplNetSynchronization
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	void Finalized();
	virtual void Finalized_Implementation() PURE_VIRTUAL(Finalized_Implementation, return;);

	UFUNCTION(BlueprintNativeEvent)
	void ClientRefresh(const AAVVMPlayerState* PlayerState) const;
	void ClientRefresh_Implementation(const AAVVMPlayerState* PlayerState) const PURE_VIRTUAL(ClientRefresh_Implementation, return;);
	
	UFUNCTION(BlueprintNativeEvent)
	bool IsNetRelevantForLocalClientOnly() const;
	bool IsNetRelevantForLocalClientOnly_Implementation() const PURE_VIRTUAL(IsNetRelevantForLocalClientOnly_Implementation, return false;);
};
