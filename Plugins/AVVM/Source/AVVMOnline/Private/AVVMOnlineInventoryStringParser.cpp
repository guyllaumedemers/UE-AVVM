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

#include "Backend/AVVMOnlineActorContent.h"
#include "Backend/AVVMOnlineActorContentProxy.h"
#include "Serialization/JsonSerializer.h"

void UAVVMOnlineInventoryStringParser::FromString(const FString& NewPayload,
                                                  FAVVMActorContent& OutActorContent) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	FAVVMActorContent NewActorContent;
	NewActorContent.UniqueId = JsonData->GetIntegerField(TEXT("UniqueId"));

	const TArray<TSharedPtr<FJsonValue>> JsonValues = JsonData->GetArrayField(TEXT("ItemHolderIds"));
	for (const auto& JsonValue : JsonValues)
	{
		NewActorContent.ItemHolderIds.Add(JsonValue->AsNumber());
	}

	OutActorContent = NewActorContent;
}

void UAVVMOnlineInventoryStringParser::ToString(const FAVVMActorContent& NewActorContent,
                                                FString& OutFormat) const
{
	TArray<TSharedPtr<FJsonValue>> ItemHolderIds;
	for (const int32 ItemHolderId : NewActorContent.ItemHolderIds)
	{
		ItemHolderIds.Add(MakeShareable(new FJsonValueNumber(ItemHolderId)));
	}

	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetNumberField(TEXT("UniqueId"), NewActorContent.UniqueId);
	JsonData->SetArrayField(TEXT("ItemHolderIds"), ItemHolderIds);

	FString JsonOutput;

	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = JsonOutput;
}

void UAVVMOnlineInventoryStringParser::FromString(const FString& NewPayload,
                                                  FAVVMItemHolder& OutItemHolder) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	FAVVMItemHolder NewItemHolder;
	NewItemHolder.UniqueId = JsonData->GetIntegerField(TEXT("UniqueId"));

	const TArray<TSharedPtr<FJsonValue>> JsonValues = JsonData->GetArrayField(TEXT("ItemIds"));
	for (const auto& JsonValue : JsonValues)
	{
		NewItemHolder.ItemIds.Add(JsonValue->AsNumber());
	}

	OutItemHolder = NewItemHolder;
}

void UAVVMOnlineInventoryStringParser::ToString(const FAVVMItemHolder& NewItemHolder,
                                                FString& OutFormat) const
{
	TArray<TSharedPtr<FJsonValue>> ItemIds;
	for (const int32 ItemId : NewItemHolder.ItemIds)
	{
		ItemIds.Add(MakeShareable(new FJsonValueNumber(ItemId)));
	}

	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetNumberField(TEXT("UniqueId"), NewItemHolder.UniqueId);
	JsonData->SetArrayField(TEXT("ItemIds"), ItemIds);

	FString JsonOutput;

	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = JsonOutput;
}

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

	const TArray<TSharedPtr<FJsonValue>> JsonValues = JsonData->GetArrayField(TEXT("ModIds"));
	for (const auto& JsonValue : JsonValues)
	{
		NewItem.ModIds.Add(JsonValue->AsNumber());
	}

	OutItem = NewItem;
}

void UAVVMOnlineInventoryStringParser::ToString(const FAVVMItem& NewItem,
                                                FString& OutFormat) const
{
	TArray<TSharedPtr<FJsonValue>> ModIds;
	for (const int32 ModId : NewItem.ModIds)
	{
		ModIds.Add(MakeShareable(new FJsonValueNumber(ModId)));
	}

	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetNumberField(TEXT("UniqueId"), NewItem.UniqueId);
	JsonData->SetNumberField(TEXT("ResourceId"), NewItem.ResourceId);
	JsonData->SetArrayField(TEXT("ModIds"), ModIds);

	FString JsonOutput;

	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = JsonOutput;
}

void UAVVMOnlineInventoryStringParser::FromString(const FString& NewPayload,
                                                  FAVVMItemModifier& OutItemModifier) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	FAVVMItemModifier NewItemModifier;
	NewItemModifier.UniqueId = JsonData->GetIntegerField(TEXT("UniqueId"));
	NewItemModifier.ResourceId = JsonData->GetIntegerField(TEXT("ResourceId"));

	OutItemModifier = NewItemModifier;
}

void UAVVMOnlineInventoryStringParser::ToString(const FAVVMItemModifier& NewItemModifier,
                                                FString& OutFormat) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetNumberField(TEXT("UniqueId"), NewItemModifier.UniqueId);
	JsonData->SetNumberField(TEXT("ResourceId"), NewItemModifier.ResourceId);

	FString JsonOutput;

	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = JsonOutput;
}

