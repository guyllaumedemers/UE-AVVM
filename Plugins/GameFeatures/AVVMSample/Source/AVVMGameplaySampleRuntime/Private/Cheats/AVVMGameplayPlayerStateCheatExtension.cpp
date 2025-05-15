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
#include "Cheats/AVVMGameplayPlayerStateCheatExtension.h"

#include "AVVM.h"
#include "AVVMGameplaySampleSettings.h"
#include "AVVMQuicktimeEventInterface.h"
#include "AVVMUtilityFunctionLibrary.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"

#if WITH_AVVM_DEBUGGER
#include "Containers/StringFwd.h"
#include <imgui.h>
#endif

void UAVVMGameplayPlayerStateCheatExtension::AddedToCheatManager_Implementation()
{
	UE_LOG(LogUI, Log, TEXT("Registering %s"), *GetName());

#if WITH_AVVM_DEBUGGER
	FAVVMDebuggerModule::Get().GetDebuggerContext().AddDescriptor(this);
#endif
}

void UAVVMGameplayPlayerStateCheatExtension::RemovedFromCheatManager_Implementation()
{
	UE_LOG(LogUI, Log, TEXT("Unregistering %s"), *GetName());

#if WITH_AVVM_DEBUGGER
	FAVVMDebuggerModule::Get().GetDebuggerContext().RemoveDescriptor(this);
#endif
}

void UAVVMGameplayPlayerStateCheatExtension::Die(const int32 PlayerIndex)
{
	APlayerState* PlayerState = UGameplayStatics::GetPlayerState(this, PlayerIndex);
	if (!IsValid(PlayerState))
	{
		return;
	}

	const auto Callback = [](const TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>& QuickTimeEventHandler,
	                         const UActorComponent* ActorComponent)
	{
		if (ensureAlwaysMsgf(UAVVMUtilityFunctionLibrary::IsScriptInterfaceValid(QuickTimeEventHandler),
		                     TEXT("Player State doesn't implement IAVVMQuicktimeEventPlayerStateInterface!")))
		{
			QuickTimeEventHandler->Die(ActorComponent);
		}
	};

	AVVM_EXECUTE_GAMEPLAY_SCOPED_DEBUGLOG(TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>(PlayerState),
	                                      UAVVMGameplaySampleSettings::GetHealthComponentClass(),
	                                      PlayerState->GetPawn(),
	                                      FAVVMOnExecuteDebugEvent::FDelegate::CreateWeakLambda(this, Callback));
}

void UAVVMGameplayPlayerStateCheatExtension::Raise(const int32 PlayerIndex)
{
	APlayerState* PlayerState = UGameplayStatics::GetPlayerState(this, PlayerIndex);
	if (!IsValid(PlayerState))
	{
		return;
	}

	const auto Callback = [](const TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>& QuickTimeEventHandler,
	                         const UActorComponent* ActorComponent)
	{
		if (ensureAlwaysMsgf(UAVVMUtilityFunctionLibrary::IsScriptInterfaceValid(QuickTimeEventHandler),
		                     TEXT("Player State doesn't implement IAVVMQuicktimeEventPlayerStateInterface!")))
		{
			QuickTimeEventHandler->Raise(ActorComponent);
		}
	};

	AVVM_EXECUTE_GAMEPLAY_SCOPED_DEBUGLOG(TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>(PlayerState),
	                                      UAVVMGameplaySampleSettings::GetHealthComponentClass(),
	                                      PlayerState->GetPawn(),
	                                      FAVVMOnExecuteDebugEvent::FDelegate::CreateWeakLambda(this, Callback));
}

