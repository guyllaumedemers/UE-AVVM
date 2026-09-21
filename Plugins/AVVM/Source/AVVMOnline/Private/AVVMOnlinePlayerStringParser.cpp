///Copyright(c) 2025 gdemers
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
#include "AVVMOnlinePlayerStringParser.h"

#include "AVVMOnlineInterface.h"
#include "Backend/AVVMOnlinePlayer.h"
#include "Backend/AVVMOnlinePlayerProxy.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

void UAVVMOnlinePlayerStringParser::FromString(const FString& NewPayload,
                                               FAVVMPlayerLoginContext& OutPlayerLoginContext) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	FAVVMPlayerLoginContext NewPlayerLoginContext{};
	NewPlayerLoginContext.UniqueId = JsonData->GetIntegerField(TEXT("UniqueId"));
	NewPlayerLoginContext.Username = JsonData->GetStringField(TEXT("Username"));
	NewPlayerLoginContext.Password = JsonData->GetStringField(TEXT("Password"));

	OutPlayerLoginContext = MoveTemp(NewPlayerLoginContext);
}

void UAVVMOnlinePlayerStringParser::ToString(const FAVVMPlayerLoginContext& NewPlayerLoginContext,
                                             FString& OutFormat) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetNumberField(TEXT("UniqueId"), NewPlayerLoginContext.UniqueId);
	JsonData->SetStringField(TEXT("Username"), NewPlayerLoginContext.Username);
	JsonData->SetStringField(TEXT("Password"), NewPlayerLoginContext.Password);

	FString JsonOutput{};
	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = MoveTemp(JsonOutput);
}

void UAVVMOnlinePlayerStringParser::FromString(const FString& NewPayload,
                                               FAVVMPlayerAccount& OutPlayerAccount) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	FAVVMPlayerAccount NewPlayerAccount{};
	NewPlayerAccount.UniqueId = JsonData->GetIntegerField(TEXT("UniqueId"));
	NewPlayerAccount.LoginId = JsonData->GetIntegerField(TEXT("LoginId"));
	NewPlayerAccount.Gamertag = JsonData->GetStringField(TEXT("Gamertag"));
	NewPlayerAccount.WalletId = JsonData->GetIntegerField(TEXT("WalletId"));
	for (const auto& ProfileId : JsonData->GetArrayField(TEXT("ProfileIds")))
	{
		NewPlayerAccount.ProfileIds.Add(ProfileId->AsNumber());
	}

	for (const auto& PresetId : JsonData->GetArrayField(TEXT("PresetIds")))
	{
		NewPlayerAccount.PresetIds.Add(PresetId->AsNumber());
	}

	OutPlayerAccount = MoveTemp(NewPlayerAccount);
}

void UAVVMOnlinePlayerStringParser::ToString(const FAVVMPlayerAccount& NewPlayerAccount,
                                             FString& OutFormat) const
{
	TArray<TSharedPtr<FJsonValue>> ProfileIds{};
	for (const int32 ProfileId : NewPlayerAccount.ProfileIds)
	{
		ProfileIds.Add(MakeShareable(new FJsonValueNumber(ProfileId)));
	}

	TArray<TSharedPtr<FJsonValue>> PresetIds{};
	for (const int32 PresetId : NewPlayerAccount.PresetIds)
	{
		PresetIds.Add(MakeShareable(new FJsonValueNumber(PresetId)));
	}

	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetNumberField(TEXT("UniqueId"), NewPlayerAccount.UniqueId);
	JsonData->SetNumberField(TEXT("LoginId"), NewPlayerAccount.LoginId);
	JsonData->SetStringField(TEXT("Gamertag"), NewPlayerAccount.Gamertag);
	JsonData->SetNumberField(TEXT("WalletId"), NewPlayerAccount.WalletId);
	JsonData->SetArrayField(TEXT("ProfileIds"), ProfileIds);
	JsonData->SetArrayField(TEXT("PresetIds"), PresetIds);

	FString JsonOutput{};
	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = MoveTemp(JsonOutput);
}

void UAVVMOnlinePlayerStringParser::FromString(const FString& NewPayload,
                                               FAVVMPlayerWallet& OutPlayerWallet) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	FAVVMPlayerWallet NewPlayerWallet{};
	NewPlayerWallet.UniqueId = JsonData->GetIntegerField(TEXT("UniqueId"));
	for (const auto& JsonValue : JsonData->GetArrayField(TEXT("IrlMoneys")))
	{
		NewPlayerWallet.IrlMoneys.Add(JsonValue->AsString());
	}

	OutPlayerWallet = MoveTemp(NewPlayerWallet);
}

void UAVVMOnlinePlayerStringParser::ToString(const FAVVMPlayerWallet& NewPlayerWallet,
                                             FString& OutFormat) const
{
	TArray<TSharedPtr<FJsonValue>> JsonValues{};
	for (const FString& IrlMoney : NewPlayerWallet.IrlMoneys)
	{
		JsonValues.Add(MakeShareable(new FJsonValueString(IrlMoney)));
	}

	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetNumberField(TEXT("UniqueId"), NewPlayerWallet.UniqueId);
	JsonData->SetArrayField(TEXT("IrlMoneys"), JsonValues);

	FString JsonOutput{};
	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = MoveTemp(JsonOutput);
}

