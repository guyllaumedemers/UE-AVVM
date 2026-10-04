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
#include "SWidgets/AVVMEditorToolkitDataVisualizer.h"

#include "DataRegistrySubsystem.h"
#include "Components/VerticalBox.h"
#include "Data/AVVMDataTableRow.h"
#include "SWidgets/AVVMEditorToolkitDataTableRowPreview.h"
#include "Widgets/SOverlay.h"

void SAVVMEditorToolkitDataVisualizer::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			// SAssignNew binds the created SVerticalBox to DynamicBox
			SAssignNew(DynamicBox, SVerticalBox)
		]
	];
}

void SAVVMEditorToolkitDataVisualizer::UpdateDataVisualizer(const FDataRegistryType& NewRegistryType)
{
	if (!DynamicBox.IsValid() || !ensureAlwaysMsgf(NewRegistryType.IsValid(), TEXT("Invalid RegistryType.")))
	{
		return;
	}

	auto* Subsystem = UDataRegistrySubsystem::Get();
	if (!IsValid(Subsystem))
	{
		return;
	}

	const UDataRegistry* DataRegistry = Subsystem->GetRegistryForType(NewRegistryType);
	if (!IsValid(DataRegistry))
	{
		return;
	}

	DynamicBox->ClearChildren();
	DataRegistry->ForEachCachedItem<FAVVMDataTableRow>(TEXT(""), [&](const FName& Name, const auto& Item)
	{
		DynamicBox->AddSlot()
		          .AutoHeight()
		          .Padding(2.0f, 4.0f)
		[
			SNew(SAVVMEditorToolkitDataTableRowPreview)
			.TableRowData(&Item)
			.TableRowName(Name)
		];
	});
}
