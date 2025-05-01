// Copyright Epic Games, Inc. All Rights Reserved.

#include "Messaging/CommonMessagingSubsystem.h"

// @gdemers BEGIN-CHANGE
#include "PrimaryGameLayout.h"
#include "Messaging/CommonGameDialog.h"
// @gdemers BEGIN-END
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "UObject/UObjectHash.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CommonMessagingSubsystem)

class FSubsystemCollectionBase;
class UClass;

void UCommonMessagingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UCommonMessagingSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

bool UCommonMessagingSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	UGameInstance* GameInstance = CastChecked<ULocalPlayer>(Outer)->GetGameInstance();
	if (GameInstance && !GameInstance->IsDedicatedServerInstance())
	{
		TArray<UClass*> ChildClasses;
		GetDerivedClasses(GetClass(), ChildClasses, false);

		// Only create an instance if there is no override implementation defined elsewhere
		return ChildClasses.Num() == 0;
	}

	return false;
}

void UCommonMessagingSubsystem::ShowConfirmation(UCommonGameDialogDescriptor* DialogDescriptor, FCommonMessagingResultDelegate ResultCallback)
{
	// @gdemers BEGIN-CHANGE
	const auto Callback = [Descriptor = TWeakObjectPtr(DialogDescriptor)](EAsyncWidgetLayerState State,
	                                                                      UCommonActivatableWidget* ActivatableWidget)
	{
		auto* DialogPrompt = Cast<UCommonGameDialog>(ActivatableWidget);
		if (IsValid(DialogPrompt) && Descriptor.IsValid())
		{
			DialogPrompt->SetupDialog(Descriptor.Get(), {});
		}
		else
		{
			UPrimaryGameLayout* GameLayout = UPrimaryGameLayout::GetPrimaryGameLayoutForPrimaryPlayer(ActivatableWidget);
			if (ensure(IsValid(GameLayout)))
			{
				GameLayout->FindAndRemoveWidgetFromLayer(ActivatableWidget);
			}
		}
	};
	
	UPrimaryGameLayout* GameLayout = UPrimaryGameLayout::GetPrimaryGameLayoutForPrimaryPlayer(this);
	if (ensure(IsValid(GameLayout)) && IsValid(DialogDescriptor))
	{
		// @gdemers adding hard reference to primaryGameLayout Prompt layer to prevent module dependency change
		GameLayout->PushWidgetToLayerStackAsync<UCommonActivatableWidget>(FGameplayTag::RequestGameplayTag(TEXT("UI.PrimaryGameLayout.Prompt")),
		                                                                  true,
		                                                                  DialogDescriptor->WidgetClass.Get(),
		                                                                  Callback);
	}
	// @gdemers BEGIN-END
}

void UCommonMessagingSubsystem::ShowError(UCommonGameDialogDescriptor* DialogDescriptor, FCommonMessagingResultDelegate ResultCallback)
{
	
}
