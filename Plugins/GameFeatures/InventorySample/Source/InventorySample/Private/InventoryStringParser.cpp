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
	NewItemHolder.ResourceId = JsonData->GetStringField(TEXT("ResourceId"));

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
	JsonData->SetStringField(TEXT("ResourceId"), NewItemHolder.ResourceId);
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
	NewItem.ResourceId = JsonData->GetStringField(TEXT("ResourceId"));

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
	JsonData->SetStringField(TEXT("ResourceId"), NewItem.ResourceId);
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
	NewItemModifier.ResourceId = JsonData->GetStringField(TEXT("ResourceId"));

	OutItemModifier = NewItemModifier;
}

void UInventoryStringParser::ToString(const FItemModifier& NewItemModifier,
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
