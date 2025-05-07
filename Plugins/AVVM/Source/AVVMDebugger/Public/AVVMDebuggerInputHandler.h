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

#include "AVVMDebuggerInputHandler.generated.h"

/**
 *	Class Description :
 *
 *	FAVVMDebuggerInputPreprocessor is the very first input handling system
 *	that can take ownership of 'OS' inputs. (See FSlateApplication - i.e Generic Message handler).
 */
class AVVMDEBUGGER_API FAVVMDebuggerInputPreprocessor : public IInputProcessor
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
 *	UAVVMDebuggerInputHandler represent the Context Object which handle invoking our debugger.
 */
UCLASS(BlueprintType, Blueprintable)
class AVVMDEBUGGER_API UAVVMDebuggerInputHandler : public UObject
{
	GENERATED_BODY()

public:
	void SafeBegin(const FSimpleDelegate& PreprocessorCallback);
	void SafeEnd();
	TSharedPtr<FAVVMDebuggerInputPreprocessor> GetPreprocessor() const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FKey> Keys;

	TSharedPtr<FAVVMDebuggerInputPreprocessor> DebuggerPreprocessor = nullptr;
};
