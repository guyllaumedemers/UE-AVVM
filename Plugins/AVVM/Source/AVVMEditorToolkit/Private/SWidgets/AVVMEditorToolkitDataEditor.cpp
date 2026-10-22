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
#include "SWidgets/AVVMEditorToolkitDataEditor.h"

#include "ClassViewerFilter.h"
#include "ClassViewerModule.h"
#include "DataRegistry.h"
#include "Kismet2/SClassPickerDialog.h"
#include "Modules/ModuleManager.h"
#include "Widgets/Images/SImage.h"

FName UAVVMEditorToolkitDataEditObject::GetRegistryType_ActorIdentifiers() const
{
	return TEXT("R_ActorIdentifier");
}

FName UAVVMEditorToolkitDataEditObject::GetRegistryType_ActorDefinitions() const
{
	return TEXT("R_ActorDefinitions");
}

FName UAVVMEditorToolkitDataEditObject::GetRegistryType_ActorUIDefinitions() const
{
	return TEXT("R_ActorUIDefinitions");
}

void SAVVMEditorToolkitDataEditor::Construct(const FArguments& InArgs)
{
	auto* Copy = DuplicateObject<UAVVMEditorToolkitDataEditObject>(InArgs._DataEditorObject.Get().Get(), nullptr);
	DataEditObject = TStrongObjectPtr(Copy);

	ChildSlot
	[
		GetDataEditContent(InArgs._OutputDataTable.Get(), InArgs._DataTablePackagePath.Get(), InArgs._Mode.Get()).ToSharedRef()
	];
}

TSharedPtr<SWidget> SAVVMEditorToolkitDataEditor::GetDataEditContent(const TWeakObjectPtr<UDataTable>& OutputDataTable,
                                                                     const FString& PackagePath,
                                                                     const EAVVMDataEditorModes CurrentMode) const
{
	if (DataEditObject.IsValid())
	{
		return DataEditObject->GetDataEditContent(OutputDataTable, PackagePath, CurrentMode);
	}
	else
	{
		return SNew(SImage)
			.ColorAndOpacity(FLinearColor::Transparent);
	}
}

FText SAVVMEditorToolkitDataEditor::GetModalMessage_OnClosure() const
{
	if (DataEditObject.IsValid())
	{
		return DataEditObject->GetModalMessage_OnClosure();
	}
	else
	{
		return NSLOCTEXT("AVVMEditorToolkitDataEditor", "DataEditor_ClosureMessage", "Are you sure you want to close this window?");
	}
}

FText SAVVMEditorToolkitDataEditor::GetModalTitle_OnClosure() const
{
	if (DataEditObject.IsValid())
	{
		return DataEditObject->GetModalTitle_OnClosure();
	}
	else
	{
		return NSLOCTEXT("AVVMEditorToolkitDataEditor", "DataEditor_ClosureTitle", "Missing Source!");
	}
}

// @gdemers IMPORTANT : UDataRegistryFactory is Private which is why im duplicating the api here!
UAVVMDataRegistryFactory::UAVVMDataRegistryFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UDataRegistry::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

class FAVVMAssetClassParentFilter : public IClassViewerFilter
{
public:
	FAVVMAssetClassParentFilter()
		: DisallowedClassFlags(CLASS_None)
	{}

	/** All children of these classes will be included unless filtered out by another setting. */
	TSet< const UClass* > AllowedChildrenOfClasses;

	/** Disallowed class flags. */
	EClassFlags DisallowedClassFlags;

	virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs) override
	{
		bool bAllowed= !InClass->HasAnyClassFlags(DisallowedClassFlags)
			&& InFilterFuncs->IfInChildOfClassesSet(AllowedChildrenOfClasses, InClass) != EFilterReturn::Failed;

		return bAllowed;
	}

	virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef< const IUnloadedBlueprintData > InUnloadedClassData, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs) override
	{
		return !InUnloadedClassData->HasAnyClassFlags(DisallowedClassFlags)
			&& InFilterFuncs->IfInChildOfClassesSet(AllowedChildrenOfClasses, InUnloadedClassData) != EFilterReturn::Failed;
	}
};

bool UAVVMDataRegistryFactory::ConfigureProperties()
{
	// nullptr the DataRegistryClass so we can check for selection
	DataRegistryClass = nullptr;

	// Load the classviewer module to display a class picker
	FClassViewerModule& ClassViewerModule = FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer");

	// Fill in options
	FClassViewerInitializationOptions Options;
	Options.Mode = EClassViewerMode::ClassPicker;
	// This doesn't currently seem to work
	// Options.InitiallySelectedClass = UDataRegistry::StaticClass();

	TSharedPtr<FAVVMAssetClassParentFilter> Filter = MakeShareable(new FAVVMAssetClassParentFilter);
	Options.ClassFilters.Add(Filter.ToSharedRef());

	Filter->DisallowedClassFlags = CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists | CLASS_HideDropDown;
	Filter->AllowedChildrenOfClasses.Add(UDataRegistry::StaticClass());

	const FText TitleText = NSLOCTEXT("", "CreateDataRegistryOptions", "Pick Data Registry Base Class");
	UClass* ChosenClass = nullptr;
	const bool bPressedOk = SClassPickerDialog::PickClass(TitleText, Options, ChosenClass, UDataRegistry::StaticClass());

	if (bPressedOk)
	{
		DataRegistryClass = ChosenClass;
	}

	return bPressedOk;
}

UObject* UAVVMDataRegistryFactory::FactoryCreateNew(UClass* Class,
                                                    UObject* InParent,
                                                    FName Name,
                                                    EObjectFlags Flags,
                                                    UObject* Context,
                                                    FFeedbackContext* Warn)
{
	if (IsValid(DataRegistryClass))
	{
		Class = DataRegistryClass;
	}

	ensure(0 != (RF_Public & Flags));
	UDataRegistry* DataRegistry = NewObject<UDataRegistry>(InParent, Class, Name, Flags | RF_Transactional);
	return DataRegistry;
}
