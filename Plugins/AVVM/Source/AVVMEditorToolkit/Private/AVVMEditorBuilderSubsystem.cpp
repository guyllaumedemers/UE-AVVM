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
#include "AVVMEditorBuilderSubsystem.h"

#include "AVVMEditorToolkitBuilderObject.h"
#include "Editor/EditorEngine.h"
#include "Engine/AssetManager.h"

void UAVVMEditorBuilderSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// @gdemers Note : The asset manager is created later in the initialization process of the engine
	auto Callback = FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &UAVVMEditorBuilderSubsystem::OnAssetManagerInitialized);
	UAssetManager::CallOrRegister_OnCompletedInitialScan(MoveTemp(Callback));
}

void UAVVMEditorBuilderSubsystem::Deinitialize()
{
	Super::Deinitialize();

	ActiveToolkitBuilder.Reset();
	StreamableHandle.Reset();
	ToolkitBuilders.Reset();
}

TArray<UAVVMEditorToolkitBuilderObject*> UAVVMEditorBuilderSubsystem::Static_GetAllBuilders(const UEditorEngine* Editor)
{
	TArray<UAVVMEditorToolkitBuilderObject*> OutResults;
	auto* Subsystem = UAVVMEditorBuilderSubsystem::Get(Editor);
	if (IsValid(Subsystem))
	{
		OutResults = Subsystem->ToolkitBuilders;
	}

	return OutResults;
}

UAVVMEditorToolkitBuilderObject* UAVVMEditorBuilderSubsystem::Static_GetActiveBuilder(const UEditorEngine* Editor)
{
	auto* Subsystem = UAVVMEditorBuilderSubsystem::Get(Editor);
	return IsValid(Subsystem) ? Subsystem->ActiveToolkitBuilder.Get() : nullptr;
}

void UAVVMEditorBuilderSubsystem::Static_SelectBuilder(const UEditorEngine* Editor,
                                                       UAVVMEditorToolkitBuilderObject* Builder)
{
	auto* Subsystem = UAVVMEditorBuilderSubsystem::Get(Editor);
	if (IsValid(Subsystem) && (Subsystem->ActiveToolkitBuilder != Builder))
	{
		Subsystem->ActiveToolkitBuilder = Builder;
		Subsystem->OnBuildContextChanged.Broadcast(Builder);
	}
}

FDelegateHandle UAVVMEditorBuilderSubsystem::Static_CallOrRegisterOnBuildContextChanged(const UEditorEngine* Editor,
                                                                                        const FOnBuildContextChangedDelegate::FDelegate& Delegate)
{
	auto* Subsystem = UAVVMEditorBuilderSubsystem::Get(Editor);
	return IsValid(Subsystem) ? Subsystem->OnBuildContextChanged.Add(Delegate) : FDelegateHandle();
}

UAVVMEditorBuilderSubsystem* UAVVMEditorBuilderSubsystem::Get(const UEditorEngine* Editor)
{
	return IsValid(Editor) ? Editor->GetEditorSubsystem<UAVVMEditorBuilderSubsystem>() : nullptr;
}

void UAVVMEditorBuilderSubsystem::OnAssetManagerInitialized()
{
	TArray<FSoftObjectPath> SoftObjectPaths;
	for (const auto& ToolkitBuilderClass : ToolkitBuilderClasses)
	{
		SoftObjectPaths.Add(ToolkitBuilderClass.ToSoftObjectPath());
	}

	if (!SoftObjectPaths.IsEmpty())
	{
		const auto Callback = FStreamableDelegate::CreateUObject(this, &UAVVMEditorBuilderSubsystem::OnSoftObjectAcquired);
		StreamableHandle = UAssetManager::Get().LoadAssetList(SoftObjectPaths, Callback);
	}
}

void UAVVMEditorBuilderSubsystem::OnSoftObjectAcquired()
{
	if (!StreamableHandle.IsValid())
	{
		return;
	}

	TArray<UObject*> OutStreamableAssets;
	StreamableHandle->GetLoadedAssets(OutStreamableAssets);

	ToolkitBuilders.Reset();
	for (const UObject* OutStreamableAsset : OutStreamableAssets)
	{
		const auto* BuilderClass = Cast<UClass>(OutStreamableAsset);
		if (IsValid(BuilderClass))
		{
			auto* Builder = NewObject<UAVVMEditorToolkitBuilderObject>(this, BuilderClass);
			ToolkitBuilders.Add(Builder);
		}
	}

	if (!ToolkitBuilders.IsEmpty())
	{
		ActiveToolkitBuilder.Reset();
		ActiveToolkitBuilder = ToolkitBuilders[0];
	}
}
