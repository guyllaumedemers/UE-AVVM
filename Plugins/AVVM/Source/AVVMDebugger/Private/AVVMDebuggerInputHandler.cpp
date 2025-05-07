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
#include "AVVMDebuggerInputHandler.h"

#include "Application/SlateApplicationBase.h"

void FAVVMDebuggerInputPreprocessor::RegisterKeys(const TArray<FKey>& NewKeys)
{
	Keys = NewKeys;
}

void FAVVMDebuggerInputPreprocessor::Tick(const float DeltaTime,
                                          FSlateApplication& SlateApp,
                                          TSharedRef<ICursor> Cursor)
{
	// @gdemers we dont care about tick but the function is pure virtual in IInputProcessor
	// so an implementation is required!
}

bool FAVVMDebuggerInputPreprocessor::HandleKeyDownEvent(FSlateApplication& SlateApp,
                                                        const FKeyEvent& InKeyEvent)
{
	const FKey* SearchResult = Keys.FindByPredicate([OsKey = InKeyEvent.GetKey()](const FKey& Key)
	{
		return Key == OsKey;
	});

	const bool bFoundResult = (SearchResult != nullptr);
	if (bFoundResult)
	{
		OnToggleDebuggerContext.ExecuteIfBound();
	}

	return bFoundResult;
}

void UAVVMDebuggerInputHandler::SafeBegin(const FSimpleDelegate& PreprocessorCallback)
{
	if (!DebuggerPreprocessor.IsValid())
	{
		DebuggerPreprocessor = MakeShared<FAVVMDebuggerInputPreprocessor>();
		DebuggerPreprocessor->RegisterKeys(Keys);
	}

	const int32 SearchResult = FSlateApplication::Get().FindInputPreProcessor(DebuggerPreprocessor, EInputPreProcessorType::Game);
	if (DebuggerPreprocessor.IsValid() && SearchResult == INDEX_NONE)
	{
		DebuggerPreprocessor->OnToggleDebuggerContext = PreprocessorCallback;
		ensureAlways(FSlateApplication::Get().RegisterInputPreProcessor(DebuggerPreprocessor));
	}
}

void UAVVMDebuggerInputHandler::SafeEnd()
{
	if (DebuggerPreprocessor.IsValid())
	{
		FSlateApplication::Get().UnregisterInputPreProcessor(DebuggerPreprocessor);
		DebuggerPreprocessor.Reset();
	}
}

TSharedPtr<FAVVMDebuggerInputPreprocessor> UAVVMDebuggerInputHandler::GetPreprocessor() const
{
	return DebuggerPreprocessor;
}
