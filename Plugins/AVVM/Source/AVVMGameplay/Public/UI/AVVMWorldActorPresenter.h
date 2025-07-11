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

#include "AVVMNotificationSubsystem.h"
#include "AVVMUIExtensionInterface.h"
#include "Archetypes/AVVMPresenter.h"
#include "Components/SlateWrapperTypes.h"
#include "StructUtils/InstancedStruct.h"

#include "AVVMWorldActorPresenter.generated.h"

class UCommonUserWidget;

/**
 *	Class description:
 *
 *	EAVVMWidgetPreviewType define if the widget should be displayed in world or on HUD.
 */
UENUM(BlueprintType)
enum class EAVVMWidgetPreviewType : uint8
{
	InWorld = 0,
	OnHUD
};

/**
 *	Class description:
 *
 *	UAVVMWorldActorPresenter is a presenter class that allow opening/closing context specific Widgets following world interaction between local client and world actor.
 *	When available, it can display the Widgets in World or on the HUD, then based on user Input, can open the Widget context specific to the owning system.
 */
UCLASS()
class AVVMGAMEPLAY_API UAVVMWorldActorPresenter : public UAVVMPresenter,
                                                  public IAVVMUIExtensionInterface
{
	GENERATED_BODY()

public:
	virtual AActor* GetOuterKey() const override;
	virtual void SafeBeginPlay() override;
	virtual void SafeEndPlay() override;

protected:
	UFUNCTION(BlueprintCallable)
	void BP_OnNotificationReceived_StartPresenter(const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	UFUNCTION(BlueprintCallable)
	void BP_OnNotificationReceived_StopPresenter(const TInstancedStruct<FAVVMNotificationPayload>& Payload);

	virtual void StartPresenting() override;
	virtual void StopPresenting() override;

	void SetupWorldWidget();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EAVVMWidgetPreviewType PreviewType = EAVVMWidgetPreviewType::InWorld;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(EditCondition="PreviewType == EAVVMWidgetPreviewType::InWorld"))
	TSubclassOf<UCommonUserWidget> WorldWidgetClass = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly)
	ESlateVisibility DefaultVisibilityOnStart = ESlateVisibility::SelfHitTestInvisible;

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<UCommonUserWidget> WorldWidget = nullptr;
	
	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<const AActor> OwningOuter = nullptr;
};