void UAVVMGameplayPlayerStateCheatExtension::Stun(const int32 PlayerIndex)
{
	APlayerState* PlayerState = UGameplayStatics::GetPlayerState(this, PlayerIndex);
	if (!IsValid(PlayerState))
	{
		return;
	}

	const auto Callback = [](const TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>& QuickTimeEventHandler,
	                         const UActorComponent* ActorComponent)
	{
		if (ensureAlwaysMsgf(UAVVMUtilityFunctionLibrary::IsScriptInterfaceValid(QuickTimeEventHandler),
		                     TEXT("Player State doesn't implement IAVVMQuicktimeEventPlayerStateInterface!")))
		{
			QuickTimeEventHandler->Stun(ActorComponent);
		}
	};

	AVVM_EXECUTE_GAMEPLAY_SCOPED_DEBUGLOG(TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>(PlayerState),
	                                      UAVVMGameplaySampleSettings::GetStatusEffectComponentClass(),
	                                      PlayerState->GetPawn(),
	                                      FAVVMOnExecuteDebugEvent::FDelegate::CreateWeakLambda(this, Callback));
}

void UAVVMGameplayPlayerStateCheatExtension::Exhaust(const int32 PlayerIndex)
{
	APlayerState* PlayerState = UGameplayStatics::GetPlayerState(this, PlayerIndex);
	if (!IsValid(PlayerState))
	{
		return;
	}

	const auto Callback = [](const TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>& QuickTimeEventHandler,
	                         const UActorComponent* ActorComponent)
	{
		if (ensureAlwaysMsgf(UAVVMUtilityFunctionLibrary::IsScriptInterfaceValid(QuickTimeEventHandler),
		                     TEXT("Player State doesn't implement IAVVMQuicktimeEventPlayerStateInterface!")))
		{
			QuickTimeEventHandler->Exhaust(ActorComponent);
		}
	};

	AVVM_EXECUTE_GAMEPLAY_SCOPED_DEBUGLOG(TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>(PlayerState),
	                                      UAVVMGameplaySampleSettings::GetStaminaComponentClass(),
	                                      PlayerState->GetPawn(),
	                                      FAVVMOnExecuteDebugEvent::FDelegate::CreateWeakLambda(this, Callback));
}

void UAVVMGameplayPlayerStateCheatExtension::StartCasting(const int32 PlayerIndex)
{
	APlayerState* PlayerState = UGameplayStatics::GetPlayerState(this, PlayerIndex);
	if (!IsValid(PlayerState))
	{
		return;
	}

	const auto Callback = [](const TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>& QuickTimeEventHandler,
	                         const UActorComponent* ActorComponent)
	{
		if (ensureAlwaysMsgf(UAVVMUtilityFunctionLibrary::IsScriptInterfaceValid(QuickTimeEventHandler),
		                     TEXT("Player State doesn't implement IAVVMQuicktimeEventPlayerStateInterface!")))
		{
			QuickTimeEventHandler->StartCasting(ActorComponent);
		}
	};

	AVVM_EXECUTE_GAMEPLAY_SCOPED_DEBUGLOG(TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>(PlayerState),
	                                      UAVVMGameplaySampleSettings::GetAbilityComponentClass(),
	                                      PlayerState->GetPawn(),
	                                      FAVVMOnExecuteDebugEvent::FDelegate::CreateWeakLambda(this, Callback));
}

void UAVVMGameplayPlayerStateCheatExtension::StopCasting(const int32 PlayerIndex)
{
	APlayerState* PlayerState = UGameplayStatics::GetPlayerState(this, PlayerIndex);
	if (!IsValid(PlayerState))
	{
		return;
	}

	const auto Callback = [](const TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>& QuickTimeEventHandler,
	                         const UActorComponent* ActorComponent)
	{
		if (ensureAlwaysMsgf(UAVVMUtilityFunctionLibrary::IsScriptInterfaceValid(QuickTimeEventHandler),
		                     TEXT("Player State doesn't implement IAVVMQuicktimeEventPlayerStateInterface!")))
		{
			QuickTimeEventHandler->StopCasting(ActorComponent);
		}
	};

	AVVM_EXECUTE_GAMEPLAY_SCOPED_DEBUGLOG(TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>(PlayerState),
	                                      UAVVMGameplaySampleSettings::GetAbilityComponentClass(),
	                                      PlayerState->GetPawn(),
	                                      FAVVMOnExecuteDebugEvent::FDelegate::CreateWeakLambda(this, Callback));
}

