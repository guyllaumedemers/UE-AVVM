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

#include "AVVMQuicktimeEventInterface.h"
#include "ModularGameState.h"
#include "GameFramework/Info.h"

#include "AVVMGameState.generated.h"

/**
 *	Class description:
 *
 *	AAVVMGameState. Modular Actor that receive a collection of Presenters (Client-Only) dynamically
 *	and define a set of unique events to communicate with them. (See IAVVMQuicktimeEventGameStateInterface)
 */
UCLASS()
class AVVMGAMEPLAY_API AAVVMGameState : public AModularGameState,
                                        public IAVVMQuicktimeEventGameStateInterface

{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// @gdemers IMPORTANT - ANY SETTER function defined on those systems should be SERVER_AUTHORITATIVE_ONLY. i.e WITH_SERVER
	// and the client side should only be able to use GETTER functions.
	
	// @gdemers here im just defining example cases of replicated systems that would allow
	// 1. updating a collection of data specific to the system.
	// 2. react post-replication and update presenters owned by the game state.

	// @gdemers these systems are queues of data to tracking which entry was recorded last.
	// they are simple 'Recording' systems for pushing sequentially content on the HUD, maybe for a PvP game
	// or COOP game, etc...
	UPROPERTY(Transient, BlueprintReadOnly, Replicated)
	TObjectPtr<AInfo> PlayerDeaths = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly, Replicated)
	TObjectPtr<AInfo> PlayerKillstreaks = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly, Replicated)
	TObjectPtr<AInfo> CapturedObjectives = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly, Replicated)
	TObjectPtr<AInfo> DiscoveredArea = nullptr;
};