void UAVVMOnlinePlayerStringParser::FromString(const FString& NewPayload,
                                               FAVVMCurrency& OutCurrency) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	FAVVMCurrency NewCurrency{};
	NewCurrency.CurrencyId = JsonData->GetStringField(TEXT("CurrencyId"));
	NewCurrency.TotalAmount = JsonData->GetIntegerField(TEXT("TotalAmount"));

	OutCurrency = MoveTemp(NewCurrency);
}

void UAVVMOnlinePlayerStringParser::ToString(const FAVVMCurrency& NewCurrency,
                                             FString& OutFormat) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetStringField(TEXT("CurrencyId"), NewCurrency.CurrencyId);
	JsonData->SetNumberField(TEXT("TotalAmount"), NewCurrency.TotalAmount);

	FString JsonOutput{};
	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = MoveTemp(JsonOutput);
}

void UAVVMOnlinePlayerStringParser::FromString(const FString& NewPayload,
                                               FAVVMPlayerProfile& OutPlayerProfile) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	FAVVMPlayerProfile NewPlayerProfile{};
	NewPlayerProfile.UniqueId = JsonData->GetIntegerField(TEXT("UniqueId"));
	NewPlayerProfile.ProfileId = JsonData->GetStringField(TEXT("ProfileId"));
	NewPlayerProfile.EquippedPresetId = JsonData->GetIntegerField(TEXT("EquippedPresetId"));

	for (const auto& InventoryId : JsonData->GetArrayField(TEXT("InventoryIds")))
	{
		NewPlayerProfile.InventoryIds.Add(InventoryId->AsNumber());
	}

	for (const auto& SkinId : JsonData->GetArrayField(TEXT("SkinIds")))
	{
		NewPlayerProfile.SkinIds.Add(SkinId->AsNumber());
	}

	for (const auto& CharmsId : JsonData->GetArrayField(TEXT("CharmsIds")))
	{
		NewPlayerProfile.CharmsIds.Add(CharmsId->AsNumber());
	}

	for (const auto& SkillId : JsonData->GetArrayField(TEXT("SkillIds")))
	{
		NewPlayerProfile.SkillIds.Add(SkillId->AsNumber());
	}

	for (const auto& ChallengeId : JsonData->GetArrayField(TEXT("ChallengeIds")))
	{
		NewPlayerProfile.ChallengeIds.Add(ChallengeId->AsNumber());
	}

	for (const auto& InventoryDependencyGraphEntry : JsonData->GetArrayField(TEXT("InventoryDependencyGraph")))
	{
		NewPlayerProfile.InventoryDependencyGraph.Add(InventoryDependencyGraphEntry->AsNumber());
	}

	for (const auto& SkillDependencyGraphEntry : JsonData->GetArrayField(TEXT("SkillDependencyGraph")))
	{
		NewPlayerProfile.SkillDependencyGraph.Add(SkillDependencyGraphEntry->AsNumber());
	}

	OutPlayerProfile = MoveTemp(NewPlayerProfile);
}

void UAVVMOnlinePlayerStringParser::ToString(const FAVVMPlayerProfile& NewPlayerProfile,
                                             FString& OutFormat) const
{
	TArray<TSharedPtr<FJsonValue>> InventoryIds{};
	for (const int32 InventoryId : NewPlayerProfile.InventoryIds)
	{
		InventoryIds.Add(MakeShareable(new FJsonValueNumber(InventoryId)));
	}

	TArray<TSharedPtr<FJsonValue>> SkinIds{};
	for (const int32 SkinId : NewPlayerProfile.SkinIds)
	{
		SkinIds.Add(MakeShareable(new FJsonValueNumber(SkinId)));
	}

	TArray<TSharedPtr<FJsonValue>> CharmsIds{};
	for (const int32 CharmsId : NewPlayerProfile.CharmsIds)
	{
		CharmsIds.Add(MakeShareable(new FJsonValueNumber(CharmsId)));
	}

	TArray<TSharedPtr<FJsonValue>> SkillIds{};
	for (const int32 SkillId : NewPlayerProfile.SkillIds)
	{
		SkillIds.Add(MakeShareable(new FJsonValueNumber(SkillId)));
	}

	TArray<TSharedPtr<FJsonValue>> ChallengeIds{};
	for (const int32 ChallengeId : NewPlayerProfile.ChallengeIds)
	{
		ChallengeIds.Add(MakeShareable(new FJsonValueNumber(ChallengeId)));
	}

	TArray<TSharedPtr<FJsonValue>> InventoryDependencyGraph;
	for (const int32 InventoryDependencyGraphEntry : NewPlayerProfile.InventoryDependencyGraph)
	{
		InventoryDependencyGraph.Add(MakeShareable(new FJsonValueNumber(InventoryDependencyGraphEntry)));
	}

	TArray<TSharedPtr<FJsonValue>> SkillDependencyGraph;
	for (const int32 SkillDependencyGraphEntry : NewPlayerProfile.SkillDependencyGraph)
	{
		SkillDependencyGraph.Add(MakeShareable(new FJsonValueNumber(SkillDependencyGraphEntry)));
	}

	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetNumberField(TEXT("UniqueId"), NewPlayerProfile.UniqueId);
	JsonData->SetStringField(TEXT("ProfileId"), NewPlayerProfile.ProfileId);
	JsonData->SetArrayField(TEXT("InventoryIds"), InventoryIds);
	JsonData->SetArrayField(TEXT("SkinIds"), SkinIds);
	JsonData->SetArrayField(TEXT("CharmsIds"), CharmsIds);
	JsonData->SetArrayField(TEXT("SkillIds"), SkillIds);
	JsonData->SetArrayField(TEXT("ChallengeIds"), ChallengeIds);
	JsonData->SetNumberField(TEXT("EquippedPresetId"), NewPlayerProfile.EquippedPresetId);
	JsonData->SetArrayField(TEXT("InventoryDependencyGraph"), InventoryDependencyGraph);
	JsonData->SetArrayField(TEXT("SkillDependencyGraph"), SkillDependencyGraph);

	FString JsonOutput{};
	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = MoveTemp(JsonOutput);
}

