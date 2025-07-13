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
#include "Engine/StreamableManager.h"
#include "Templates/SubclassOf.h"

#include "AVVMFrameWidget.generated.h"

class UAVVMWidgetPickerDataAsset;
class UAVVMWindowWidget;

/**
 *	Class description:
 *
 *	FWindowZOrder is a POD that cache data about the window context and the layer on which it lives.
 */
USTRUCT(BlueprintType)
struct AVVMTOOLKIT_API FWindowZOrder
{
	GENERATED_BODY()

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<UCommonUserWidget> Window;

	UPROPERTY(Transient, BlueprintReadOnly)
	int32 ZOrder = INDEX_NONE;
};

/**
 *	Class description:
 *
 *	UAVVMWindowDecorator is an abstract around the implementation details of a Widget behavior. Derived types such as Drag/Drop, Docking and Minimized are expected!
 *	Instead of having all Context Window own instance running the same behaviour, the Frame will hold all instance and batch update registered windows.
 */
UCLASS(Abstract, BlueprintType, NotBlueprintable)
class AVVMTOOLKIT_API UAVVMWindowDecorator : public UObject
{
	GENERATED_BODY()

public:
	virtual bool DoesSupportTick() const PURE_VIRTUAL(DoesSupportTick, return false;)
	virtual void Tick(UAVVMWindowWidget* Window, const float NewDeltaTime) PURE_VIRTUAL(Tick, return;);
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
	void SetupWindows(const TArray<UObject*>& NewViewModels);

	UFUNCTION(BlueprintCallable)
	void AddWindow(UObject* NewViewModel);

	UFUNCTION(BlueprintCallable)
	void RemoveWindow(UObject* NewViewModel);

	UFUNCTION(BlueprintCallable)
	void CloseAllWindows();

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void SetupWindows_Internal(TArray<UObject*> NewViewModels) PURE_VIRTUAL(SetupWindows_Internal, return;);
	virtual void AddWindow_Internal(UObject* NewViewModel) PURE_VIRTUAL(AddWindow_Internal, return;);
	virtual void RemoveWindow_Internal(UObject* NewViewModel) PURE_VIRTUAL(RemoveWindow_Internal, return;);

#if WITH_EDITORONLY_DATA
	virtual void PreviewEntries();
#endif

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Designers")
	TSoftObjectPtr<UAVVMWidgetPickerDataAsset> WidgetPickerDataAsset = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Designers")
	bool bOverrideWidgetPicker = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Designers", meta=(EditCondition="bOverrideWidgetPicker"))
	TSoftClassPtr<UCommonUserWidget> WidgetClass = nullptr;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Designers", meta=(UIMin=1, UIMax=20))
	int32 NumPreviewEntries = 5;

	UPROPERTY(Transient, BlueprintReadOnly)
	TArray<TObjectPtr<UObject>> EditorPreviewObjects;

	UPROPERTY(Transient, BlueprintReadOnly)
	TSubclassOf<UCommonUserWidget> PreviousWidgetClass = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly)
	int32 PreviousNumPreviewEntries = 0;
#endif

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Designers")
	TArray<TSubclassOf<UAVVMWindowDecorator>> WindowDecoratorClasses;

	UPROPERTY(Transient, BlueprintReadOnly)
	TArray<TObjectPtr<UAVVMWindowDecorator>> WindowDecorators;

	UPROPERTY(Transient)
	TMap<TWeakObjectPtr<UObject>, FWindowZOrder> ViewModelToWindowContext;

	TSharedPtr<FStreamableHandle> StreamableHandle = nullptr;
};
