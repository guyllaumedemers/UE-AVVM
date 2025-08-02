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
#include "InventoryStringParser.h"

#include "Backend/ActorContent.h"
#include "Backend/ActorContentProxy.h"

void UInventoryStringParser::FromString(const FString& NewPayload,
                                        FActorContent& OutActorContent) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	FActorContent NewActorContent;
	NewActorContent.UniqueId = JsonData->GetIntegerField(TEXT("UniqueId"));

	const TArray<TSharedPtr<FJsonValue>> JsonValues = JsonData->GetArrayField(TEXT("ItemHolderIds"));
	for (const auto& JsonValue : JsonValues)
	{
		NewActorContent.ItemHolderIds.Add(JsonValue->AsNumber());
	}

	OutActorContent = NewActorContent;
}

void UInventoryStringParser::ToString(const FActorContent& NewActorContent,
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

void UInventoryStringParser::FromString(const FString& NewPayload,
                                        FItemHolder& OutItemHolder) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	FItemHolder NewItemHolder;
	NewItemHolder.UniqueId = JsonData->GetIntegerField(TEXT("UniqueId"));
	NewItemHolder.ResourceId = JsonData->GetIntegerField(TEXT("ResourceId"));

	const TArray<TSharedPtr<FJsonValue>> JsonValues = JsonData->GetArrayField(TEXT("ItemIds"));
	for (const auto& JsonValue : JsonValues)
	{
		NewItemHolder.ItemIds.Add(JsonValue->AsNumber());
	}

	OutItemHolder = NewItemHolder;
}

void UInventoryStringParser::ToString(const FItemHolder& NewItemHolder,
                                      FString& OutFormat) const
{
	TArray<TSharedPtr<FJsonValue>> ItemIds;
	for (const int32 ItemId : NewItemHolder.ItemIds)
	{
		ItemIds.Add(MakeShareable(new FJsonValueNumber(ItemId)));
	}

	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetNumberField(TEXT("UniqueId"), NewItemHolder.UniqueId);
	JsonData->SetNumberField(TEXT("ResourceId"), NewItemHolder.ResourceId);
	JsonData->SetArrayField(TEXT("ItemIds"), ItemIds);

	FString JsonOutput;

	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = JsonOutput;
}

void UInventoryStringParser::FromString(const FString& NewPayload,
                                        FItem& OutItem) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	FItem NewItem;
	NewItem.UniqueId = JsonData->GetIntegerField(TEXT("UniqueId"));
	NewItem.ResourceId = JsonData->GetIntegerField(TEXT("ResourceId"));

	const TArray<TSharedPtr<FJsonValue>> JsonValues = JsonData->GetArrayField(TEXT("ModIds"));
	for (const auto& JsonValue : JsonValues)
	{
		NewItem.ModIds.Add(JsonValue->AsNumber());
	}

	OutItem = NewItem;
}

void UInventoryStringParser::ToString(const FItem& NewItem,
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

void UInventoryStringParser::FromString(const FString& NewPayload,
                                        FItemModifier& OutItemModifier) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	FItemModifier NewItemModifier;
	NewItemModifier.UniqueId = JsonData->GetIntegerField(TEXT("UniqueId"));
	NewItemModifier.ResourceId = JsonData->GetIntegerField(TEXT("ResourceId"));

	OutItemModifier = NewItemModifier;
}

void UInventoryStringParser::ToString(const FItemModifier& NewItemModifier,
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

void UInventoryStringParser::FromString(const FString& NewPayload,
                                        FActorContentProxy& OutActorContent) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	FActorContentProxy NewActorContentProxy;
	NewActorContentProxy.UniqueId = JsonData->GetIntegerField(TEXT("UniqueId"));

	const TArray<TSharedPtr<FJsonValue>> JsonValues = JsonData->GetArrayField(TEXT("ItemHolderValues"));
	for (const auto& JsonValue : JsonValues)
	{
		NewActorContentProxy.ItemHolderValues.Add(JsonValue->AsString());
	}

	OutActorContent = NewActorContentProxy;
}

void UInventoryStringParser::ToString(const FActorContentProxy& NewActorContent,
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

void UInventoryStringParser::FromString(const FString& NewPayload,
                                        FItemHolderProxy& OutItemHolder) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	FItemHolderProxy NewItemHolderProxy;
	NewItemHolderProxy.UniqueId = JsonData->GetIntegerField(TEXT("UniqueId"));
	NewItemHolderProxy.ResourceId = JsonData->GetStringField(TEXT("ResourceId"));

	const TArray<TSharedPtr<FJsonValue>> JsonValues = JsonData->GetArrayField(TEXT("ItemValues"));
	for (const auto& JsonValue : JsonValues)
	{
		NewItemHolderProxy.ItemValues.Add(JsonValue->AsString());
	}

	OutItemHolder = NewItemHolderProxy;
}

void UInventoryStringParser::ToString(const FItemHolderProxy& NewItemHolder,
                                      FString& OutFormat) const
{
	TArray<TSharedPtr<FJsonValue>> ItemValues;
	for (const FString& ItemValue : NewItemHolder.ItemValues)
	{
		ItemValues.Add(MakeShareable(new FJsonValueString(ItemValue)));
	}

	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetNumberField(TEXT("UniqueId"), NewItemHolder.UniqueId);
	JsonData->SetStringField(TEXT("ResourceId"), NewItemHolder.ResourceId);
	JsonData->SetArrayField(TEXT("ItemValues"), ItemValues);

	FString JsonOutput;

	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = JsonOutput;
}

void UInventoryStringParser::FromString(const FString& NewPayload,
                                        FItemProxy& OutItem) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	FItemProxy NewItemProxy;
	NewItemProxy.UniqueId = JsonData->GetIntegerField(TEXT("UniqueId"));
	NewItemProxy.ResourceId = JsonData->GetStringField(TEXT("ResourceId"));

	const TArray<TSharedPtr<FJsonValue>> JsonValues = JsonData->GetArrayField(TEXT("ModValues"));
	for (const auto& JsonValue : JsonValues)
	{
		NewItemProxy.ModValues.Add(JsonValue->AsString());
	}

	OutItem = NewItemProxy;
}

void UInventoryStringParser::ToString(const FItemProxy& NewItem,
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

void UInventoryStringParser::FromString(const FString& NewPayload,
                                        FItemModifierProxy& OutItemModifier) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	FItemModifierProxy NewItemModifierProxy;
	NewItemModifierProxy.UniqueId = JsonData->GetIntegerField(TEXT("UniqueId"));
	NewItemModifierProxy.ResourceId = JsonData->GetStringField(TEXT("ResourceId"));

	OutItemModifier = NewItemModifierProxy;
}

void UInventoryStringParser::ToString(const FItemModifierProxy& NewItemModifier,
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
