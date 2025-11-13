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
#include "Data/InteractionExecutionContext.h"

#include "AVVMNotificationSubsystem.h"
#include "Data/AVVMHandshakePayload.h"
#include "Data/AVVMHearbeatPayload.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"

UScriptStruct* TBaseStructure<FInteractionExecutionContext>::Get()
{
	return FInteractionExecutionContext::StaticStruct();
}

void FInteractionExecutionContextAVVMNotify::PumpHeartbeat(const AActor* NewInstigator, const AActor* NewTarget, const float NewDelta) const
{
	const auto* PC = Cast<APlayerController>(NewTarget);
	if (!ensureAlwaysMsgf(IsValid(PC), TEXT("NewTarget doesn't derive from APlayerController!")))
	{
		return;
	}

	UE_AVVM_NOTIFY_IF_PC_LOCALLY_CONTROLLED(NewTarget,
	                                        PumpHeartbeatChannelTag,
	                                        PC,
	                                        NewInstigator,
	                                        FAVVMNotificationPayload::Make<FAVVMHearbeatPayload>(NewDelta));
}

void FInteractionExecutionContextAVVMNotify::Execute(const AActor* NewInstigator, const AActor* NewTarget) const
{
	const auto* PC = Cast<APlayerController>(NewTarget);
	if (!ensureAlwaysMsgf(IsValid(PC), TEXT("NewTarget doesn't derive from APlayerController!")))
	{
		return;
	}

	UE_AVVM_NOTIFY_IF_PC_LOCALLY_CONTROLLED(NewTarget,
	                                        ExecuteChannelTag,
	                                        PC,
	                                        NewInstigator,
	                                        FAVVMNotificationPayload::Make<FAVVMHandshakePayload>(NewInstigator, PC->PlayerState));
}

void FInteractionExecutionContextAVVMNotify::Kill(const AActor* NewInstigator,
                                                  const AActor* NewTarget) const
{
	const auto* PC = Cast<APlayerController>(NewTarget);
	if (!ensureAlwaysMsgf(IsValid(PC), TEXT("NewTarget doesn't derive from APlayerController!")))
	{
		return;
	}

	UE_AVVM_NOTIFY_IF_PC_LOCALLY_CONTROLLED(NewTarget,
	                                        KillChannelTag,
	                                        PC,
	                                        NewInstigator,
	                                        FAVVMNotificationPayload::Make<FAVVMHearbeatPayload>(static_cast<float>(INDEX_NONE)));
}