void UAVVMOnlinePlayerStringParser::FromString(const FString& NewPayload,
                                               FAVVMPlayerPreset& OutPlayerPreset) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	FAVVMPlayerPreset NewPlayerPreset{};
	NewPlayerPreset.UniqueId = JsonData->GetIntegerField(TEXT("UniqueId"));
	NewPlayerPreset.PresetId = JsonData->GetStringField(TEXT("PresetId"));
	for (const auto& JsonValue : JsonData->GetArrayField(TEXT("EquippedItems")))
	{
		const TSharedPtr<FJsonObject>* OutKVPJsonObject = nullptr;
		JsonValue->TryGetObject(OutKVPJsonObject);

		if ((OutKVPJsonObject != nullptr) && OutKVPJsonObject->IsValid())
		{
			const FString GameplayTag = (*OutKVPJsonObject)->GetStringField(TEXT("SlotTag"));
			const int32 ItemId = (*OutKVPJsonObject)->GetIntegerField(TEXT("ItemId"));
			NewPlayerPreset.EquippedItems.FindOrAdd(FGameplayTag::RequestGameplayTag(FName(GameplayTag)), ItemId);
		}
	}

	OutPlayerPreset = MoveTemp(NewPlayerPreset);
}

void UAVVMOnlinePlayerStringParser::ToString(const FAVVMPlayerPreset& NewPlayerPreset,
                                             FString& OutFormat) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetNumberField(TEXT("UniqueId"), NewPlayerPreset.UniqueId);
	JsonData->SetStringField(TEXT("PresetId"), NewPlayerPreset.PresetId);

	TArray<TSharedPtr<FJsonValue>> EquippedItems{};
	for (const auto& [SlotTag, ItemId] : NewPlayerPreset.EquippedItems)
	{
		TSharedPtr<FJsonObject> KVPJsonObject = MakeShareable(new FJsonObject);
		KVPJsonObject->SetStringField(TEXT("SlotTag"), SlotTag.ToString());
		KVPJsonObject->SetNumberField("ItemId", ItemId);
		EquippedItems.Add(MakeShareable(new FJsonValueObject(MoveTemp(KVPJsonObject))));
	}

	JsonData->SetArrayField(TEXT("EquippedItems"), EquippedItems);

	FString JsonOutput{};
	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = MoveTemp(JsonOutput);
}

void UAVVMOnlinePlayerStringParser::FromString(const FAVVMStringPayload& NewPayload,
                                               TArray<FAVVMPlayerResource>& OutPlayerResources) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload.Payload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	OutPlayerResources.Reset();
	for (const auto& JsonValue : JsonData->GetArrayField(TEXT("PlayerResources")))
	{
		FAVVMPlayerResource OutResource{};
		FromString(JsonValue->AsString(), OutResource);
		OutPlayerResources.Add(MoveTemp(OutResource));
	}
}

void UAVVMOnlinePlayerStringParser::ToString(const TArray<FAVVMPlayerResource>& NewPlayerResources,
                                             FString& OutFormat) const
{
	TArray<TSharedPtr<FJsonValue>> JsonValues{};
	for (const FAVVMPlayerResource& PlayerResource : NewPlayerResources)
	{
		FString OutValue{};
		ToString(PlayerResource, OutValue);

		JsonValues.Add(MakeShareable(new FJsonValueString(MoveTemp(OutValue))));
	}

	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetArrayField(TEXT("PlayerResources"), JsonValues);

	FString JsonOutput{};
	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = MoveTemp(JsonOutput);
}

void UAVVMOnlinePlayerStringParser::FromString(const FString& NewPayload,
                                               FAVVMPlayerResource& OutPlayerResource) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	FAVVMPlayerResource NewPlayerResource{};
	NewPlayerResource.UniqueId = JsonData->GetIntegerField(TEXT("UniqueId"));
	NewPlayerResource.ResourceId = JsonData->GetStringField(TEXT("ResourceId"));

	OutPlayerResource = MoveTemp(NewPlayerResource);
}