void UAVVMGameplayPlayerStateCheatExtension::EarnMoney(const int32 PlayerIndex)
{
	APlayerState* PlayerState = UGameplayStatics::GetPlayerState(this, PlayerIndex);
	if (!IsValid(PlayerState))
	{
		return;
	}

	const auto Callback = [](const TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>& QuickTimeEventHandler,
	                         const UActorComponent* ActorComponent)
	{
		if (ensureAlwaysMsgf(UAVVMUtilityFunctionLibrary::IsScriptInterfaceValid(QuickTimeEventHandler),
		                     TEXT("Player State doesn't implement IAVVMQuicktimeEventPlayerStateInterface!")))
		{
			QuickTimeEventHandler->EarnMoney(ActorComponent);
		}
	};

	AVVM_EXECUTE_GAMEPLAY_SCOPED_DEBUGLOG(TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>(PlayerState),
	                                      UAVVMGameplaySampleSettings::GetCurrencyComponentClass(),
	                                      PlayerState->GetPawn(),
	                                      FAVVMOnExecuteDebugEvent::FDelegate::CreateWeakLambda(this, Callback));
}

void UAVVMGameplayPlayerStateCheatExtension::SpendMoney(const int32 PlayerIndex)
{
	APlayerState* PlayerState = UGameplayStatics::GetPlayerState(this, PlayerIndex);
	if (!IsValid(PlayerState))
	{
		return;
	}

	const auto Callback = [](const TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>& QuickTimeEventHandler,
	                         const UActorComponent* ActorComponent)
	{
		if (ensureAlwaysMsgf(UAVVMUtilityFunctionLibrary::IsScriptInterfaceValid(QuickTimeEventHandler),
		                     TEXT("Player State doesn't implement IAVVMQuicktimeEventPlayerStateInterface!")))
		{
			QuickTimeEventHandler->SpendMoney(ActorComponent);
		}
	};

	AVVM_EXECUTE_GAMEPLAY_SCOPED_DEBUGLOG(TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>(PlayerState),
	                                      UAVVMGameplaySampleSettings::GetCurrencyComponentClass(),
	                                      PlayerState->GetPawn(),
	                                      FAVVMOnExecuteDebugEvent::FDelegate::CreateWeakLambda(this, Callback));
}

void UAVVMGameplayPlayerStateCheatExtension::StartInteractingWithWorld(const int32 PlayerIndex)
{
	APlayerState* PlayerState = UGameplayStatics::GetPlayerState(this, PlayerIndex);
	if (!IsValid(PlayerState))
	{
		return;
	}

	const auto Callback = [](const TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>& QuickTimeEventHandler,
	                         const UActorComponent* ActorComponent)
	{
		if (ensureAlwaysMsgf(UAVVMUtilityFunctionLibrary::IsScriptInterfaceValid(QuickTimeEventHandler),
		                     TEXT("Player State doesn't implement IAVVMQuicktimeEventPlayerStateInterface!")))
		{
			QuickTimeEventHandler->StartInteractingWithWorld(ActorComponent);
		}
	};

	AVVM_EXECUTE_GAMEPLAY_SCOPED_DEBUGLOG(TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>(PlayerState),
	                                      UAVVMGameplaySampleSettings::GetInteractionComponentClass(),
	                                      PlayerState->GetPawn(),
	                                      FAVVMOnExecuteDebugEvent::FDelegate::CreateWeakLambda(this, Callback));
}

void UAVVMGameplayPlayerStateCheatExtension::StopInteractingWithWorld(const int32 PlayerIndex)
{
	APlayerState* PlayerState = UGameplayStatics::GetPlayerState(this, PlayerIndex);
	if (!IsValid(PlayerState))
	{
		return;
	}

	const auto Callback = [](const TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>& QuickTimeEventHandler,
	                         const UActorComponent* ActorComponent)
	{
		if (ensureAlwaysMsgf(UAVVMUtilityFunctionLibrary::IsScriptInterfaceValid(QuickTimeEventHandler),
		                     TEXT("Player State doesn't implement IAVVMQuicktimeEventPlayerStateInterface!")))
		{
			QuickTimeEventHandler->StopInteractingWithWorld(ActorComponent);
		}
	};

	AVVM_EXECUTE_GAMEPLAY_SCOPED_DEBUGLOG(TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>(PlayerState),
	                                      UAVVMGameplaySampleSettings::GetInteractionComponentClass(),
	                                      PlayerState->GetPawn(),
	                                      FAVVMOnExecuteDebugEvent::FDelegate::CreateWeakLambda(this, Callback));
}

