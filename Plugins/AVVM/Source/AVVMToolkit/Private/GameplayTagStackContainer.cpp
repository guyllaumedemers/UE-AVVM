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

#include "GameplayTagStackContainer.h"

#include "AVVMLogger.h"
// #include "AVVMNotificationSubsystem.h"
#include "AVVMToolkitModule.h"
#include "Engine/Engine.h"

FAVVMGameplayTagStackObject::FAVVMGameplayTagStackObject(const FGameplayTag& NewTag,
                                                         const int32 NewReplicatedStackCount,
                                                         const int32 NewPredictedStackCount)
{
}

bool FAVVMGameplayTagStackObject::operator==(const FAVVMGameplayTagStackObject& Rhs) const
{
	return (Tag == Rhs.Tag) &&
			(ReplicatedStackCount == Rhs.ReplicatedStackCount) &&
			(PredictedStackCount == Rhs.PredictedStackCount);
}

void FAVVMGameplayTagStackObject::PostReplicatedAdd(const struct FFastArraySerializer& InArraySerializer)
{
	AVVM_LOGGER_LOG(LogToolkit,
	                GEngine,
	                GEngine,
	                TEXT("New GameplayStack Detected! \r\n Value: %d."),
	                ReplicatedStackCount);

	// FAVVMNotificationContextArgs ContextArgs;
	// ContextArgs.ChannelTag = TAG_TRANSACTION_NOTIFICATION;
	// ContextArgs.Payload = {};
	// ContextArgs.Target = nullptr;
	//
	// UAVVMNotificationSubsystem::Static_BroadcastChannel(GEngine, ContextArgs);
}

void FAVVMGameplayTagStackObject::PostReplicatedChange(const struct FFastArraySerializer& InArraySerializer)
{
	AVVM_LOGGER_LOG(LogToolkit,
	                GEngine,
	                GEngine,
	                TEXT("GameplayStack Change Detected! \r\n Value: %d."),
	                ReplicatedStackCount);

	// FAVVMNotificationContextArgs ContextArgs;
	// ContextArgs.ChannelTag = TAG_TRANSACTION_NOTIFICATION;
	// ContextArgs.Payload = UTransactionObjectUtils::GetValue(*this);
	// ContextArgs.Target = nullptr;
	//
	// UAVVMNotificationSubsystem::Static_BroadcastChannel(GEngine, ContextArgs);
}
