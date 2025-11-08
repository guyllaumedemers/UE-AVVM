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

#include "AVVMGameState.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerStateActionRecordedDelegate, const APlayerState*);

/**
 *	Class description:
 *
*	AAVVMGameState is a GFP Receiver who registers with the GameFeatureFramework and react to GFP actions. It expects to receive system specific components depending on your project needs.
 *	
 *	IMPORTANT : Replicated systems SHOULD BE managed through a component added via GFP. Unreal's USubsystem derived classes CANNOT be replicated! Doing this approach makes for a more modular system
 *	due to component addition and removal no longer requiring hard references in the Actor derived class.
 *
 *	Note : for your UI systems. We expect to push an AVVMComponent with a pre-defined list of UAVVMPresenter class on it! Additionally, by implementing IAVVMQuicktimeEventGameStateInterface base
 *	api, you will be able to forward gameplay information without creating dependencies between modules. However, it comes with a price due to interface dispatch!
 */
UCLASS()
class AVVMGAMEPLAY_API AAVVMGameState : public AModularGameState,
                                        public IAVVMQuicktimeEventGameStateInterface

{
	GENERATED_BODY()

public:
	AAVVMGameState(const FObjectInitializer& ObjectInitializer);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;

	// @gdemers those are replicated call.
	virtual void HandleMatchIsWaitingToStart() override;
	virtual void HandleMatchHasStarted() override;
	virtual void HandleMatchHasEnded() override;
	virtual void HandleLeavingMap() override;

	FOnPlayerStateActionRecordedDelegate OnPlayerStateRemoved;
	FOnPlayerStateActionRecordedDelegate OnPlayerStateAdded;
	FSimpleDelegate OnMatchWaitingToStart;
	FSimpleDelegate OnMatchStart;
	FSimpleDelegate OnMatchEnd;
	FSimpleDelegate OnLeavingMap;
};
