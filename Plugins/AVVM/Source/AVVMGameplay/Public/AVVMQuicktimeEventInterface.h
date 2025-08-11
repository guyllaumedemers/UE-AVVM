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

#include "AVVMGameplay.h"
#include "AVVMNotificationSubsystem.h"
#include "UObject/Interface.h"

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

	UPROPERTY(Transient, BlueprintReadWrite)
	TWeakObjectPtr<const AActor> Caller = nullptr;

	UPROPERTY(Transient, BlueprintReadWrite)
	EAVVMQuicktimeEventRequestType RequestType = EAVVMQuicktimeEventRequestType::None;

	UPROPERTY(Transient, BlueprintReadWrite)
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
	virtual void Disconnect()
	{
		UE_LOG(LogGameplay, Log, TEXT("Disconnecting."));
	};

	virtual void Connect()
	{
		UE_LOG(LogGameplay, Log, TEXT("Connecting."));
	};

	virtual void Win()
	{
		UE_LOG(LogGameplay, Log, TEXT("Game Won."));
	};

	virtual void Lose()
	{
		UE_LOG(LogGameplay, Log, TEXT("Game Lost."));
	};

	virtual void Kill()
	{
		// @gdemers Post-Death replication. the owned AInfo update its replicated properties. OnRep_, 'this' call, TScriptInterface<ThisClass>(SomeGameState)->Kill is invoked.
		// Through our GameState, we can notify the relevant presenters of a player death events on all remote players.
		UE_LOG(LogGameplay, Log, TEXT("Kill."));
	};

	virtual void Killstreak()
	{
		UE_LOG(LogGameplay, Log, TEXT("Killstreak."));
	};

	virtual void CaptureObjective()
	{
		UE_LOG(LogGameplay, Log, TEXT("Capturing new Objective."));
	};

	virtual void DiscoverArea()
	{
		// @gdemers same principles can be applied here. a player could enter an area, consume a token which would no longer make the area "non-discovered", and push the information
		// of the area to the AInfo of the game state so whenever a player actually discover the area, all players gets notified OnRep_.
		UE_LOG(LogGameplay, Log, TEXT("Discovering new Area."));
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
	virtual void Damage(const UActorComponent* Component)
	{
		UE_LOG(LogGameplay, Log, TEXT("Player Damaged."));
	}

	virtual void Heal(const UActorComponent* Component)
	{
		UE_LOG(LogGameplay, Log, TEXT("Player Healed."));
	}

	virtual void Die(const UActorComponent* Component)
	{
		// @gdemers Example case : Whomever dies, invoke 'this' call, IAVVMQuicktimeEventPlayerStateInterface::Die from the Health component of the player.
		// From the override of the interface, we can execute various actions. we could :
		//	A) fetch the derived game state, add the player to a replicated system that record a queue of players and play a new ui banner (on all clients) when the collection changed. (See IAVVMQuicktimeEventGameStateInterface::Kill)
		//	B) fetch the replicated system held by the derived player state and record the death of the player, incrementing the player death count.
		//	C) fetch the replicated system held by the derived player state and clear any world actors placed/owned by the dead actor so they are cleared (on all clients).
		//	etc...
		// All of this is obviously depended on your project!
		// Additionally, note that we heavily rely on interface dispatch here. It creates separation of concern in modules as they only need to know about "Engine", and "AVVMGameplay".
		UE_LOG(LogGameplay, Log, TEXT("Player Died."));
	}

	virtual void Raise(const UActorComponent* Component)
	{
		UE_LOG(LogGameplay, Log, TEXT("Player Raised."));
	}

	virtual void Stun(const UActorComponent* Component)
	{
		UE_LOG(LogGameplay, Log, TEXT("Player Stunned."));
	}

	virtual void Exhaust(const UActorComponent* Component)
	{
		UE_LOG(LogGameplay, Log, TEXT("Player Exhausted."));
	}

	virtual void StartCasting(const UActorComponent* Component)
	{
		UE_LOG(LogGameplay, Log, TEXT("Player Started Casting."));
	}

	virtual void StopCasting(const UActorComponent* Component)
	{
		UE_LOG(LogGameplay, Log, TEXT("Player Stopped Casting."));
	}

	virtual void EarnMoney(const UActorComponent* Component)
	{
		UE_LOG(LogGameplay, Log, TEXT("Player Earned Money."));
	}

	virtual void SpendMoney(const UActorComponent* Component)
	{
		UE_LOG(LogGameplay, Log, TEXT("Player Spent Money."));
	}

	virtual void StartInteractingWithWorld(const UActorComponent* Component)
	{
		UE_LOG(LogGameplay, Log, TEXT("Player Started Interacting with World."));
	}

	virtual void StopInteractingWithWorld(const UActorComponent* Component)
	{
		UE_LOG(LogGameplay, Log, TEXT("Player Stopped Interacting with World."));
	}

	virtual void ConsumeItem(const UActorComponent* Component)
	{
		UE_LOG(LogGameplay, Log, TEXT("Player Consumed Item."));
	}

	virtual void StartTalkingToNpc(const UActorComponent* Component)
	{
		UE_LOG(LogGameplay, Log, TEXT("Player Started Talking to NPC."));
	}

	virtual void StopTalkingToNpc(const UActorComponent* Component)
	{
		UE_LOG(LogGameplay, Log, TEXT("Player Stopped Talking to NPC."));
	}
};
