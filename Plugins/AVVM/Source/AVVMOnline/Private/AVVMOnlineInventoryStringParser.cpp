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
#include "AVVMOnlineInventoryStringParser.h"

#include "Backend/AVVMOnlineInventory.h"
#include "Backend/AVVMOnlineInventoryProxy.h"
#include "Serialization/JsonSerializer.h"

void UAVVMOnlineInventoryStringParser::FromString(const FString& NewPayload,
                                                  FAVVMItem& OutItem) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	FAVVMItem NewItem;
	NewItem.UniqueId = JsonData->GetIntegerField(TEXT("UniqueId"));
	NewItem.ResourceId = JsonData->GetIntegerField(TEXT("ResourceId"));

	OutItem = NewItem;
}

void UAVVMOnlineInventoryStringParser::ToString(const FAVVMItem& NewItem,
                                                FString& OutFormat) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetNumberField(TEXT("UniqueId"), NewItem.UniqueId);
	JsonData->SetNumberField(TEXT("ResourceId"), NewItem.ResourceId);

	FString JsonOutput;

	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = JsonOutput;
}

void UAVVMOnlineInventoryStringParser::FromString(const FString& NewPayload,
                                                  FAVVMItemProxy& OutItem) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	FAVVMItemProxy NewItemProxy;
	NewItemProxy.UniqueId = JsonData->GetIntegerField(TEXT("UniqueId"));
	NewItemProxy.ResourceId = JsonData->GetStringField(TEXT("ResourceId"));

	OutItem = NewItemProxy;
}

void UAVVMOnlineInventoryStringParser::ToString(const FAVVMItemProxy& NewItem,
                                                FString& OutFormat) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetNumberField(TEXT("UniqueId"), NewItem.UniqueId);
	JsonData->SetStringField(TEXT("ResourceId"), NewItem.ResourceId);

	FString JsonOutput;

	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = JsonOutput;
}
