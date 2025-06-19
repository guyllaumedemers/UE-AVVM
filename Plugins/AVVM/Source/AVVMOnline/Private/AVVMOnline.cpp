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

#include "AVVMOnline.h"

#include "AVVMOnlineSettings.h"
#include "Engine.h"

DEFINE_LOG_CATEGORY(LogOnline);

TSharedPtr<IConsoleVariable> FAVVMOnlineModule::CVarOnlineRequestReturnedStatus = nullptr;
TStrongObjectPtr<UObject> FAVVMOnlineModule::JsonParser = nullptr;

void FAVVMOnlineModule::StartupModule()
{
	IConsoleVariable* NewCVar = IConsoleManager::Get()
			.RegisterConsoleVariable(TEXT("OnlineRequestReturnedStatus"),
			                         false,
			                         TEXT("Stub Data for Testing Online request callback result. bWasSuccess=True/False."));

	CVarOnlineRequestReturnedStatus = MakeShareable<IConsoleVariable>(NewCVar);

	const TSubclassOf<UObject> ParserClass = UAVVMOnlineSettings::GetJsonParserClass();
	if (IsValid(ParserClass))
	{
		UObject* Parser = NewObject<UObject>(GEngine, ParserClass.Get());
		JsonParser = TStrongObjectPtr<UObject>(Parser);
	}
}

void FAVVMOnlineModule::ShutdownModule()
{
	IConsoleManager::Get().UnregisterConsoleObject(CVarOnlineRequestReturnedStatus.Get());
	CVarOnlineRequestReturnedStatus.Reset();

	JsonParser.Reset();
}

TSharedRef<IConsoleVariable> FAVVMOnlineModule::GetCVarOnlineRequestReturnedStatus()
{
	return CVarOnlineRequestReturnedStatus.ToSharedRef();
}

UObject* FAVVMOnlineModule::GetJsonParser()
{
	return JsonParser.Get();
}

IMPLEMENT_MODULE(FAVVMOnlineModule, AVVMOnline)
