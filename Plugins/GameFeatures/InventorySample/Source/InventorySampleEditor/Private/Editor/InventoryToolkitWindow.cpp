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
#include "InventoryToolkitWindow.h"

#include "AVVMLogger.h"
#include "InventorySettings.h"
#include "SWidgets/AVVMEditorToolkitDataVisualizer.h"
#include "Widgets/SOverlay.h"

void SInventoryToolkitWindow::Construct(const FArguments& InArgs)
{
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
				SNew(SAVVMEditorToolkitDataImporter)
				.DataRegistryTypes(GetInventoryDataRegistryTypes())
				.OnDataImporterSourceChanged(this, &SInventoryToolkitWindow::OnDataImporterSourceChanged)
				.OnButtonClick_Create(this, &SInventoryToolkitWindow::OnButtonClick_Create)
				.OnButtonClick_Edit(this, &SInventoryToolkitWindow::OnButtonClick_Edit)
				.OnButtonClick_Delete(this, &SInventoryToolkitWindow::OnButtonClick_Delete)
			]
			+ SVerticalBox::Slot()
			.Padding(FMargin(0.0f, 0.0f, 0.0f, 2.0f))
			.FillContentHeight(1.f)
			[
				SAssignNew(DataVisualizer, SAVVMEditorToolkitDataVisualizer)
				.Visibility(this, &SInventoryToolkitWindow::OnDataVisualizerVisibilityStateChanged)
			]
		]
	];
}

bool SInventoryToolkitWindow::OnDataImporterSourceChanged(FName SelectedSourceType)
{
	SelectedDataRegistryType = FDataRegistryType{SelectedSourceType};
	if (DataVisualizer.IsValid())
	{
		DataVisualizer->UpdateDataVisualizer(SelectedDataRegistryType);
	}

	return SelectedDataRegistryType.IsValid();
}

EVisibility SInventoryToolkitWindow::OnDataVisualizerVisibilityStateChanged() const
{
	return SelectedDataRegistryType.IsValid() ? EVisibility::Visible : EVisibility::Collapsed;
}

TArray<FName> SInventoryToolkitWindow::GetInventoryDataRegistryTypes() const
{
	return {
			UInventorySettings::GetItemGroupRegistryType(),
			UInventorySettings::GetItemRegistryType(),
			UInventorySettings::GetFtueInventoryProviderRegistryType(),
			UInventorySettings::GetStubDataInventoryDependencyGraphRegistryType()
	};
}

FReply SInventoryToolkitWindow::OnButtonClick_Create()
{
	if (!SelectedDataRegistryType.IsValid())
	{
		return FReply::Unhandled();
	}
	else
	{
		OpenCreateWindow(SelectedDataRegistryType);
		return FReply::Handled();
	}
}

FReply SInventoryToolkitWindow::OnButtonClick_Edit()
{
	if (!SelectedDataRegistryType.IsValid() || !DataVisualizer.IsValid())
	{
		return FReply::Unhandled();
	}

	bool bResult{false};

	TArray<FName> OutItemNames{};
	bResult = DataVisualizer->GetSelectedItems(OutItemNames);

	for (const auto& ItemName : OutItemNames)
	{
		OpenEditWindow(SelectedDataRegistryType, ItemName);
	}

	return bResult ? FReply::Handled() : FReply::Unhandled();
}

FReply SInventoryToolkitWindow::OnButtonClick_Delete()
{
	if (SelectedDataRegistryType.IsValid())
	{
		return FReply::Handled();
	}
	else
	{
		return FReply::Unhandled();
	}
}

void SInventoryToolkitWindow::OpenCreateWindow(const FName RegistryType)
{
	// TODO @gdemers make a context window for creating element
	AVVM_EDITOR_LOGGER_LOG(TEXT("Create new entry of RegistryType %s."),
	                       *RegistryType.ToString());
}

void SInventoryToolkitWindow::OpenEditWindow(const FName RegistryType, const FName ItemName)
{
	// TODO @gdemers make a context window for editing element
	AVVM_EDITOR_LOGGER_LOG(TEXT("Edit entry of RegistryType: %s, ItemName: %s."),
	                       *RegistryType.ToString(),
	                       *ItemName.ToString());
}
