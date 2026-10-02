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

#include "Widgets/SOverlay.h"

void SInventoryToolkitWindow::Construct(const FArguments& InArgs)
{
	// @gdemers Note : im unsure if that is dangerous in the slate framework ?
	auto Callback = [this]()
	{
		return OnRegisterDataImporterSourceChangeDelegate();
	};

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
				.DataSourceTypes(GetInventoryDataImporterSourceTypes())
				.OnDataImporterSourceChanged_Lambda(MoveTemp(Callback))
			]
			+ SVerticalBox::Slot()
			.Padding(FMargin(0.0f, 0.0f, 0.0f, 2.0f))
			.FillContentHeight(1.f)
			[
				SNew(SBorder)
				.Padding(12.f)
				.BorderImage(FAppStyle::Get().GetBrush("NoBorder"))
			]
		]
	];
}

SOnDataImporterSourceChangedDelegate SInventoryToolkitWindow::OnRegisterDataImporterSourceChangeDelegate()
{
	SOnDataImporterSourceChangedDelegate OutDelegate{};
	OutDelegate.AddRaw(this, &SInventoryToolkitWindow::OnDataImporterSourceChanged);
	return OutDelegate;
}

TArray<FName> SInventoryToolkitWindow::GetInventoryDataImporterSourceTypes() const
{
	return {TEXT("Data Asset"), TEXT("Backend")};
}

void SInventoryToolkitWindow::OnDataImporterSourceChanged(FName SelectedSourceType)
{
}
