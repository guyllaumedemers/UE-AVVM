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

#include "AVVM.h"
#include "MVVMViewModelBase.h"
#include "Data/AVVMHandshakePayload.h"

#include "InteractionViewModel.generated.h"

class UInputAction;

/**
 *	Class description:
 *
 *	FInputProgress is a struct that cache context information specific to an input event.
 */
USTRUCT(BlueprintType)
struct INTERACTIONSAMPLE_API FInputProgress
{
	GENERATED_BODY()

	FInputProgress() = default;
	FInputProgress(const FAVVMHandshakePayload* NewPayload);
	bool operator==(const FInputProgress& Rhs) const;

	float Tick(const float OldProgress, const float NewDelta) const;

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<UInputAction> InputAction = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly)
	FSlateBrush SlateBrush = FSlateBrush();

	UPROPERTY(Transient, BlueprintReadOnly)
	bool bRequireInputHolding = false;

	UPROPERTY(Transient, BlueprintReadOnly)
	bool bRequireInputMashing = false;

	UPROPERTY(Transient, BlueprintReadOnly)
	float CompletionThreshold = 0.f;
};

/**
 *	Class description:
 *
 *	UInteractionViewModel is a view model type that provide ui information between two end point user interacting with each other.
 *
 *	Example : Hold time.
 */
UCLASS(BlueprintType, Blueprintable)
class INTERACTIONSAMPLE_API UInteractionViewModel : public UMVVMViewModelBase,
                                                    public IAVVMViewModelFNameHelper
{
	GENERATED_BODY()

public:
	virtual FName GetViewModelFName() const override { return TEXT("UInteractionViewModel"); };
	virtual void SetPayload(const TInstancedStruct<FAVVMNotificationPayload>& NewPayload);
	virtual void PumpHeartbeat(const float NewHeartbeat);
	virtual void Execute();
	virtual void Kill();

protected:
	UPROPERTY(Transient, BlueprintReadOnly, FieldNotify)
	FInputProgress InputContext = FInputProgress();

	UPROPERTY(Transient, BlueprintReadOnly, FieldNotify)
	float InputProgress = 0.f;
};
