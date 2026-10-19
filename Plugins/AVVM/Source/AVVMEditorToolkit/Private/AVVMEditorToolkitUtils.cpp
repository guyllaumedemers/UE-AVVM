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
#include "AVVMEditorToolkitUtils.h"

#include "DataRegistrySource_DataTable.h"
#include "DataRegistrySubsystem.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "Framework/Notifications/NotificationManager.h"
#include "GenericPlatform/GenericApplication.h"
#include "HAL/PlatformApplicationMisc.h"

TSharedRef<SWindow> UAVVMEditorToolkitUtils::OpenWindow(const FAVVMWindowContextArgs& Params)
{
	FDisplayMetrics DisplayMetrics{};
	FSlateApplication::Get().GetDisplayMetrics(DisplayMetrics);
	const float DPIScaleFactor = FPlatformApplicationMisc::GetDPIScaleFactorAtPoint(DisplayMetrics.PrimaryDisplayWorkAreaRect.Left, DisplayMetrics.PrimaryDisplayWorkAreaRect.Top);
	const FVector2D ClientSize(1200.0f * DPIScaleFactor, 800.0f * DPIScaleFactor);

	auto NewWindow = SNew(SWindow)
		.Title(NSLOCTEXT("AVVMEditorToolkit", "WindowTitle_DataEditor", "DataEditor"))
		.CreateTitleBar(true)
		.SupportsMaximize(true)
		.SupportsMinimize(true)
		.IsInitiallyMaximized(false)
		.IsInitiallyMinimized(false)
		.SizingRule(ESizingRule::UserSized)
		.AutoCenter(EAutoCenter::PreferredWorkArea)
		.ClientSize(ClientSize)
		.AdjustInitialSizeAndPositionForDPIScale(false)
		.Content()
		[
			Params.Content.ToSharedRef()
		];

	NewWindow->SetRequestDestroyWindowOverride(Params.OnRequestDestroyWindowOverride);
	NewWindow->SetOnWindowClosed(Params.OnWindowClosed);

	FSlateApplication::Get().AddWindow(NewWindow, true);
	FGlobalTabmanager::Get()->SetRootWindow(NewWindow);
	FGlobalTabmanager::Get()->SetAllowWindowMenuBar(true);
	FSlateNotificationManager::Get().SetRootWindow(NewWindow);
	return NewWindow;
}

TArray<const UDataRegistrySource_DataTable*> UAVVMEditorToolkitUtils::GetDataTables(const FDataRegistryType& RegistryType)
{
	TArray<const UDataRegistrySource_DataTable*> OutResult{};

	const auto* Subsystem = UDataRegistrySubsystem::Get();
	if (!IsValid(Subsystem))
	{
		return OutResult;
	}

	const auto* DataRegistry = Subsystem->GetRegistryForType(RegistryType);
	if (!IsValid(DataRegistry))
	{
		return OutResult;
	}

	// @gdemers sadly, the DataRegistry class doesnt allow direct access to the DataSources property.
	TArray<FDataRegistrySourceItemId> OutElements{};
	DataRegistry->GetAllSourceItems(OutElements);

	for (const auto& Element : OutElements)
	{
		const auto* Source = Cast<UDataRegistrySource_DataTable>(Element.CachedSource);
		if (IsValid(Source))
		{
			OutResult.AddUnique(Source);
		}
	}

	return OutResult;
}

UDataRegistry* UAVVMEditorToolkitUtils::GetDataRegistry(const FDataRegistryType& RegistryType)
{
	const auto* Subsystem = UDataRegistrySubsystem::Get();
	return IsValid(Subsystem) ? Subsystem->GetRegistryForType(RegistryType) : nullptr;
}