void UAVVMOnlineInventoryStringParser::FromString(const FString& NewPayload,
                                                  FAVVMActorContentProxy& OutActorContent) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	FAVVMActorContentProxy NewActorContentProxy;
	NewActorContentProxy.UniqueId = JsonData->GetIntegerField(TEXT("UniqueId"));

	const TArray<TSharedPtr<FJsonValue>> JsonValues = JsonData->GetArrayField(TEXT("ItemHolderValues"));
	for (const auto& JsonValue : JsonValues)
	{
		NewActorContentProxy.ItemHolderValues.Add(JsonValue->AsString());
	}

	OutActorContent = NewActorContentProxy;
}

void UAVVMOnlineInventoryStringParser::ToString(const FAVVMActorContentProxy& NewActorContent,
                                                FString& OutFormat) const
{
	TArray<TSharedPtr<FJsonValue>> ItemHolderValues;
	for (const FString& ItemHolderValue : NewActorContent.ItemHolderValues)
	{
		ItemHolderValues.Add(MakeShareable(new FJsonValueString(ItemHolderValue)));
	}

	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetNumberField(TEXT("UniqueId"), NewActorContent.UniqueId);
	JsonData->SetArrayField(TEXT("ItemHolderValues"), ItemHolderValues);

	FString JsonOutput;

	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = JsonOutput;
}

void UAVVMOnlineInventoryStringParser::FromString(const FString& NewPayload,
                                                  FAVVMItemHolderProxy& OutItemHolder) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	FAVVMItemHolderProxy NewItemHolderProxy;
	NewItemHolderProxy.UniqueId = JsonData->GetIntegerField(TEXT("UniqueId"));

	const TArray<TSharedPtr<FJsonValue>> JsonValues = JsonData->GetArrayField(TEXT("ItemValues"));
	for (const auto& JsonValue : JsonValues)
	{
		NewItemHolderProxy.ItemValues.Add(JsonValue->AsString());
	}

	OutItemHolder = NewItemHolderProxy;
}

void UAVVMOnlineInventoryStringParser::ToString(const FAVVMItemHolderProxy& NewItemHolder,
                                                FString& OutFormat) const
{
	TArray<TSharedPtr<FJsonValue>> ItemValues;
	for (const FString& ItemValue : NewItemHolder.ItemValues)
	{
		ItemValues.Add(MakeShareable(new FJsonValueString(ItemValue)));
	}

	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetNumberField(TEXT("UniqueId"), NewItemHolder.UniqueId);
	JsonData->SetArrayField(TEXT("ItemValues"), ItemValues);

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

	const TArray<TSharedPtr<FJsonValue>> JsonValues = JsonData->GetArrayField(TEXT("ModValues"));
	for (const auto& JsonValue : JsonValues)
	{
		NewItemProxy.ModValues.Add(JsonValue->AsString());
	}

	OutItem = NewItemProxy;
}

void UAVVMOnlineInventoryStringParser::ToString(const FAVVMItemProxy& NewItem,
                                                FString& OutFormat) const
{
	TArray<TSharedPtr<FJsonValue>> ModValues;
	for (const FString& ModValue : NewItem.ModValues)
	{
		ModValues.Add(MakeShareable(new FJsonValueString(ModValue)));
	}

	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetNumberField(TEXT("UniqueId"), NewItem.UniqueId);
	JsonData->SetStringField(TEXT("ResourceId"), NewItem.ResourceId);
	JsonData->SetArrayField(TEXT("ModValues"), ModValues);

	FString JsonOutput;

	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = JsonOutput;
}

void UAVVMOnlineInventoryStringParser::FromString(const FString& NewPayload,
                                                  FAVVMItemModifierProxy& OutItemModifier) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	FAVVMItemModifierProxy NewItemModifierProxy;
	NewItemModifierProxy.UniqueId = JsonData->GetIntegerField(TEXT("UniqueId"));
	NewItemModifierProxy.ResourceId = JsonData->GetStringField(TEXT("ResourceId"));

	OutItemModifier = NewItemModifierProxy;
}

void UAVVMOnlineInventoryStringParser::ToString(const FAVVMItemModifierProxy& NewItemModifier,
                                                FString& OutFormat) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetNumberField(TEXT("UniqueId"), NewItemModifier.UniqueId);
	JsonData->SetStringField(TEXT("ResourceId"), NewItemModifier.ResourceId);

	FString JsonOutput;

	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = JsonOutput;
}