void UAVVMOnlinePlayerStringParser::ToString(const FAVVMPlayerResource& NewPlayerResource,
                                             FString& OutFormat) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetNumberField(TEXT("UniqueId"), NewPlayerResource.UniqueId);
	JsonData->SetStringField(TEXT("ResourceId"), NewPlayerResource.ResourceId);

	FString JsonOutput{};
	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = MoveTemp(JsonOutput);
}

void UAVVMOnlinePlayerStringParser::FromString(const FAVVMStringPayload& NewPayload,
                                               TArray<FAVVMPlayerChallenge>& OutPlayerChallenges) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload.Payload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	OutPlayerChallenges.Reset();
	for (const auto& JsonValue : JsonData->GetArrayField(TEXT("PlayerChallenges")))
	{
		FAVVMPlayerChallenge OutPlayerChallenge{};
		FromString(JsonValue->AsString(), OutPlayerChallenge);
		OutPlayerChallenges.Add(MoveTemp(OutPlayerChallenge));
	}
}

void UAVVMOnlinePlayerStringParser::ToString(const TArray<FAVVMPlayerChallenge>& NewPlayerChallenges,
                                             FString& OutFormat) const
{
	TArray<TSharedPtr<FJsonValue>> JsonValues{};
	for (const FAVVMPlayerChallenge& PlayerChallenge : NewPlayerChallenges)
	{
		FString OutValue{};
		ToString(PlayerChallenge, OutValue);

		JsonValues.Add(MakeShareable(new FJsonValueString(MoveTemp(OutValue))));
	}

	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetArrayField(TEXT("PlayerChallenges"), JsonValues);

	FString JsonOutput{};
	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = MoveTemp(JsonOutput);
}

void UAVVMOnlinePlayerStringParser::FromString(const FString& NewPayload,
                                               FAVVMPlayerChallenge& OutPlayerChallenge) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	FAVVMPlayerChallenge NewPlayerChallenge{};
	NewPlayerChallenge.UniqueId = JsonData->GetIntegerField(TEXT("UniqueId"));
	NewPlayerChallenge.ChallengeId = JsonData->GetStringField(TEXT("ChallengeId"));

	OutPlayerChallenge = MoveTemp(NewPlayerChallenge);
}

void UAVVMOnlinePlayerStringParser::ToString(const FAVVMPlayerChallenge& NewPlayerChallenge,
                                             FString& OutFormat) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetNumberField(TEXT("UniqueId"), NewPlayerChallenge.UniqueId);
	JsonData->SetStringField(TEXT("ChallengeId"), NewPlayerChallenge.ChallengeId);

	FString JsonOutput{};
	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = MoveTemp(JsonOutput);
}

void UAVVMOnlinePlayerStringParser::FromString(const FAVVMStringPayload& NewPayload,
                                               TArray<FAVVMParty>& OutParties) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload.Payload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	OutParties.Reset();
	for (const auto& JsonValue : JsonData->GetArrayField(TEXT("Parties")))
	{
		FAVVMParty OutParty{};
		FromString(JsonValue->AsString(), OutParty);
		OutParties.Add(MoveTemp(OutParty));
	}
}

void UAVVMOnlinePlayerStringParser::ToString(const TArray<FAVVMParty>& NewParties,
                                             FString& OutFormat) const
{
	TArray<TSharedPtr<FJsonValue>> JsonValues{};
	for (const FAVVMParty& Party : NewParties)
	{
		FString OutValue{};
		ToString(Party, OutValue);

		JsonValues.Add(MakeShareable(new FJsonValueString(MoveTemp(OutValue))));
	}

	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetArrayField(TEXT("Parties"), JsonValues);

	FString JsonOutput{};
	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = MoveTemp(JsonOutput);
}

void UAVVMOnlinePlayerStringParser::FromString(const FString& NewPayload,
                                               FAVVMParty& OutParty) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	FAVVMParty NewParty{};
	NewParty.UniqueId = JsonData->GetIntegerField(TEXT("UniqueId"));
	NewParty.PartyId = JsonData->GetStringField(TEXT("PartyId"));
	NewParty.RegionId = JsonData->GetIntegerField(TEXT("RegionId"));
	NewParty.DistrictId = JsonData->GetIntegerField(TEXT("DistrictId"));
	NewParty.HostConfigurationId = JsonData->GetIntegerField(TEXT("HostConfigurationId"));
	for (const auto& PlayerConnectionId : JsonData->GetArrayField(TEXT("PlayerConnectionIds")))
	{
		NewParty.PlayerConnectionIds.Add(PlayerConnectionId->AsNumber());
	}

	OutParty = MoveTemp(NewParty);
}

