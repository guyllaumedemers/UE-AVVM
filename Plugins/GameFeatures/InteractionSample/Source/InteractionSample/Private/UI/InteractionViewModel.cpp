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
#include "UI/InteractionViewModel.h"

#include "ActorInteractionComponent.h"
#include "AVVMGameplayModule.h"
#include "AVVMLogger.h"
#include "CommonInputSubsystem.h"
#include "CommonUITypes.h"
#include "MVVMViewModelBase.h"
#include "Data/InteractionExecutionRequirements.h"
#include "GameFramework/PlayerController.h"

FInputProgress::FInputProgress(const FAVVMHandshakePayload* NewPayload)
{
	if (NewPayload == nullptr)
	{
		return;
	}

	Instigator = NewPayload->Instigator.Get();

	const auto* InteractionComponent = UActorInteractionComponent::GetActorComponent(Instigator.Get());
	if (!IsValid(InteractionComponent))
	{
		return;
	}

	TInstancedStruct<FInteractionExecutionRequirements> OutRequirements;
	InteractionComponent->GetInteractionRequirements(OutRequirements);

	const auto* FloatRequirements = OutRequirements.GetPtr<FInteractionExecutionFloatRequirements>();
	if (FloatRequirements != nullptr)
	{
		const auto* PC = Cast<APlayerController>(NewPayload->Target.Get());
		if (IsValid(PC))
		{
			auto* InputSubsystem = UCommonInputSubsystem::Get(PC->GetLocalPlayer());
			SlateBrush = CommonUI::GetIconForEnhancedInputAction(InputSubsystem, FloatRequirements->InputAction);
		}

		InputAction = FloatRequirements->InputAction;
		bRequireInputHolding = FloatRequirements->bRequireInputHolding;
		bRequireInputMashing = FloatRequirements->bRequireInputMashing;
		CompletionThreshold = FloatRequirements->CompletionThreshold;
	}

#if !UE_BUILD_SHIPPING
	FStringView InteractionType = TEXT("Unknown");
	if (bRequireInputHolding)
	{
		InteractionType = TEXT("Input Holding");
	}
	else if (bRequireInputMashing)
	{
		InteractionType = TEXT("Input Mashing");
	}

	AVVM_LOGGER_LOG(LogGameplay,
	                Instigator.Get(),
	                Instigator.Get(),
	                TEXT("%s Input Action Pressed. Has Max Threshold %s."),
	                *GetNameSafe(InputAction.Get()),
	                *FString::SanitizeFloat(CompletionThreshold, 2));
#endif
}

bool FInputProgress::operator==(const FInputProgress& Rhs) const
{
	return (InputAction == Rhs.InputAction)
			&& (SlateBrush == Rhs.SlateBrush)
			&& (bRequireInputHolding == Rhs.bRequireInputHolding)
			&& (bRequireInputMashing == Rhs.bRequireInputMashing)
			&& (FMath::IsNearlyEqual(CompletionThreshold, Rhs.CompletionThreshold));
}

float FInputProgress::Tick(const float OldProgress, const float NewDelta) const
{
	const float NewProgress = FMath::Clamp(OldProgress + (NewDelta / CompletionThreshold), 0.f, CompletionThreshold);

#if !UE_BUILD_SHIPPING
	FStringView InteractionType = TEXT("Unknown");
	if (bRequireInputHolding)
	{
		InteractionType = TEXT("Input Holding");
	}
	else if (bRequireInputMashing)
	{
		InteractionType = TEXT("Input Mashing");
	}

	AVVM_LOGGER_LOG(LogGameplay,
	                Instigator.Get(),
	                Instigator.Get(),
	                TEXT("%s Input Action Held. Progress %s."),
	                *GetNameSafe(InputAction.Get()),
	                *FString::SanitizeFloat(NewProgress, 2));
#endif

	return NewProgress;
}

void UInteractionViewModel::SetPayload(const TInstancedStruct<FAVVMNotificationPayload>& NewPayload)
{
	UE_MVVM_SET_PROPERTY_VALUE(InputContext,
	                           FInputProgress(NewPayload.GetPtr<FAVVMHandshakePayload>()));
}

void UInteractionViewModel::PumpHeartbeat(const float NewHeartbeat)
{
	if (!FMath::IsNearlyEqual(NewHeartbeat, static_cast<float>(INDEX_NONE)))
	{
		const float NewProgress = InputContext.Tick(InputProgress, NewHeartbeat);
		UE_MVVM_SET_PROPERTY_VALUE(InputProgress, NewProgress);
	}
	else
	{
		Kill();
	}
}

void UInteractionViewModel::Kill()
{
	UE_MVVM_SET_PROPERTY_VALUE(InputProgress, 0.f);
	AVVM_LOGGER_LOG(LogGameplay,
	                InputContext.Instigator.Get(),
	                InputContext.Instigator.Get(),
	                TEXT("%s Input Action Cancelled."),
	                *GetNameSafe(InputContext.InputAction.Get()));
}

void UInteractionViewModel::Execute()
{
	UE_MVVM_SET_PROPERTY_VALUE(InputProgress, 0.f);
	AVVM_LOGGER_LOG(LogGameplay,
	                InputContext.Instigator.Get(),
	                InputContext.Instigator.Get(),
	                TEXT("%s Input Action Executed."),
	                *GetNameSafe(InputContext.InputAction.Get()));
}
