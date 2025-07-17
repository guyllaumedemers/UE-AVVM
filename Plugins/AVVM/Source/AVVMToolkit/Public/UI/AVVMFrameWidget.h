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

#include "CommonUserWidget.h"
#include "GameplayTagContainer.h"
#include "Components/ContentWidget.h"
#include "Engine/StreamableManager.h"
#include "Templates/SubclassOf.h"

#include "AVVMFrameWidget.generated.h"

class UAVVMFrameBorder;
class UAVVMFrameWidget;
class UAVVMWidgetPickerDataAsset;
class UCommonButtonBase;
class UCommonTextBlock;
class UOverlay;

/**
 *	Class description:
 *
 *	FFrameZOrder is a POD that cache data about the Frame context and the layer on which it lives.
 */
USTRUCT(BlueprintType)
struct AVVMTOOLKIT_API FFrameZOrder
{
	GENERATED_BODY()

	FFrameZOrder() = default;
	explicit FFrameZOrder(UAVVMFrameWidget* NewFrame, const int32 NewZOrder);

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<UAVVMFrameWidget> Frame = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly)
	int32 ZOrder = INDEX_NONE;
};

/**
 *	Class description:
 *
 *	EFrameBitmask is a bitmask system that tracks the state of a Frame object.
 */
UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EFrameBitmask : uint8
{
	None       = 0 UMETA(Hidden),
	Docked     = 1 << 0,
	Minimized  = 1 << 1,
	Closed     = 1 << 2,
	Borderless = 1 << 3,
};

ENUM_CLASS_FLAGS(EFrameBitmask);

/**
 *	Class description:
 *
 *	UAVVMFrameDecorator is an abstraction detail around the behavior of a UAVVMFrameWidget. Derived types such as Drag/Drop, Docking and Minimized are expected!
 *	Instead of having all Context Window own instance running the same behavior, the Frame will hold all instance and batch update registered windows.
 */
UCLASS(Abstract, BlueprintType, NotBlueprintable)
class AVVMTOOLKIT_API UAVVMFrameDecorator : public UObject
{
	GENERATED_BODY()

public:
	virtual bool DoesSupportTick() const PURE_VIRTUAL(DoesSupportTick, return false;)
	virtual void Tick(UAVVMFrameWidget* Frame, const float NewDeltaTime) const PURE_VIRTUAL(Tick, return;);
};

/**
 *	Class description:
 *
 *	UAVVMFrameWidget is a widget class that define the base behaviour for our frame system.
 *
 *	What is a Frame System ?
 *
 *		* No, I'm not talking about a Render frame... but rather the anchor used for Window docking/undocking,
 *		dragging, etc...
 *
 *		* The goal is simple! Be able to instance a View that can accept free form content. i.e a Window (and it's children).
 *
 *	See UAVVMFloatingFrameWidget & UAVVMStaticFrameWidget for additional details.
 */
UCLASS(Abstract, NotBlueprintable)
class AVVMTOOLKIT_API UAVVMFrameWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetupFrames(const TArray<UObject*>& NewViewModels);

	UFUNCTION(BlueprintCallable)
	void AddFrame(UObject* NewViewModel);

	UFUNCTION(BlueprintCallable)
	void RemoveFrame(UObject* NewViewModel);

	UFUNCTION(BlueprintCallable)
	void CloseAllFrames();

	UFUNCTION(BlueprintCallable)
	void SetParent(const UAVVMFrameWidget* NewParent, UObject* NewViewModel);

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual bool Initialize() override;
	virtual void SetupFrames_Internal(TArray<UObject*> NewViewModels) PURE_VIRTUAL(SetupFrames_Internal, return;);
	virtual void AddFrame_Internal(UObject* NewViewModel) PURE_VIRTUAL(AddFrame_Internal, return;);
	virtual void RemoveFrame_Internal(UObject* NewViewModel) PURE_VIRTUAL(RemoveFrame_Internal, return;);

	void MakeWidgetClass();
	void MakeBorderClass();