void UAVVMGameplayPlayerStateCheatExtension::ConsumeItem(const int32 PlayerIndex)
{
	APlayerState* PlayerState = UGameplayStatics::GetPlayerState(this, PlayerIndex);
	if (!IsValid(PlayerState))
	{
		return;
	}

	const auto Callback = [](const TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>& QuickTimeEventHandler,
	                         const UActorComponent* ActorComponent)
	{
		if (ensureAlwaysMsgf(UAVVMUtilityFunctionLibrary::IsScriptInterfaceValid(QuickTimeEventHandler),
		                     TEXT("Player State doesn't implement IAVVMQuicktimeEventPlayerStateInterface!")))
		{
			QuickTimeEventHandler->ConsumeItem(ActorComponent);
		}
	};

	AVVM_EXECUTE_GAMEPLAY_SCOPED_DEBUGLOG(TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>(PlayerState),
	                                      UAVVMGameplaySampleSettings::GetInteractionComponentClass(),
	                                      PlayerState->GetPawn(),
	                                      FAVVMOnExecuteDebugEvent::FDelegate::CreateWeakLambda(this, Callback));
}

void UAVVMGameplayPlayerStateCheatExtension::StartTalkingToNpc(const int32 PlayerIndex)
{
	APlayerState* PlayerState = UGameplayStatics::GetPlayerState(this, PlayerIndex);
	if (!IsValid(PlayerState))
	{
		return;
	}

	const auto Callback = [](const TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>& QuickTimeEventHandler,
	                         const UActorComponent* ActorComponent)
	{
		if (ensureAlwaysMsgf(UAVVMUtilityFunctionLibrary::IsScriptInterfaceValid(QuickTimeEventHandler),
		                     TEXT("Player State doesn't implement IAVVMQuicktimeEventPlayerStateInterface!")))
		{
			QuickTimeEventHandler->StartTalkingToNpc(ActorComponent);
		}
	};

	AVVM_EXECUTE_GAMEPLAY_SCOPED_DEBUGLOG(TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>(PlayerState),
	                                      UAVVMGameplaySampleSettings::GetInteractionComponentClass(),
	                                      PlayerState->GetPawn(),
	                                      FAVVMOnExecuteDebugEvent::FDelegate::CreateWeakLambda(this, Callback));
}

void UAVVMGameplayPlayerStateCheatExtension::StopTalkingToNpc(const int32 PlayerIndex)
{
	APlayerState* PlayerState = UGameplayStatics::GetPlayerState(this, PlayerIndex);
	if (!IsValid(PlayerState))
	{
		return;
	}

	const auto Callback = [](const TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>& QuickTimeEventHandler,
	                         const UActorComponent* ActorComponent)
	{
		if (ensureAlwaysMsgf(UAVVMUtilityFunctionLibrary::IsScriptInterfaceValid(QuickTimeEventHandler),
		                     TEXT("Player State doesn't implement IAVVMQuicktimeEventPlayerStateInterface!")))
		{
			QuickTimeEventHandler->StopTalkingToNpc(ActorComponent);
		}
	};

	AVVM_EXECUTE_GAMEPLAY_SCOPED_DEBUGLOG(TScriptInterface<IAVVMQuicktimeEventPlayerStateInterface>(PlayerState),
	                                      UAVVMGameplaySampleSettings::GetInteractionComponentClass(),
	                                      PlayerState->GetPawn(),
	                                      FAVVMOnExecuteDebugEvent::FDelegate::CreateWeakLambda(this, Callback));
}

#if WITH_AVVM_DEBUGGER
void UAVVMGameplayPlayerStateCheatExtension::Draw()
{
}
#endif
