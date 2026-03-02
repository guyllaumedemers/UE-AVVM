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

#include "EditorSubsystem.h"
#include "Engine/StreamableManager.h"

#include "AVVMEditorBuilderSubsystem.generated.h"

class UAVVMEditorToolkitBuilderObject;
class UEditorEngine;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnBuildContextChangedDelegate, UAVVMEditorToolkitBuilderObject* NewBuilderContext);

/**
 *	Class description:
 *	
 *	UAVVMEditorBuilderSubsystem is an Editor subsystem that manage a set of UAVVMEditorToolkitBuilderObject. Those are referenced from Config files (.ini)
 *	and used as building blocks for dynamically populating the slate layout of our Toolkit.
 */
UCLASS(config = Game)
class AVVMEDITORTOOLKIT_API UAVVMEditorBuilderSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

	FOnBuildContextChangedDelegate OnBuildContextChanged;
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	UFUNCTION(BlueprintCallable)
	static TArray<UAVVMEditorToolkitBuilderObject*> Static_GetAllBuilders(const UEditorEngine* Editor);
	
	UFUNCTION(BlueprintCallable)
	static UAVVMEditorToolkitBuilderObject* Static_GetActiveBuilder(const UEditorEngine* Editor);

	UFUNCTION(BlueprintCallable)
	static void Static_SelectBuilder(const UEditorEngine* Editor,
	                                 UAVVMEditorToolkitBuilderObject* Builder);

	static FDelegateHandle Static_CallOrRegisterOnBuildContextChanged(const UEditorEngine* Editor,
	                                                                  const FOnBuildContextChangedDelegate::FDelegate& Delegate);

protected:
	static UAVVMEditorBuilderSubsystem* Get(const UEditorEngine* Editor);
	
	UFUNCTION()
	void OnAssetManagerInitialized();
	
	UFUNCTION()
	void OnSoftObjectAcquired();
	
	// @gdemers IMPORTANT : check DefaultGame.ini to inspect class references.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, config)
	TArray<TSoftClassPtr<UAVVMEditorToolkitBuilderObject>> ToolkitBuilderClasses;

	UPROPERTY(Transient, BlueprintReadOnly)
	TArray<TObjectPtr<UAVVMEditorToolkitBuilderObject>> ToolkitBuilders;

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<UAVVMEditorToolkitBuilderObject> ActiveToolkitBuilder = nullptr;

	TSharedPtr<FStreamableHandle> StreamableHandle = nullptr;
};
