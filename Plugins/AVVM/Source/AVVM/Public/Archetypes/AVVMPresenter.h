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

#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "Templates/SubclassOf.h"
#include "UObject/Object.h"

#include "AVVMPresenter.generated.h"

struct FAVVMNotificationPayload;
class UCommonUserWidget;
class UMVVMViewModelBase;

/**
 *	Class description:
 *
 *	UAVVMObserver register with the UAVVMNotificationSubsystem to be notified of "Gameplay event" through Tag Channels.
 */
UINTERFACE(BlueprintType)
class AVVM_API UAVVMObserver : public UInterface
{
	GENERATED_BODY()
};

class AVVM_API IAVVMObserver
{
	GENERATED_BODY()

public:
	virtual AActor* GetOuterKey() const PURE_VIRTUAL(GetOuterKey, return nullptr;)
	virtual FGameplayTagContainer GetChannelTags() const PURE_VIRTUAL(GetChannelTags, return FGameplayTagContainer::EmptyContainer;);
	virtual bool Broadcast(const FGameplayTag& ChannelTag, const TInstancedStruct<FAVVMNotificationPayload>& Payload) PURE_VIRTUAL(Broadcast, return false;);
};

/**
 *	Class description:
 *
 *	UAVVMPresenter is a UObject type that bridge the "Gameplay code" and "UI code". It's an Abstract Class
 *	that define a Key-Value pair information required for mapping an Actor to a "Manual" type ViewModel on
 *	the UAVVMSubsystem.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class AVVM_API UAVVMPresenter : public UObject,
                                public IAVVMObserver
{
	GENERATED_BODY()

public:
	virtual void PostInitProperties() override;
	virtual void BeginDestroy() override;

	virtual AActor* GetOuterKey() const override { return GetTypedOuter<AActor>(); };
	virtual FGameplayTagContainer GetChannelTags() const override { return ChannelTags; };
	virtual bool Broadcast(const FGameplayTag& ChannelTag, const TInstancedStruct<FAVVMNotificationPayload>& Payload) override;

	virtual TSubclassOf<UMVVMViewModelBase> GetViewModelClass() const { return ViewModelClass; };

protected:
	virtual void StartPresenting() PURE_VIRTUAL(StartPresenting, return;);
	virtual void StopPresenting() PURE_VIRTUAL(StartPresenting, return;);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(MustImplement="/Script/AVVM.AVVMViewModelFNameHelper"))
	TSubclassOf<UMVVMViewModelBase> ViewModelClass = nullptr;

	// @gdemers notification channels to listen
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagContainer ChannelTags = FGameplayTagContainer::EmptyContainer;

	// @gdemers widget to be pushed onto the target tag. UCommonActivatableWidget for menus, UCommonUserWidget
	// for HUD elements.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UCommonUserWidget> WidgetClass = nullptr;

	// @gdemers primarylayout.layer.tag or hud.extensionpoint.tag
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag TargetTag = FGameplayTag::EmptyTag;

	TWeakObjectPtr<UMVVMViewModelBase> ViewModel = nullptr;
};
