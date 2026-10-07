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
#include "SWidgets/AVVMEditorToolkitDataTableRowPreview.h"

#include "DataRegistrySubsystem.h"
#include "Components/HorizontalBox.h"
#include "Data/AVVMDataTableRow.h"
#include "Editor/PropertyEditor/Private/SDetailsView.h"
#include "Engine/AssetManager.h"
#include "Modules/ModuleManager.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

void SAVVMEditorToolkitDataTableRowPreview::Construct(const FArguments& InArgs)
{
	RegistryId = FDataRegistryId{InArgs._RegistryType.Get(), InArgs._RegistryItemName.Get()};

	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& EditModule = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

		FDetailsViewArgs DetailsViewArgs{};
		DetailsViewArgs.bAllowSearch = false;
		DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
		DetailsViewArgs.bHideSelectionTip = true;

		ObjectPropertyView = EditModule.CreateDetailView(DetailsViewArgs);
		if (ensureAlwaysMsgf(ObjectPropertyView.IsValid(), TEXT("Invalid DetailView.")))
		{
			ObjectPropertyView->SetObjects(GetAssetsFromRegistryId());
			ObjectPropertyView->GetIsPropertyEditingEnabledDelegate().BindRaw(this, &SAVVMEditorToolkitDataTableRowPreview::OnEnable_DetailView);
		}
	}

	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(12.f)
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromName(RegistryId.ItemName))
				.Font(FAppStyle::GetFontStyle(TEXT("PropertyWindow.BoldFont")))
			]
			+ SVerticalBox::Slot()
			[
				SNew(SSplitter)
			]
			+ SVerticalBox::Slot()
			.Padding(12.f)
			.AutoHeight()
			[
				ObjectPropertyView.ToSharedRef()
			]
		]
	];
}

TArray<UObject*> SAVVMEditorToolkitDataTableRowPreview::GetAssetsFromRegistryId() const
{
	auto* Subsystem = UDataRegistrySubsystem::Get();
	if (!IsValid(Subsystem))
	{
		return TArray<UObject*>{};
	}

	// TODO @gdemers make loading process more efficient using async query
	const auto* RowData = Subsystem->GetCachedItem<FAVVMDataTableRow>(RegistryId);
	if (ensureAlwaysMsgf(RowData != nullptr, TEXT("Invalid Payload.")))
	{
		TArray<UObject*> Objects{};
		for (const FSoftObjectPath& Path : RowData->GetResourcesPaths())
		{
			Objects.Add(Path.TryLoad());
		}

		return Objects;
	}
	else
	{
		return TArray<UObject*>{};
	}
}

bool SAVVMEditorToolkitDataTableRowPreview::OnEnable_DetailView() const
{
	return false;
}