void UAVVMOnlinePlayerStringParser::ToString(const FAVVMParty& NewParty,
                                             FString& OutFormat) const
{
	TArray<TSharedPtr<FJsonValue>> PlayerConnectionIds{};
	for (const int32 PlayerConnectionId : NewParty.PlayerConnectionIds)
	{
		PlayerConnectionIds.Add(MakeShareable(new FJsonValueNumber(PlayerConnectionId)));
	}

	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetNumberField(TEXT("UniqueId"), NewParty.UniqueId);
	JsonData->SetStringField(TEXT("PartyId"), NewParty.PartyId);
	JsonData->SetNumberField(TEXT("RegionId"), NewParty.RegionId);
	JsonData->SetNumberField(TEXT("DistrictId"), NewParty.DistrictId);
	JsonData->SetNumberField(TEXT("HostConfigurationId"), NewParty.HostConfigurationId);
	JsonData->SetArrayField(TEXT("PlayerConnectionIds"), PlayerConnectionIds);

	FString JsonOutput{};
	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = MoveTemp(JsonOutput);
}

void UAVVMOnlinePlayerStringParser::FromString(const FAVVMStringPayload& NewPayload,
                                               TArray<FAVVMPlayerConnection>& OutPlayerConnections) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload.Payload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	OutPlayerConnections.Reset();
	for (const auto& JsonValue : JsonData->GetArrayField(TEXT("PlayerConnections")))
	{
		FAVVMPlayerConnection OutPlayerConnection{};
		FromString(JsonValue->AsString(), OutPlayerConnection);
		OutPlayerConnections.Add(MoveTemp(OutPlayerConnection));
	}
}

void UAVVMOnlinePlayerStringParser::ToString(const TArray<FAVVMPlayerConnection>& NewPlayerConnections,
                                             FString& OutFormat) const
{
	TArray<TSharedPtr<FJsonValue>> JsonValues{};
	for (const FAVVMPlayerConnection& PlayerConnection : NewPlayerConnections)
	{
		FString OutValue{};
		ToString(PlayerConnection, OutValue);

		JsonValues.Add(MakeShareable(new FJsonValueString(MoveTemp(OutValue))));
	}

	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetArrayField(TEXT("PlayerConnections"), JsonValues);

	FString JsonOutput{};
	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = MoveTemp(JsonOutput);
}

void UAVVMOnlinePlayerStringParser::FromString(const FString& NewPayload,
                                               FAVVMPlayerConnection& OutPlayerConnection) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	FAVVMPlayerConnection NewPlayerConnection{};
	NewPlayerConnection.UniqueId = JsonData->GetIntegerField(TEXT("UniqueId"));
	NewPlayerConnection.UniqueNetId = JsonData->GetStringField(TEXT("UniqueNetId"));
	NewPlayerConnection.PlayerStatus = StaticCast<EAVVMPlayerStatus>(JsonData->GetIntegerField(TEXT("PlayerStatus")));
	NewPlayerConnection.ProfileId = JsonData->GetIntegerField(TEXT("ProfileId"));

	OutPlayerConnection = MoveTemp(NewPlayerConnection);
}

void UAVVMOnlinePlayerStringParser::ToString(const FAVVMPlayerConnection& NewPlayerConnection,
                                             FString& OutFormat) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetNumberField(TEXT("UniqueId"), NewPlayerConnection.UniqueId);
	JsonData->SetStringField(TEXT("UniqueNetId"), NewPlayerConnection.UniqueNetId);
	JsonData->SetNumberField(TEXT("PlayerStatus"), StaticCast<int32>(NewPlayerConnection.PlayerStatus));
	JsonData->SetNumberField(TEXT("ProfileId"), NewPlayerConnection.ProfileId);

	FString JsonOutput{};
	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = MoveTemp(JsonOutput);
}

void UAVVMOnlinePlayerStringParser::FromString(const FString& NewPayload,
                                               FAVVMHostConfiguration& OutHostConfiguration) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	FAVVMHostConfiguration NewHostConfiguration{};
	NewHostConfiguration.UniqueId = JsonData->GetIntegerField(TEXT("UniqueId"));
	NewHostConfiguration.GameMode = JsonData->GetStringField(TEXT("GameMode"));
	NewHostConfiguration.GameModeAdditiveOptions = JsonData->GetStringField(TEXT("GameModeAdditiveOptions"));

	OutHostConfiguration = MoveTemp(NewHostConfiguration);
}

void UAVVMOnlinePlayerStringParser::ToString(const FAVVMHostConfiguration& NewHostConfiguration,
                                             FString& OutFormat) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetNumberField(TEXT("UniqueId"), NewHostConfiguration.UniqueId);
	JsonData->SetStringField(TEXT("GameMode"), NewHostConfiguration.GameMode);
	JsonData->SetStringField(TEXT("GameModeAdditiveOptions"), NewHostConfiguration.GameModeAdditiveOptions);

	FString JsonOutput{};
	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = MoveTemp(JsonOutput);
}

