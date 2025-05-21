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
// ReSharper disable CppIncompleteSwitchStatement
// ReSharper disable CppDefaultCaseNotHandledInSwitchStatement
#pragma once

#include "CoreMinimal.h"

#include "AVVM.h"
#include "AVVMNotificationSubsystem.h"

#include "AVVMQuicktimeEventInterface.generated.h"

/**
 *	Class description:
 *
 *	EAVVMQuicktimeEventRequestType define a set of possible actions from which a quicktime event can be
 *	invoked from.
 */
UENUM(BlueprintType)
enum class EAVVMQuicktimeEventRequestType : uint8
{
	None,
	Disconnect,
	Connect,
	Win,
	Lose,
	Kill,
	Killstreak,
	CaptureObjective,
	DiscoverArea,
};

inline const TCHAR* EnumToString(EAVVMQuicktimeEventRequestType State)
{
	switch (State)
	{
		case EAVVMQuicktimeEventRequestType::Disconnect:
			return TEXT("Disconnect");
		case EAVVMQuicktimeEventRequestType::Connect:
			return TEXT("Connect");
		case EAVVMQuicktimeEventRequestType::Win:
			return TEXT("Win");
		case EAVVMQuicktimeEventRequestType::Lose:
			return TEXT("Lose");
		case EAVVMQuicktimeEventRequestType::Kill:
			return TEXT("Kill");
		case EAVVMQuicktimeEventRequestType::Killstreak:
			return TEXT("Killstreak");
		case EAVVMQuicktimeEventRequestType::CaptureObjective:
			return TEXT("CaptureObjective");
		case EAVVMQuicktimeEventRequestType::DiscoverArea:
			return TEXT("DiscoverArea");
	}
	ensure(false);
	return TEXT("Unknown");
}

/*
 *	Class description:
 *
 *	FAVVMQuicktimeEvent. gameplay notification request type that forward a payload to a Presenter UObject.
 */
USTRUCT(BlueprintType)
struct AVVMGAMEPLAY_API FAVVMQuicktimeEvent : public FAVVMNotificationPayload
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	AActor* Caller = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	EAVVMQuicktimeEventRequestType RequestType = EAVVMQuicktimeEventRequestType::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TInstancedStruct<FAVVMNotificationPayload> Payload;
};

/**
*	Class description:
 *
*	UAVVMQuicktimeEventInterface. handle Gameplay events and broadcast to the Presenters via the AVVM Notification Subsystem.
 *
 *	Note : Our events are non-recurring, i.e - we don't expect to broadcast OnTick, or multiple times in a short period. Doing so,
 *	would be highly inefficient memory-wise due to TInstancedStruct.
 *
 *	For UAVVMQuicktimeEventGameStateInterface, this is less of a problem as we expect events defined here to be broadcast
 *	infrequently, so the price of a generic payload out weight the flexibility it provides to the system. (However not true for UAVVMQuicktimeEventPlayerStateInterface. See below!)
 */
UINTERFACE(BlueprintType, Blueprintable)
class AVVMGAMEPLAY_API UAVVMQuicktimeEventGameStateInterface : public UInterface
{
	GENERATED_BODY()
};

class AVVMGAMEPLAY_API IAVVMQuicktimeEventGameStateInterface
{
	GENERATED_BODY()

public:
	// @gdemers a player lost connection, who was it ?
	virtual void Disconnect()
	{
		UE_LOG(LogUI, Log, TEXT("Disconnecting."));
	};

	// @gdemers a player gain connection, who was it ?
	virtual void Connect()
	{
		UE_LOG(LogUI, Log, TEXT("Connecting."));
	};

	// @gdemers the game ended and we won! show the After Action Report (if any)
	virtual void Win()
	{
		UE_LOG(LogUI, Log, TEXT("Game Won."));
	};

	// @gdemers the game ended and we lost! show the After Action Report (if any)
	virtual void Lose()
	{
		UE_LOG(LogUI, Log, TEXT("Game Lost."));
	};

	// @gdemers a kill was recorded
	virtual void Kill()
	{
		// @gdemers Post-Player Died. the AInfo (which we updated last), update its replicated properties. OnRep_, 'this' call, TScriptInterface<ThisClass>(SomeGameState)->Kill
		// is invoked. Through our GameState, we can notify the relevant presenters of a player death events on all remote players.
		UE_LOG(LogUI, Log, TEXT("Kill."));
	};

	// @gdemers a killstreak was recorded
	virtual void Killstreak()
	{
		UE_LOG(LogUI, Log, TEXT("Killstreak."));
	};

	// @gdemers an objective was captured
	virtual void CaptureObjective()
	{
		UE_LOG(LogUI, Log, TEXT("Capturing new Objective."));
	};

	// @gdemers an area was discovered
	virtual void DiscoverArea()
	{
		// @gdemers same principles can be applied here. a player could enter an area, consume a token which would no longer make the area "non-discovered", and push the information
		// of the area to the AInfo of the game state so whenever a player actually discover the area, all players gets notified OnRep_.
		UE_LOG(LogUI, Log, TEXT("Discovering new Area."));
	};
};

