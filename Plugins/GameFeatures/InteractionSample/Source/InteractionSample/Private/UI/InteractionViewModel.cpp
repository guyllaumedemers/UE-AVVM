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
#include "InputAction.h"
#include "Data/InteractionExecutionRequirements.h"

FInputProgress::FInputProgress(const FAVVMHandshakePayload* NewPayload)
{
	if (NewPayload == nullptr)
	{
		return;
	}

	const auto* Instigator = UActorInteractionComponent::GetActorComponent(NewPayload->Instigator.Get());
	if (!IsValid(Instigator))
	{
		return;
	}

	TInstancedStruct<FInteractionExecutionRequirements> OutRequirements;
	Instigator->GetInteractionRequirements(OutRequirements);

	const auto* FloatRequirements = OutRequirements.GetPtr<FInteractionExecutionFloatRequirements>();
	if (FloatRequirements != nullptr)
	{
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

	UE_LOG(LogUI,
	       Log,
	       TEXT("New \"%s\" Interaction Recorded. Interaction through Input Action \"%s\". Has Max Threshold \"%s\"."),
	       InteractionType.GetData(),
	       InputAction.IsValid() ? *InputAction->GetName() : TEXT("InputAction Unknown"),
	       *FString::SanitizeFloat(CompletionThreshold, 2));
#endif
}

bool FInputProgress::operator==(const FInputProgress& Rhs) const
{
	return (InputAction == Rhs.InputAction)
			&& (bRequireInputHolding == Rhs.bRequireInputHolding)
			&& (bRequireInputMashing == Rhs.bRequireInputMashing)
			&& (InputProgress == Rhs.InputProgress)
			&& (CompletionThreshold == Rhs.CompletionThreshold);
}

void FInputProgress::Tick(const float NewDelta)
{
	InputProgress = FMath::Clamp(InputProgress + NewDelta, 0.f, CompletionThreshold);

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

	UE_LOG(LogUI,
	       Log,
	       TEXT("\"%s\" Ticked. Progress: \"%s\"/\"%s\"."),
	       InteractionType.GetData(),
	       *FString::SanitizeFloat(InputProgress, 2),
	       *FString::SanitizeFloat(CompletionThreshold, 2));
#endif
}

void UInteractionViewModel::SetPayload(const TInstancedStruct<FAVVMNotificationPayload>& NewPayload)
{
	UE_MVVM_SET_PROPERTY_VALUE(InputContext,
	                           FInputProgress(NewPayload.GetPtr<FAVVMHandshakePayload>()));
}

void UInteractionViewModel::OnInputEvent(const float NewDelta)
{
	InputContext.Tick(NewDelta);
}
