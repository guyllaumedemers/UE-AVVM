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

#include "InputCoreTypes.h"
#include "UObject/Object.h"
#include "Framework/Application/IInputProcessor.h"

#include "AVVMImGuiInputHandler.generated.h"

/**
 *	Class Description :
 *
 *	FAVVMImGuiInputPreprocessor is the very first input handling system
 *	that can take ownership of 'OS' inputs. (See FSlateApplication - i.e Generic Message handler).
 */
class AVVMIMGUI_API FAVVMImGuiInputPreprocessor : public IInputProcessor
{
public:
	void RegisterKeys(const TArray<FKey>& NewKeys);
	virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override;
	virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override;

	FSimpleDelegate OnToggleDebuggerContext;

protected:
	TArray<FKey> Keys;
};

/**
 *	Class Description :
 *
 *	UAVVMImGuiInputHandler is the context object that handle opening/closing our debugger. It creates and
 *	destroy the Input Preprocessor based on the application state.
 */
UCLASS(BlueprintType, Blueprintable)
class AVVMIMGUI_API UAVVMImGuiInputHandler : public UObject
{
	GENERATED_BODY()

public:
	void SafeBegin(const FSimpleDelegate& PreprocessorCallback);
	void SafeEnd();
	TSharedPtr<FAVVMImGuiInputPreprocessor> GetPreprocessor() const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	TArray<FKey> Keys;

	TSharedPtr<FAVVMImGuiInputPreprocessor> DebuggerPreprocessor = nullptr;
};