void UAVVMOnlinePlayerStringParser::FromString(const FString& NewPayload,
                                               FAVVMPlayerAccountProxy& OutPlayerAccountProxy) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	FAVVMPlayerAccountProxy NewPlayerAccountProxy{};
	NewPlayerAccountProxy.UniqueId = JsonData->GetIntegerField(TEXT("UniqueId"));
	NewPlayerAccountProxy.Login = JsonData->GetStringField(TEXT("Login"));
	NewPlayerAccountProxy.Gamertag = JsonData->GetStringField(TEXT("Gamertag"));
	NewPlayerAccountProxy.Wallet = JsonData->GetStringField(TEXT("Wallet"));
	for (const auto& Profile : JsonData->GetArrayField(TEXT("Profiles")))
	{
		NewPlayerAccountProxy.Profiles.Add(Profile->AsString());
	}

	for (const auto& Preset : JsonData->GetArrayField(TEXT("Presets")))
	{
		NewPlayerAccountProxy.Presets.Add(Preset->AsString());
	}

	OutPlayerAccountProxy = MoveTemp(NewPlayerAccountProxy);
}

void UAVVMOnlinePlayerStringParser::ToString(const FAVVMPlayerAccountProxy& NewPlayerAccountProxy,
                                             FString& OutFormat) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetNumberField(TEXT("UniqueId"), NewPlayerAccountProxy.UniqueId);
	JsonData->SetStringField(TEXT("Login"), NewPlayerAccountProxy.Login);
	JsonData->SetStringField(TEXT("Gamertag"), NewPlayerAccountProxy.Gamertag);
	JsonData->SetStringField(TEXT("Wallet"), NewPlayerAccountProxy.Wallet);

	TArray<TSharedPtr<FJsonValue>> Profiles{};
	for (const FString& Profile : NewPlayerAccountProxy.Profiles)
	{
		Profiles.Add(MakeShareable(new FJsonValueString(Profile)));
	}

	JsonData->SetArrayField(TEXT("Profiles"), Profiles);

	TArray<TSharedPtr<FJsonValue>> Presets{};
	for (const FString& Preset : NewPlayerAccountProxy.Presets)
	{
		Presets.Add(MakeShareable(new FJsonValueString(Preset)));
	}

	JsonData->SetArrayField(TEXT("Presets"), Presets);

	FString JsonOutput{};
	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = MoveTemp(JsonOutput);
}

void UAVVMOnlinePlayerStringParser::FromString(const FString& NewPayload,
                                               FAVVMPlayerWalletProxy& OutPlayerWalletProxy) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	FAVVMPlayerWalletProxy PlayerWalletProxy{};
	PlayerWalletProxy.UniqueId = JsonData->GetIntegerField(TEXT("UniqueId"));
	for (const auto& IrlsMoney : JsonData->GetArrayField(TEXT("IrlMoneys")))
	{
		PlayerWalletProxy.IrlMoneys.Add(IrlsMoney->AsString());
	}

	OutPlayerWalletProxy = MoveTemp(PlayerWalletProxy);
}

void UAVVMOnlinePlayerStringParser::ToString(const FAVVMPlayerWalletProxy& NewPlayerWalletProxy,
                                             FString& OutFormat) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetNumberField(TEXT("UniqueId"), NewPlayerWalletProxy.UniqueId);

	TArray<TSharedPtr<FJsonValue>> IrlMoneys{};
	for (const FString& IrlMoney : NewPlayerWalletProxy.IrlMoneys)
	{
		IrlMoneys.Add(MakeShareable(new FJsonValueString(IrlMoney)));
	}

	JsonData->SetArrayField(TEXT("IrlMoneys"), IrlMoneys);

	FString JsonOutput{};
	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = MoveTemp(JsonOutput);
}

void UAVVMOnlinePlayerStringParser::FromString(const FString& NewPayload,
                                               FAVVMPlayerProfileProxy& OutPlayerProfileProxy) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	FAVVMPlayerProfileProxy PlayerProfileProxy{};
	PlayerProfileProxy.UniqueId = JsonData->GetIntegerField(TEXT("UniqueId"));
	PlayerProfileProxy.ProfileId = JsonData->GetStringField(TEXT("ProfileId"));
	PlayerProfileProxy.EquippedPreset = JsonData->GetStringField(TEXT("EquippedPreset"));

	for (const auto& Inventory : JsonData->GetArrayField(TEXT("Inventories")))
	{
		PlayerProfileProxy.Inventories.Add(Inventory->AsString());
	}

	for (const auto& Skill : JsonData->GetArrayField(TEXT("Skills")))
	{
		PlayerProfileProxy.Skills.Add(Skill->AsString());
	}

	for (const auto& Challenge : JsonData->GetArrayField(TEXT("Challenges")))
	{
		PlayerProfileProxy.Challenges.Add(Challenge->AsString());
	}

	OutPlayerProfileProxy = MoveTemp(PlayerProfileProxy);
}