#if WITH_EDITORONLY_DATA
	virtual void PreviewEntries();
	virtual void PreviewBorder();
#endif

	void RegisterChild(UObject* NewViewModel, const FFrameZOrder& NewZOrder);
	virtual void RegisterChild_Internal(UObject* NewViewModel, const FFrameZOrder& NewZOrder) const PURE_VIRTUAL(RegisterChild_Internal, return;);

	void UnRegisterChild(UObject* NewViewModel);
	virtual void UnRegisterChild_Internal(UObject* NewViewModel) const PURE_VIRTUAL(UnRegisterChild_Internal, return;);

	UAVVMFrameBorder* IfCheckCreateBorder();
	virtual bool AllowInnerBorders() const;
	void SafeAddBorder(UObject* NewViewModel);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Designers")
	TSoftObjectPtr<UAVVMWidgetPickerDataAsset> WidgetPickerDataAsset = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Designers")
	bool bOverrideWidgetPicker = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Designers", meta=(EditCondition="bOverrideWidgetPicker"))
	TSoftClassPtr<UAVVMFrameWidget> WidgetClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Designers")
	bool bSupportBorderClass = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Designers", meta=(EditCondition="bSupportBorderClass"))
	TSoftClassPtr<UAVVMFrameBorder> BorderWidgetClass = nullptr;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Designers", meta=(Bitmask, BitmaskEnum="EFrameBitmask"))
	int32 PreviewFrameFlags = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Designers", meta=(UIMin=1, UIMax=20))
	int32 NumPreviewEntries = 5;

	UPROPERTY(Transient, BlueprintReadOnly)
	TArray<TObjectPtr<UObject>> EditorPreviewObjects;

	UPROPERTY(Transient, BlueprintReadOnly)
	TSubclassOf<UAVVMFrameWidget> PreviousWidgetClass = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly)
	bool bPreviousBorderFlagStatus = false;

	UPROPERTY(Transient, BlueprintReadOnly)
	TSubclassOf<UAVVMFrameBorder> PreviousBorderClass = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly)
	int32 PreviousNumPreviewEntries = 0;
#endif

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Designers")
	FGameplayTag FrameIdTag = FGameplayTag::EmptyTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Designers")
	TArray<TSubclassOf<UAVVMFrameDecorator>> WindowDecoratorClasses;

	UPROPERTY(Transient, BlueprintReadOnly)
	TArray<TObjectPtr<const UAVVMFrameDecorator>> WindowDecorators;

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<const UAVVMFrameWidget> Parent = nullptr;

	UPROPERTY(Transient)
	TMap<TWeakObjectPtr<UObject>, FFrameZOrder> ViewModelToWindowContext;

	UPROPERTY(Transient, BlueprintReadOnly)
	EFrameBitmask FrameFlags = EFrameBitmask::None;

	UPROPERTY(Transient, BlueprintReadOnly)
	int32 ZOrder = INDEX_NONE;

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<UAVVMFrameBorder> OwningBorder = nullptr;

	TSharedPtr<FStreamableHandle> WidgetClassPickerHandle = nullptr;
	TSharedPtr<FStreamableHandle> WidgetClassHandle = nullptr;
	TSharedPtr<FStreamableHandle> BorderClassHandle = nullptr;
};

/**
 *	Class description:
 *
 *	UAVVMFrameHeader is a widget class that owns a Frame and is only visible when the owned Frame is non-borderless.
 */
UCLASS(Blueprintable)
class AVVMTOOLKIT_API UAVVMFrameBorder : public UAVVMFrameWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SwapRoots(UAVVMFrameWidget* NewFrame);

	UFUNCTION(BlueprintCallable)
	void Revert(UAVVMFrameWidget* NewFrame);

protected:
	UPROPERTY(Transient, BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UOverlay> Anchor = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> Title = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UCommonButtonBase> Button = nullptr;
};
