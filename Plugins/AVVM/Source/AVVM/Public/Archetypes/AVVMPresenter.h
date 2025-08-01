﻿//Copyright(c) 2025 gdemers
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

#include "GameplayTags.h"
#include "Templates/SubclassOf.h"
#include "UObject/Object.h"

#include "AVVMPresenter.generated.h"

class UCommonUserWidget;
class UMVVMViewModelBase;

/**
 *	Class description:
 *
 *	UAVVMPresenter is a UObject type that bridge the "Gameplay code" and "UI code". It binds and responds to the AVVMNotification system channel tags
 *	to perform actions based on the request received. It defines a Key-Value pair required for mapping an Actor type (i.e it's Outer) to a "Manual" type ViewModel
 *	registered with the AVVMSubsystem.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class AVVM_API UAVVMPresenter : public UObject
{
	GENERATED_BODY()

public:
	virtual void SafeBeginPlay();
	virtual void SafeEndPlay();

protected:
	virtual TSubclassOf<UMVVMViewModelBase> GetViewModelClass() const { return ViewModelClass; };
	virtual AActor* GetOuterKey() const { return GetTypedOuter<AActor>(); }

	UFUNCTION(BlueprintImplementableEvent)
	void BP_RegisterNotificationChannels();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_UnregisterNotificationChannels();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_BroadcastDeferredNotifications();

	virtual void StartPresenting() PURE_VIRTUAL(StartPresenting, return;);
	virtual void StopPresenting() PURE_VIRTUAL(StartPresenting, return;);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ToolTip="GFP may broadcast before the presenter channel is registered. This flag allow OnBeginPlay broadcast for deferred calls."))
	bool bAllowDeferredBroadcast = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(MustImplement="/Script/AVVM.AVVMViewModelFNameHelper"))
	TSubclassOf<UMVVMViewModelBase> ViewModelClass = nullptr;

	// @gdemers widget to be pushed onto the target tag. UCommonActivatableWidget for menus, UCommonUserWidget
	// for HUD elements.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UCommonUserWidget> WidgetClass = nullptr;

	// @gdemers primarylayout.layer.tag or hud.extensionpoint.tag
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag TargetTag = FGameplayTag::EmptyTag;

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<UMVVMViewModelBase> ViewModel = nullptr;

	friend class UAVVMComponent;
	friend class UAVVMSubsystem;
};