void UAVVMOnlinePlayerStringParser::ToString(const FAVVMPlayerProfileProxy& NewPlayerProfileProxy,
                                             FString& OutFormat) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetNumberField(TEXT("UniqueId"), NewPlayerProfileProxy.UniqueId);
	JsonData->SetStringField(TEXT("ProfileId"), NewPlayerProfileProxy.ProfileId);
	JsonData->SetStringField(TEXT("EquippedPreset"), NewPlayerProfileProxy.EquippedPreset);

	TArray<TSharedPtr<FJsonValue>> Inventories{};
	for (const FString& Inventory : NewPlayerProfileProxy.Inventories)
	{
		Inventories.Add(MakeShareable(new FJsonValueString(Inventory)));
	}

	JsonData->SetArrayField(TEXT("Inventories"), Inventories);

	TArray<TSharedPtr<FJsonValue>> Skills;
	for (const FString& Skill : NewPlayerProfileProxy.Skills)
	{
		Skills.Add(MakeShareable(new FJsonValueString(Skill)));
	}

	JsonData->SetArrayField(TEXT("Skills"), Skills);

	TArray<TSharedPtr<FJsonValue>> Challenges;
	for (const FString& Challenge : NewPlayerProfileProxy.Challenges)
	{
		Challenges.Add(MakeShareable(new FJsonValueString(Challenge)));
	}

	JsonData->SetArrayField(TEXT("Challenges"), Challenges);

	FString JsonOutput{};
	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = MoveTemp(JsonOutput);
}

void UAVVMOnlinePlayerStringParser::FromString(const FString& NewPayload,
                                               FAVVMPlayerPresetProxy& OutPlayerPresetProxy) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	FAVVMPlayerPresetProxy PlayerPresetProxy{};
	PlayerPresetProxy.UniqueId = JsonData->GetIntegerField(TEXT("UniqueId"));
	PlayerPresetProxy.PresetId = JsonData->GetStringField(TEXT("PresetId"));
	for (const auto& EquippedItem : JsonData->GetArrayField(TEXT("EquippedItems")))
	{
		PlayerPresetProxy.EquippedItems.Add(EquippedItem->AsString());
	}

	OutPlayerPresetProxy = MoveTemp(PlayerPresetProxy);
}

void UAVVMOnlinePlayerStringParser::ToString(const FAVVMPlayerPresetProxy& NewPlayerPresetProxy,
                                             FString& OutFormat) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetNumberField(TEXT("UniqueId"), NewPlayerPresetProxy.UniqueId);
	JsonData->SetStringField(TEXT("PresetId"), NewPlayerPresetProxy.PresetId);

	TArray<TSharedPtr<FJsonValue>> JsonValues{};
	for (const FString& EquippedItem : NewPlayerPresetProxy.EquippedItems)
	{
		JsonValues.Add(MakeShareable(new FJsonValueString(EquippedItem)));
	}

	JsonData->SetArrayField(TEXT("EquippedItems"), JsonValues);

	FString JsonOutput{};
	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = MoveTemp(JsonOutput);
}

void UAVVMOnlinePlayerStringParser::FromString(const FAVVMStringPayload& NewPayload,
                                               TArray<FAVVMPartyProxy>& OutPartyProxies) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload.Payload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	OutPartyProxies.Reset();
	for (const auto& PartyProxy : JsonData->GetArrayField(TEXT("PartyProxies")))
	{
		FAVVMPartyProxy OutPartyProxy{};
		FromString(PartyProxy->AsString(), OutPartyProxy);

		OutPartyProxies.Add(MoveTemp(OutPartyProxy));
	}
}

void UAVVMOnlinePlayerStringParser::ToString(const TArray<FAVVMPartyProxy>& NewPartyProxies,
                                             FString& OutFormat) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	TArray<TSharedPtr<FJsonValue>> JsonValues{};
	for (const FAVVMPartyProxy& PartyProxy : NewPartyProxies)
	{
		FString OutPartyProxy{};
		ToString(PartyProxy, OutPartyProxy);

		JsonValues.Add(MakeShareable(new FJsonValueString(MoveTemp(OutPartyProxy))));
	}

	JsonData->SetArrayField(TEXT("PartyProxies"), JsonValues);

	FString JsonOutput{};
	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = MoveTemp(JsonOutput);
}

void UAVVMOnlinePlayerStringParser::FromString(const FString& NewPayload,
                                               FAVVMPartyProxy& OutPartyProxy) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	FAVVMPartyProxy PartyProxy{};
	PartyProxy.UniqueId = JsonData->GetIntegerField(TEXT("UniqueId"));
	PartyProxy.PartyId = JsonData->GetStringField(TEXT("PartyId"));
	PartyProxy.Region = JsonData->GetStringField(TEXT("Region"));
	PartyProxy.District = JsonData->GetStringField(TEXT("District"));
	PartyProxy.HostConfiguration = JsonData->GetStringField(TEXT("HostConfiguration"));
	for (const auto& PlayerConnection : JsonData->GetArrayField(TEXT("PlayerConnections")))
	{
		PartyProxy.PlayerConnections.Add(PlayerConnection->AsString());
	}

	OutPartyProxy = MoveTemp(PartyProxy);
}

