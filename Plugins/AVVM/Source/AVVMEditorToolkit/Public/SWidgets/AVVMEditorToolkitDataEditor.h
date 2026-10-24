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

#include "AVVMDoesWidgetProvideModalMessaging.h"
#include "AVVMEditorToolkitDataImporter.h"
#include "Factories/Factory.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/SCompoundWidget.h"

#include "AVVMEditorToolkitDataEditor.generated.h"

struct FDataRegistryType;
class UDataRegistry;
class UDataTable;
class SWindow;

/**
 *	Class description:
 *	
 *	EAVVMDataEditorModes is an Enum type to track active mode of an active Window instance.
 */
UENUM(NotBlueprintType)
enum class EAVVMDataEditorModes : uint8
{
	None UMETA(Hidden),
	Create,
	Edit
};

/**
 *	Class description:
 *	
 *	FAVVMEditContextArgs is a context struct encapsulating arguments.
 */
USTRUCT(BlueprintType)
struct AVVMEDITORTOOLKIT_API FAVVMEditContextArgs
{
	GENERATED_BODY()
	
	UPROPERTY(Transient, BlueprintReadWrite)
	TWeakObjectPtr<UDataTable> OutputDataTable{nullptr};
	
	UPROPERTY(Transient, BlueprintReadWrite)
	FString PackagePath{};
	
	UPROPERTY(Transient, BlueprintReadWrite)
	EAVVMDataEditorModes CurrentMode{EAVVMDataEditorModes::None};
};

/**
 *	Class description:
 *	
 *	UAVVMEditorToolkitDataEditObject is an abstract UObject type that define the internal behaviour
 *	to editing plugin specific data types.
 */
UCLASS(Abstract, NotBlueprintType, NotBlueprintable)
class AVVMEDITORTOOLKIT_API UAVVMEditorToolkitDataEditObject : public UObject
{
	GENERATED_BODY()

public:
	virtual TSharedPtr<SWidget> GetDataEditContent(const FAVVMEditContextArgs& Params) PURE_VIRTUAL(GetDataEditContent, return nullptr;)
	virtual int32 GenerateActorIdentifier(const FName ActorType) const PURE_VIRTUAL(GenerateActorIdentifier, return INDEX_NONE;)
	virtual FText GetModalMessage_OnClosure() const PURE_VIRTUAL(GetModalMessage_OnClosure, return FText::GetEmpty(););
	virtual FText GetModalTitle_OnClosure() const PURE_VIRTUAL(GetModalTitle_OnClosure, return FText::GetEmpty(););

protected:
	static FName GetRegistryType_ActorIdentifiers();
	static FName GetRegistryType_ActorDefinitions();
	static FName GetRegistryType_ActorUIDefinitions();

	void OnWindowClosedOverride(const TSharedRef<SWindow>& PendingCloseWindow) const;
	void OnWindowClosed(const TSharedRef<SWindow>& PendingCloseWindow);
	virtual void OnPostWindowClosed() const PURE_VIRTUAL(OnPostWindowClosed, return;);
	
	TArray<TSharedPtr<SWindow>> FloatingWindows{};
};

/**
 *	Class description:
 *	
 *	SAVVMEditorToolkitDataEditor is a slate context handling data edit modes.
 */
class AVVMEDITORTOOLKIT_API SAVVMEditorToolkitDataEditor : public SCompoundWidget,
                                                           public IAVVMDoesWidgetProvideModalMessaging
{
public:
	SLATE_BEGIN_ARGS(SAVVMEditorToolkitDataEditor){}
	SLATE_ATTRIBUTE(TWeakObjectPtr<const UAVVMEditorToolkitDataEditObject>, DataEditorObject)
	SLATE_ATTRIBUTE(TWeakObjectPtr<UDataTable>, OutputDataTable)
	SLATE_ATTRIBUTE(FString, DataTablePackagePath)
	SLATE_ATTRIBUTE(EAVVMDataEditorModes, Mode)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	TSharedPtr<SWidget> GetDataEditContent(const FAVVMEditContextArgs& Params) const;

	// IAVVMDoesWidgetProvideModalMessaging
	virtual FText GetModalMessage_OnClosure() const override;
	virtual FText GetModalTitle_OnClosure() const override;

private:
	TStrongObjectPtr<UAVVMEditorToolkitDataEditObject> DataEditObject{nullptr};
};

/**
 *	Class description:
 *	
 *	UAVVMDataRegistryFactory is a custom factory class that mimic UDataRegistryFactory, and create new UDataRegistry Asset. This class
 *	however goes one step further, and is able to resolve provided ItemStruct & RegistryType during initialization done in c++.
 */
UCLASS(hidecategories = Object)
class AVVMEDITORTOOLKIT_API UAVVMDataRegistryFactory : public UFactory
{
	GENERATED_UCLASS_BODY()

	virtual bool ConfigureProperties() override;
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Designers")
	TSubclassOf<UDataRegistry> DataRegistryClass{nullptr};

	/** Structure type of all for items in this registry */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Designers")
	TObjectPtr<const UScriptStruct> ItemStruct{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Designers")
	FName RegistryType{NAME_None};
};