/*
*	Class description:
 *
 *	UAVVMQuicktimeEventInterface. handle Gameplay events and broadcast to the Presenters via it's implementing Actor.
 *
 *	Note : Our notification system has the overhead of copying TInstancedStruct around when providing Payload information. This may be fine when
 *	running for non-recurring events but in situation where multiple request can be executed sequentially, this may become a bottleneck!
 *
 *	To work around the above issue mentioned, Im currently brainstorming with this idea :
 *
 *		How do we prevent this overhead (we have to bind to delegates for player specific context) ?
 *
 *		if the main module of your project reference AVVM (so to broadcast events), AVVMOnline and AVVM Gameplay to override virtual calls,
 *		then external modules only have to know about the interface (IAVVMQuicktimeEventGameStateInterface) to execute
 *		calls to recurring actions. We dont have to fetch from the main module the project specific Actors types like : AGameMode, AGameState & APlayerState.
 *
 *			Note : This has the benefit of preventing circular dependencies. (so thats a good things already!)
 *
 *		events then become encapsulated by the Actor type which can query the relevant system and fetch latest data (async or sync) before
 *		pushing to the presenters or even receive the Component that run the behaviour directly to access public getters.
 *
 *		That may makes things interesting for recurrent actions like health changed specific to a player ?
 *
 *		example : A player drop health, from the cached ptr to the Player state, we call health change method. We broadcast a delegate, owned by our owning Actor, which our presenter
 *		has registered to via GetOuterKey().
 *
 *		Doing so would imply the following :
 *
 *			- our main project module, which already references ALL modules, will have a reference in the Actor class to the ActorComponent type
 *			- our health module would only know about the AVVM interface. (which would be from AVVMGameplay - most-likely) (it would cast the owning actor type to the interface and cache it)
 *			- our health module would not have to be fetched by a Presenter UObject and bind to, instead we directly bind to our GetOuterKey on the Presenter Class.
 *			- this would also allow multiple presenters to bind to the same delegate without the fetching of the delegate at the component level.
 *			- our Presenter Class doesn't have to know about the external module anymore, just the one with the interface (which define delegates) (this one however imply we have access to delegate getters)
 */
UINTERFACE(BlueprintType, Blueprintable)
class AVVMGAMEPLAY_API UAVVMQuicktimeEventPlayerStateInterface : public UInterface
{
	GENERATED_BODY()
};

class AVVMGAMEPLAY_API IAVVMQuicktimeEventPlayerStateInterface
{
	GENERATED_BODY()

public:
	// @gdemers have owning player state broadcast delegate to child presenter.
	// similar behaviour is expected. in some cases, the behaviour can be more advanced. Like when dying! as you maybe have to notify various systems.
	virtual void Damage(const UActorComponent* Component)
	{
		UE_LOG(LogUI, Log, TEXT("Player Damaged."));
	}

	virtual void Heal(const UActorComponent* Component)
	{
		UE_LOG(LogUI, Log, TEXT("Player Healed."));
	}

	// @gdemers a player died.
	// Note : Most systems will forward their component and be casted in the PlayerState derived type.
	virtual void Die(const UActorComponent* Component)
	{
		// @gdemers Example case : Whomever dies, invoke 'this' call, IAVVMQuicktimeEventPlayerStateInterface::Die from the Health component of the player.
		// From the override of the interface, we can register our newly killed player to the GameState replicated AInfo property. (See IAVVMQuicktimeEventGameStateInterface::kill for whats next!)
		// Note : using dynamic dispatch, we create separation of concerns in the Health Component module as we only need to know about "Engine", and "AVVMGameplay".
		UE_LOG(LogUI, Log, TEXT("Player Died."));
	}

	virtual void Raise(const UActorComponent* Component)
	{
		UE_LOG(LogUI, Log, TEXT("Player Raised."));
	}

	virtual void Stun(const UActorComponent* Component)
	{
		UE_LOG(LogUI, Log, TEXT("Player Stunned."));
	}

	virtual void Exhaust(const UActorComponent* Component)
	{
		UE_LOG(LogUI, Log, TEXT("Player Exhausted."));
	}

	virtual void StartCasting(const UActorComponent* Component)
	{
		UE_LOG(LogUI, Log, TEXT("Player Started Casting."));
	}

	virtual void StopCasting(const UActorComponent* Component)
	{
		UE_LOG(LogUI, Log, TEXT("Player Stopped Casting."));
	}

	// @gdemers most-likely an action that is replicated (so use an AInfo - held by the PlayerState) to all players if currencies
	// are shown in a leaderboard or simply on the HUD
	virtual void EarnMoney(const UActorComponent* Component)
	{
		UE_LOG(LogUI, Log, TEXT("Player Earned Money."));
	}

	virtual void SpendMoney(const UActorComponent* Component)
	{
		UE_LOG(LogUI, Log, TEXT("Player Spent Money."));
	}

	virtual void StartInteractingWithWorld(const UActorComponent* Component)
	{
		UE_LOG(LogUI, Log, TEXT("Player Started Interacting with World."));
	}

	virtual void StopInteractingWithWorld(const UActorComponent* Component)
	{
		UE_LOG(LogUI, Log, TEXT("Player Stopped Interacting with World."));
	}

	virtual void ConsumeItem(const UActorComponent* Component)
	{
		UE_LOG(LogUI, Log, TEXT("Player Consumed Item."));
	}

	virtual void StartTalkingToNpc(const UActorComponent* Component)
	{
		UE_LOG(LogUI, Log, TEXT("Player Started Talking to NPC."));
	}

	virtual void StopTalkingToNpc(const UActorComponent* Component)
	{
		UE_LOG(LogUI, Log, TEXT("Player Stopped Talking to NPC."));
	}
};