void UAVVMOnlinePlayerStringParser::ToString(const FAVVMPartyProxy& NewPartyProxy,
                                             FString& OutFormat) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetNumberField(TEXT("UniqueId"), NewPartyProxy.UniqueId);
	JsonData->SetStringField(TEXT("PartyId"), NewPartyProxy.PartyId);
	JsonData->SetStringField(TEXT("Region"), NewPartyProxy.Region);
	JsonData->SetStringField(TEXT("District"), NewPartyProxy.District);
	JsonData->SetStringField(TEXT("HostConfiguration"), NewPartyProxy.HostConfiguration);

	TArray<TSharedPtr<FJsonValue>> JsonValues{};
	for (const FString& PlayerConnection : NewPartyProxy.PlayerConnections)
	{
		JsonValues.Add(MakeShareable(new FJsonValueString(PlayerConnection)));
	}

	JsonData->SetArrayField(TEXT("PlayerConnections"), JsonValues);

	FString JsonOutput{};
	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = MoveTemp(JsonOutput);
}

void UAVVMOnlinePlayerStringParser::FromString(const FAVVMStringPayload& NewPayload,
                                               TArray<FAVVMPlayerConnectionProxy>& OutPlayerConnectionProxies) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload.Payload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	OutPlayerConnectionProxies.Reset();
	for (const auto& PlayerConnection : JsonData->GetArrayField(TEXT("PlayerConnections")))
	{
		FAVVMPlayerConnectionProxy OutPlayerConnectionProxy{};
		FromString(PlayerConnection->AsString(), OutPlayerConnectionProxy);

		OutPlayerConnectionProxies.Add(MoveTemp(OutPlayerConnectionProxy));
	}
}

void UAVVMOnlinePlayerStringParser::ToString(const TArray<FAVVMPlayerConnectionProxy>& NewPlayerConnectionProxies,
                                             FString& OutFormat) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	TArray<TSharedPtr<FJsonValue>> JsonValues{};
	for (const FAVVMPlayerConnectionProxy& PlayerConnectionProxy : NewPlayerConnectionProxies)
	{
		FString OutPlayerConnectionProxy{};
		ToString(PlayerConnectionProxy, OutPlayerConnectionProxy);

		JsonValues.Add(MakeShareable(new FJsonValueString(MoveTemp(OutPlayerConnectionProxy))));
	}

	JsonData->SetArrayField(TEXT("PlayerConnections"), JsonValues);

	FString JsonOutput{};
	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = MoveTemp(JsonOutput);
}

void UAVVMOnlinePlayerStringParser::FromString(const FString& NewPayload,
                                               FAVVMPlayerConnectionProxy& OutPlayerConnectionProxy) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	FAVVMPlayerConnectionProxy PlayerConnectionProxy{};
	PlayerConnectionProxy.UniqueId = JsonData->GetIntegerField(TEXT("UniqueId"));
	PlayerConnectionProxy.UniqueNetId = JsonData->GetStringField(TEXT("UniqueNetId"));
	PlayerConnectionProxy.PlayerStatus = StaticCast<EAVVMPlayerStatus>(JsonData->GetIntegerField(TEXT("PlayerStatus")));
	PlayerConnectionProxy.Profile = JsonData->GetStringField(TEXT("Profile"));

	OutPlayerConnectionProxy = MoveTemp(PlayerConnectionProxy);
}

void UAVVMOnlinePlayerStringParser::ToString(const FAVVMPlayerConnectionProxy& NewPlayerConnectionProxy,
                                             FString& OutFormat) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetNumberField(TEXT("UniqueId"), NewPlayerConnectionProxy.UniqueId);
	JsonData->SetStringField(TEXT("UniqueNetId"), NewPlayerConnectionProxy.UniqueNetId);
	JsonData->SetNumberField(TEXT("PlayerStatus"), StaticCast<int32>(NewPlayerConnectionProxy.PlayerStatus));
	JsonData->SetStringField(TEXT("Profile"), NewPlayerConnectionProxy.Profile);

	FString JsonOutput{};
	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = MoveTemp(JsonOutput);
}

void UAVVMOnlinePlayerStringParser::FromString(const FString& NewPayload,
                                               FAVVMHostConfigurationProxy& OutHostConfigurationProxy) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	FAVVMHostConfigurationProxy HostConfigurationProxy{};
	HostConfigurationProxy.UniqueId = JsonData->GetIntegerField(TEXT("UniqueId"));
	HostConfigurationProxy.GameMode = JsonData->GetStringField(TEXT("GameMode"));
	HostConfigurationProxy.GameModeAdditiveOptions = JsonData->GetStringField(TEXT("GameModeAdditiveOptions"));

	OutHostConfigurationProxy = MoveTemp(HostConfigurationProxy);
}

void UAVVMOnlinePlayerStringParser::ToString(const FAVVMHostConfigurationProxy& NewHostConfigurationProxy,
                                             FString& OutFormat) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetNumberField(TEXT("UniqueId"), NewHostConfigurationProxy.UniqueId);
	JsonData->SetStringField(TEXT("GameMode"), NewHostConfigurationProxy.GameMode);
	JsonData->SetStringField(TEXT("GameModeAdditiveOptions"), NewHostConfigurationProxy.GameModeAdditiveOptions);

	FString JsonOutput{};
	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = MoveTemp(JsonOutput);
}
