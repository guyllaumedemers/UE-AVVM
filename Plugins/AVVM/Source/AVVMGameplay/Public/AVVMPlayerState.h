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
#include "ModularPlayerState.h"
#include "GameFramework/Info.h"

#include "AVVMPlayerState.generated.h"

/**
 *	Class description:
 *
 *	AAVVMPlayerState. Modular Actor that receive a collection of Presenters (Client-Only) dynamically
 *	and define a set of unique events to communicate with them. (See IAVVMQuicktimeEventPlayerStateInterface)
 */
UCLASS()
class AVVMGAMEPLAY_API AAVVMPlayerState : public AModularPlayerState,
                                          public IAVVMQuicktimeEventPlayerStateInterface
{
	GENERATED_BODY()

public:
	AAVVMPlayerState(const FObjectInitializer& ObjectInitializer);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// @gdemers here im just defining example cases of replicated systems that would allow
	// 1. updating a collection of data specific to the system.
	// 2. react post-replication and update presenters owned by the player state.
	// Note : replicated system would be accessed from inside the api defined in : IAVVMQuicktimeEventPlayerStateInterface

	// @gdemers example property of a bank system. each player has it's own bank which
	// handle content that ARE NOT tied to the user account (so not inventory, or profile currencies. so only in-game resources that can be gathered!)
	// note : this could aggregate damage dealt by this user, number of death/raise, etc... ALL types or simply in-game resources gathered.
	UPROPERTY(Transient, BlueprintReadOnly, Replicated)
	TObjectPtr<AInfo> Bank = nullptr;

	// @gdemers example property for any short-time lived, or not, placeable actor. lets say we want to show a visual representation of the level on screen
	// with our visual actors representation. Our local system would fetch all player states from the game state and read from the collection type held by this replicated actors
	// to generate position information about the in-world actors on screen.
	UPROPERTY(Transient, BlueprintReadOnly, Replicated)
	TObjectPtr<AInfo> Pings = nullptr;
};
