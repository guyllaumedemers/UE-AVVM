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
}

void AAVVMGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);
	OnPlayerStateAdded.Broadcast(PlayerState);
}

void AAVVMGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);
	OnPlayerStateRemoved.Broadcast(PlayerState);
}

void AAVVMGameState::HandleMatchIsWaitingToStart()
{
	Super::HandleMatchIsWaitingToStart();
	OnMatchWaitingToStart.ExecuteIfBound();
}

void AAVVMGameState::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
	OnMatchStart.ExecuteIfBound();
}

void AAVVMGameState::HandleMatchHasEnded()
{
	Super::HandleMatchHasEnded();
	OnMatchEnd.ExecuteIfBound();
}

void AAVVMGameState::HandleLeavingMap()
{
	Super::HandleLeavingMap();
	OnLeavingMap.ExecuteIfBound();
}
