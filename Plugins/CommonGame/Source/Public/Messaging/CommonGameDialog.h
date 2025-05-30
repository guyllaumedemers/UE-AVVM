// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonActivatableWidget.h"
#include "CommonMessagingSubsystem.h"

#include "CommonGameDialog.generated.h"

USTRUCT(BlueprintType)
struct FConfirmationDialogAction
{
	GENERATED_BODY()

public:
	/** Required: The dialog option to provide. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECommonMessagingResult Result = ECommonMessagingResult::Unknown;

	/** Optional: Display Text to use instead of the action name associated with the result. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText OptionalDisplayText;

	bool operator==(const FConfirmationDialogAction& Other) const
	{
		return Result == Other.Result &&
			OptionalDisplayText.EqualTo(Other.OptionalDisplayText);
	}
};

// @gdemers BEGIN-CHANGE
// UCLASS()
UCLASS(BlueprintType, Blueprintable)
// @gdemers BEGIN-END
class COMMONGAME_API UCommonGameDialogDescriptor : public UObject
{
	GENERATED_BODY()
	
public:
	static UCommonGameDialogDescriptor* CreateConfirmationOk(const FText& Header, const FText& Body);
	static UCommonGameDialogDescriptor* CreateConfirmationOkCancel(const FText& Header, const FText& Body);
	static UCommonGameDialogDescriptor* CreateConfirmationYesNo(const FText& Header, const FText& Body);
	static UCommonGameDialogDescriptor* CreateConfirmationYesNoCancel(const FText& Header, const FText& Body);

public:
	/** The header of the message to display */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Header;
	
	/** The body of the message to display */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Body;

	/** The confirm button's input action to use. */
	// @gdemers BEGIN-CHANGE
	// UPROPERTY(BlueprintReadWrite)
	// TArray<FConfirmationDialogAction> ButtonActions;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FConfirmationDialogAction> ButtonActions;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class UCommonGameDialog> WidgetClass = nullptr;
	// @gdemers BEGIN-END
};


UCLASS(Abstract)
class COMMONGAME_API UCommonGameDialog : public UCommonActivatableWidget
{
	GENERATED_BODY()
	
public:
	UCommonGameDialog();
	
	virtual void SetupDialog(UCommonGameDialogDescriptor* Descriptor, FCommonMessagingResultDelegate ResultCallback);

	virtual void KillDialog();
	
	// @gdemers BEGIN-CHANGE
protected:
	UFUNCTION(BlueprintImplementableEvent)
	void BP_SetupDialog(UCommonGameDialogDescriptor* Descriptor);
	// @gdemers BEGIN-END
};
