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

#include "CommonActivatableWidget.h"
#include "GameplayTagContainer.h"
#include "Widgets/UIExtensionPointWidget.h"

#include "AVVMHUDWidget.generated.h"

/**
 *	Class description:
 *
 *	UAVVMHUDWidget is a widget class that holds location point on screen (UAVVMHUDSlotWidget) to with we publish STATIC elements. These elements cannot be closed,
 *	moved around or minimized in any way.
 */
UCLASS()
class AVVMGAMEPLAY_API UAVVMHUDWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVisibilityRequirementsChanged, const FGameplayTagContainer&, NewVisibilityRequirements);

public:
	UFUNCTION(BlueprintCallable)
	void SetHUDVisibilityRequirements(const FGameplayTagContainer& NewVisibilityRequirements);

	UPROPERTY(BlueprintAssignable)
	FOnVisibilityRequirementsChanged OnHUDVisibilityRequirementsChanged;
};

/**
 *	Class description:
 *
 *	UAVVMHUDSlotWidget is a UIExntesionPointWidget class that extend the UIExtension plugin api and encapsulate visibility requirements specific
 *	to the owned child widget. During gameplay, user may experience some situation where the HUD elements are required to be hidden. (But maybe not all of them ?)
 *
 *	Example : during cutscene, when being knock down, whenever they are at the HQ where they get a mission or even after all system are initialized, etc...
 *
 *	Handling these cases on a per-feature basis is cumbersome and often becomes very messy (worst when VM are involved!). Using this parenting architecture,
 *	the HUD can easily notify all elements on screen about a change without complex acrobatics.
 *
 *	Simply update the visibility constraint owned by the HUD using a tag and notify all registered slots to compare the new flags against those
 *	held by the HUD slot.
 */
UCLASS()
class AVVMGAMEPLAY_API UAVVMHUDSlotWidget : public UUIExtensionPointWidget
{
	GENERATED_BODY()

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

	UFUNCTION()
	void OnVisibilityRequirementsChanged(const FGameplayTagContainer& NewVisibilityRequirements);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnVisiblityRequired();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnVisiblityBlocked();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	FGameplayTagContainer RequiredTags = FGameplayTagContainer::EmptyContainer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	FGameplayTagContainer BlockingTags = FGameplayTagContainer::EmptyContainer;

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<UAVVMHUDWidget> OwningOuter = nullptr;
};
