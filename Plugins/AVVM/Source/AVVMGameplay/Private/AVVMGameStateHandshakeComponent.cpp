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
#include "AVVMGameStateHandshakeComponent.h"

#include "AVVMGameplay.h"
#include "AVVMGameplayUtils.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "ProfilingDebugging/CountersTrace.h"

TRACE_DECLARE_INT_COUNTER(UAVVMGameStateHandshakeComponent_InstanceCounter, TEXT("GameState Handshake Component Instance Counter"));

void UAVVMGameStateHandshakeComponent::BeginPlay()
{
	Super::BeginPlay();

	const auto* Outer = GetTypedOuter<AActor>();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	TRACE_COUNTER_INCREMENT(UAVVMGameStateHandshakeComponent_InstanceCounter);

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Adding \"%s\" Class Instance to Outer \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *UAVVMGameStateHandshakeComponent::StaticClass()->GetName(),
	       *Outer->GetName())

	OwningOuter = Outer;
}

void UAVVMGameStateHandshakeComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	const auto* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Executed from \"%s\". Removing \"%s\" Class Instance to Outer \"%s\"."),
	       UAVVMGameplayUtils::PrintNetSource(Outer).GetData(),
	       *UAVVMGameStateHandshakeComponent::StaticClass()->GetName(),
	       *Outer->GetName())

	OwningOuter.Reset();
}

UAVVMGameStateHandshakeComponent* UAVVMGameStateHandshakeComponent::GetActorComponent(const UObject* WorldContextObject)
{
	const AGameStateBase* GameStateBase = UGameplayStatics::GetGameState(WorldContextObject);
	return IsValid(GameStateBase) ? GameStateBase->GetComponentByClass<UAVVMGameStateHandshakeComponent>() : nullptr;
}

void UAVVMGameStateHandshakeComponent::ProcessHandshake(const TInstancedStruct<FAVVMNotificationPayload>& NewHandshakePayload,
                                                        const FOnHandshakeRequestComplete& NewCallback) const
{
	// TODO @gdemers Add validtion impl details here!
	NewCallback.ExecuteIfBound(true, NewHandshakePayload);
}
