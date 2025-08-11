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

#include "InventorySample.h"

#include "InventorySettings.h"
#include "InventoryStringParser.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY(LogInventorySample);

TStrongObjectPtr<UInventoryStringParser> FInventorySampleModule::JsonParser = nullptr;

void FInventorySampleModule::StartupModule()
{
	IModuleInterface::StartupModule();
}

void FInventorySampleModule::ShutdownModule()
{
	IModuleInterface::ShutdownModule();

	JsonParser.Reset();
}

UInventoryStringParser* FInventorySampleModule::GetJsonParser()
{
	if (!JsonParser.IsValid())
	{
		auto* Parser = NewObject<UInventoryStringParser>(GEngine, UInventorySettings::GetJsonParserClass());
		JsonParser = TStrongObjectPtr<UInventoryStringParser>(Parser);
	}

	return JsonParser.Get();
}

IMPLEMENT_MODULE(FInventorySampleModule, InventorySample)
