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
#include "AVVMOnlineStringParser.h"

#include "AVVMOnlineInterface.h"

void UAVVMOnlineStringParser::FromString(const FString& NewPayload,
                                         FAVVMCurrency& OutCurrency) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	FAVVMCurrency NewCurrency;
	NewCurrency.CurrencyId = JsonData->GetStringField(TEXT("CurrencyId"));
	NewCurrency.TotalAmount = JsonData->GetIntegerField(TEXT("TotalAmount"));

	OutCurrency = NewCurrency;
}

void UAVVMOnlineStringParser::ToString(const FAVVMCurrency& NewCurrency,
                                       FString& OutFormat) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetStringField(TEXT("CurrencyId"), NewCurrency.CurrencyId);
	JsonData->SetNumberField(TEXT("TotalAmount"), NewCurrency.TotalAmount);

	FString JsonOutput;

	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = JsonOutput;
}

void UAVVMOnlineStringParser::FromString(const FString& NewPayload,
                                         FAVVMPlayerWallet& OutPlayerWallet) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	FAVVMPlayerWallet NewPlayerWallet;
	NewPlayerWallet.UniqueId = JsonData->GetIntegerField(TEXT("UniqueId"));

	const TArray<TSharedPtr<FJsonValue>> JsonValues = JsonData->GetArrayField(TEXT("IrlMoneys"));
	for (const auto& JsonValue : JsonValues)
	{
		NewPlayerWallet.IrlMoneys.Add(JsonValue->AsString());
	}

	OutPlayerWallet = NewPlayerWallet;
}

void UAVVMOnlineStringParser::ToString(const FAVVMPlayerWallet& NewPlayerWallet,
                                       FString& OutFormat) const
{
	TArray<TSharedPtr<FJsonValue>> JsonValues;
	for (const FString& IrlMoney : NewPlayerWallet.IrlMoneys)
	{
		JsonValues.Add(MakeShareable(new FJsonValueString(IrlMoney)));
	}

	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetNumberField(TEXT("UniqueId"), NewPlayerWallet.UniqueId);
	JsonData->SetArrayField(TEXT("IrlMoneys"), JsonValues);

	FString JsonOutput;

	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = JsonOutput;
}

void UAVVMOnlineStringParser::FromString(const FString& NewPayload,
                                         FAVVMPlayerProfile& OutPlayerProfile) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	FAVVMPlayerProfile NewPlayerProfile;
	NewPlayerProfile.UniqueId = JsonData->GetIntegerField(TEXT("UniqueId"));
	NewPlayerProfile.ProfileId = JsonData->GetStringField(TEXT("ProfileId"));
	NewPlayerProfile.Progression = JsonData->GetStringField(TEXT("Progression"));
	NewPlayerProfile.EquippedPresetId = JsonData->GetIntegerField(TEXT("EquippedPresetId"));

	const TArray<TSharedPtr<FJsonValue>> InventoryIds = JsonData->GetArrayField(TEXT("InventoryIds"));
	for (const auto& InventoryId : InventoryIds)
	{
		NewPlayerProfile.InventoryIds.Add(InventoryId->AsNumber());
	}

	const TArray<TSharedPtr<FJsonValue>> ChallengeIds = JsonData->GetArrayField(TEXT("ChallengeIds"));
	for (const auto& ChallengeId : ChallengeIds)
	{
		NewPlayerProfile.ChallengeIds.Add(ChallengeId->AsNumber());
	}

	OutPlayerProfile = NewPlayerProfile;
}

void UAVVMOnlineStringParser::ToString(const FAVVMPlayerProfile& NewPlayerProfile,
                                       FString& OutFormat) const
{
	TArray<TSharedPtr<FJsonValue>> InventoryIds;
	for (const int32 InventoryId : NewPlayerProfile.InventoryIds)
	{
		InventoryIds.Add(MakeShareable(new FJsonValueNumber(InventoryId)));
	}

	TArray<TSharedPtr<FJsonValue>> ChallengeIds;
	for (const int32 ChallengeId : NewPlayerProfile.ChallengeIds)
	{
		ChallengeIds.Add(MakeShareable(new FJsonValueNumber(ChallengeId)));
	}

	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetNumberField(TEXT("UniqueId"), NewPlayerProfile.UniqueId);
	JsonData->SetStringField(TEXT("ProfileId"), NewPlayerProfile.ProfileId);
	JsonData->SetStringField(TEXT("Progression"), NewPlayerProfile.Progression);
	JsonData->SetArrayField(TEXT("InventoryIds"), InventoryIds);
	JsonData->SetNumberField(TEXT("EquippedPresetId"), NewPlayerProfile.EquippedPresetId);
	JsonData->SetArrayField(TEXT("ChallengeIds"), ChallengeIds);

	FString JsonOutput;

	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = JsonOutput;
}

void UAVVMOnlineStringParser::FromString(const FString& NewPayload,
                                         FAVVMPlayerAccount& OutPlayerAccount) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	FAVVMPlayerAccount NewPlayerAccount;
	NewPlayerAccount.UniqueId = JsonData->GetIntegerField(TEXT("UniqueId"));
	NewPlayerAccount.Gamertag = JsonData->GetStringField(TEXT("Gamertag"));
	NewPlayerAccount.WalletId = JsonData->GetIntegerField(TEXT("WalletId"));

	const TArray<TSharedPtr<FJsonValue>> ProfileIds = JsonData->GetArrayField(TEXT("ProfileIds"));
	for (const auto& ProfileId : ProfileIds)
	{
		NewPlayerAccount.ProfileIds.Add(ProfileId->AsNumber());
	}

	const TArray<TSharedPtr<FJsonValue>> PresetIds = JsonData->GetArrayField(TEXT("PresetIds"));
	for (const auto& PresetId : PresetIds)
	{
		NewPlayerAccount.PresetIds.Add(PresetId->AsNumber());
	}

	OutPlayerAccount = NewPlayerAccount;
}

void UAVVMOnlineStringParser::ToString(const FAVVMPlayerAccount& NewPlayerAccount,
                                       FString& OutFormat) const
{
	TArray<TSharedPtr<FJsonValue>> ProfileIds;
	for (const int32 ProfileId : NewPlayerAccount.ProfileIds)
	{
		ProfileIds.Add(MakeShareable(new FJsonValueNumber(ProfileId)));
	}

	TArray<TSharedPtr<FJsonValue>> PresetIds;
	for (const int32 PresetId : NewPlayerAccount.PresetIds)
	{
		PresetIds.Add(MakeShareable(new FJsonValueNumber(PresetId)));
	}

	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetNumberField(TEXT("UniqueId"), NewPlayerAccount.UniqueId);
	JsonData->SetStringField(TEXT("Gamertag"), NewPlayerAccount.Gamertag);
	JsonData->SetNumberField(TEXT("WalletId"), NewPlayerAccount.WalletId);
	JsonData->SetArrayField(TEXT("ProfileIds"), ProfileIds);
	JsonData->SetArrayField(TEXT("PresetIds"), PresetIds);

	FString JsonOutput;

	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = JsonOutput;
}

void UAVVMOnlineStringParser::FromString(const FString& NewPayload,
                                         FAVVMHostConfiguration& OutHostConfiguration) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	FAVVMHostConfiguration NewHostConfiguration;
	NewHostConfiguration.GameMode = JsonData->GetStringField(TEXT("GameMode"));
	NewHostConfiguration.Options = JsonData->GetStringField(TEXT("Options"));

	OutHostConfiguration = NewHostConfiguration;
}

void UAVVMOnlineStringParser::ToString(const FAVVMHostConfiguration& NewHostConfiguration,
                                       FString& OutFormat) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetStringField(TEXT("GameMode"), NewHostConfiguration.GameMode);
	JsonData->SetStringField(TEXT("Options"), NewHostConfiguration.Options);

	FString JsonOutput;

	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = JsonOutput;
}

void UAVVMOnlineStringParser::FromString(const FAVVMStringPayload& NewPayload,
                                         TArray<FAVVMParty>& OutParties) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload.Payload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	OutParties.Reset();

	const TArray<TSharedPtr<FJsonValue>> JsonValues = JsonData->GetArrayField(TEXT("Parties"));
	for (const auto& JsonValue : JsonValues)
	{
		FAVVMParty OutParty;
		FromString(JsonValue->AsString(), OutParty);
		OutParties.Add(OutParty);
	}
}

void UAVVMOnlineStringParser::ToString(const TArray<FAVVMParty>& NewParties,
                                       FString& OutFormat) const
{
	TArray<TSharedPtr<FJsonValue>> JsonValues;
	for (const FAVVMParty& Party : NewParties)
	{
		FString OutValue;
		ToString(Party, OutValue);

		JsonValues.Add(MakeShareable(new FJsonValueString(OutValue)));
	}

	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetArrayField(TEXT("Parties"), JsonValues);

	FString JsonOutput;

	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = JsonOutput;
}

void UAVVMOnlineStringParser::FromString(const FString& NewPayload,
                                         FAVVMParty& OutParty) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	FAVVMParty NewParty;
	NewParty.UniqueId = JsonData->GetIntegerField(TEXT("UniqueId"));
	NewParty.PartyId = JsonData->GetStringField(TEXT("PartyId"));
	NewParty.RegionId = JsonData->GetIntegerField(TEXT("RegionId"));
	NewParty.DistrictId = JsonData->GetIntegerField(TEXT("DistrictId"));
	NewParty.HostConfiguration = JsonData->GetStringField(TEXT("HostConfiguration"));

	TArray<TSharedPtr<FJsonValue>> PlayerConnectionIds = JsonData->GetArrayField(TEXT("PlayerConnectionIds"));
	for (const auto& PlayerConnectionId : PlayerConnectionIds)
	{
		NewParty.PlayerConnectionIds.Add(PlayerConnectionId->AsNumber());
	}

	OutParty = NewParty;
}

void UAVVMOnlineStringParser::ToString(const FAVVMParty& NewParty,
                                       FString& OutFormat) const
{
	TArray<TSharedPtr<FJsonValue>> PlayerConnectionIds;
	for (const int32 PlayerConnectionId : NewParty.PlayerConnectionIds)
	{
		PlayerConnectionIds.Add(MakeShareable(new FJsonValueNumber(PlayerConnectionId)));
	}

	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetNumberField(TEXT("UniqueId"), NewParty.UniqueId);
	JsonData->SetStringField(TEXT("PartyId"), NewParty.PartyId);
	JsonData->SetNumberField(TEXT("RegionId"), NewParty.RegionId);
	JsonData->SetNumberField(TEXT("DistrictId"), NewParty.DistrictId);
	JsonData->SetStringField(TEXT("HostConfiguration"), NewParty.HostConfiguration);
	JsonData->SetArrayField(TEXT("PlayerConnections"), PlayerConnectionIds);

	FString JsonOutput;

	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = JsonOutput;
}

void UAVVMOnlineStringParser::FromString(const FAVVMStringPayload& NewPayload,
                                         TArray<FAVVMPlayerConnection>& OutPlayerConnections) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload.Payload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	OutPlayerConnections.Reset();

	const TArray<TSharedPtr<FJsonValue>> JsonValues = JsonData->GetArrayField(TEXT("PlayerConnections"));
	for (const auto& JsonValue : JsonValues)
	{
		FAVVMPlayerConnection OutPlayerConnection;
		FromString(JsonValue->AsString(), OutPlayerConnection);
		OutPlayerConnections.Add(OutPlayerConnection);
	}
}

void UAVVMOnlineStringParser::ToString(const TArray<FAVVMPlayerConnection>& NewPlayerConnections,
                                       FString& OutFormat) const
{
	TArray<TSharedPtr<FJsonValue>> JsonValues;
	for (const FAVVMPlayerConnection& PlayerConnection : NewPlayerConnections)
	{
		FString OutValue;
		ToString(PlayerConnection, OutValue);

		JsonValues.Add(MakeShareable(new FJsonValueString(OutValue)));
	}

	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetArrayField(TEXT("PlayerConnections"), JsonValues);

	FString JsonOutput;

	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = JsonOutput;
}

void UAVVMOnlineStringParser::FromString(const FString& NewPayload,
                                         FAVVMPlayerConnection& OutPlayerConnection) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	FAVVMPlayerConnection NewPlayerConnection;
	NewPlayerConnection.UniqueId = JsonData->GetIntegerField(TEXT("UniqueId"));
	NewPlayerConnection.UniqueNetId = JsonData->GetStringField(TEXT("UniqueNetId"));
	NewPlayerConnection.PlayerStatus = StaticCast<EAVVMPlayerStatus>(JsonData->GetIntegerField(TEXT("PlayerStatus")));
	NewPlayerConnection.ProfileId = JsonData->GetIntegerField(TEXT("ProfileId"));

	OutPlayerConnection = NewPlayerConnection;
}

void UAVVMOnlineStringParser::ToString(const FAVVMPlayerConnection& NewPlayerConnection,
                                       FString& OutFormat) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetNumberField(TEXT("UniqueId"), NewPlayerConnection.UniqueId);
	JsonData->SetStringField(TEXT("UniqueNetId"), NewPlayerConnection.UniqueNetId);
	JsonData->SetStringField(TEXT("PlayerStatus"), EnumToString(NewPlayerConnection.PlayerStatus));
	JsonData->SetNumberField(TEXT("ProfileId"), NewPlayerConnection.ProfileId);

	FString JsonOutput;

	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = JsonOutput;
}

void UAVVMOnlineStringParser::FromString(const FAVVMStringPayload& NewPayload,
                                         TArray<FAVVMPlayerChallenge>& OutPlayerChallenges) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload.Payload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	OutPlayerChallenges.Reset();

	const TArray<TSharedPtr<FJsonValue>> JsonValues = JsonData->GetArrayField(TEXT("PlayerChallenges"));
	for (const auto& JsonValue : JsonValues)
	{
		FAVVMPlayerChallenge OutPlayerChallenge;
		FromString(JsonValue->AsString(), OutPlayerChallenge);
		OutPlayerChallenges.Add(OutPlayerChallenge);
	}
}

void UAVVMOnlineStringParser::ToString(const TArray<FAVVMPlayerChallenge>& NewPlayerChallenges,
                                       FString& OutFormat) const
{
	TArray<TSharedPtr<FJsonValue>> JsonValues;
	for (const FAVVMPlayerChallenge& PlayerChallenge : NewPlayerChallenges)
	{
		FString OutValue;
		ToString(PlayerChallenge, OutValue);

		JsonValues.Add(MakeShareable(new FJsonValueString(OutValue)));
	}

	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetArrayField(TEXT("PlayerChallenges"), JsonValues);

	FString JsonOutput;

	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = JsonOutput;
}

void UAVVMOnlineStringParser::FromString(const FString& NewPayload,
                                         FAVVMPlayerChallenge& OutPlayerChallenge) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	FAVVMPlayerChallenge NewPlayerChallenge;
	NewPlayerChallenge.UniqueId = JsonData->GetIntegerField(TEXT("UniqueId"));
	NewPlayerChallenge.ChallengeId = JsonData->GetStringField(TEXT("ChallengeId"));
	NewPlayerChallenge.Goal = JsonData->GetIntegerField(TEXT("Goal"));

	const TArray<TSharedPtr<FJsonValue>> JsonValues = JsonData->GetArrayField(TEXT("ResourceIds"));
	for (const auto JsonValue : JsonValues)
	{
		NewPlayerChallenge.ResourceIds.Add(JsonValue->AsNumber());
	}

	OutPlayerChallenge = NewPlayerChallenge;
}

void UAVVMOnlineStringParser::ToString(const FAVVMPlayerChallenge& NewPlayerChallenge,
                                       FString& OutFormat) const
{
	TArray<TSharedPtr<FJsonValue>> ResourceIds;
	for (const auto ResourceId : NewPlayerChallenge.ResourceIds)
	{
		ResourceIds.Add(MakeShareable(new FJsonValueNumber(ResourceId)));
	}

	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetNumberField(TEXT("UniqueId"), NewPlayerChallenge.UniqueId);
	JsonData->SetStringField(TEXT("ChallengeId"), NewPlayerChallenge.ChallengeId);
	JsonData->SetNumberField(TEXT("Goal"), NewPlayerChallenge.Goal);
	JsonData->SetArrayField(TEXT("ResourceIds"), ResourceIds);

	FString JsonOutput;

	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = JsonOutput;
}

void UAVVMOnlineStringParser::FromString(const FAVVMStringPayload& NewPayload,
                                         TArray<FAVVMPlayerResource>& OutPlayerResources) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload.Payload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	OutPlayerResources.Reset();

	const TArray<TSharedPtr<FJsonValue>> JsonValues = JsonData->GetArrayField(TEXT("PlayerResources"));
	for (const auto& JsonValue : JsonValues)
	{
		FAVVMPlayerResource OutResource;
		FromString(JsonValue->AsString(), OutResource);
		OutPlayerResources.Add(OutResource);
	}
}

void UAVVMOnlineStringParser::ToString(const TArray<FAVVMPlayerResource>& NewPlayerResources,
                                       FString& OutFormat) const
{
	TArray<TSharedPtr<FJsonValue>> JsonValues;
	for (const FAVVMPlayerResource& PlayerResource : NewPlayerResources)
	{
		FString OutValue;
		ToString(PlayerResource, OutValue);

		JsonValues.Add(MakeShareable(new FJsonValueString(OutValue)));
	}

	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetArrayField(TEXT("PlayerResources"), JsonValues);

	FString JsonOutput;

	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = JsonOutput;
}

void UAVVMOnlineStringParser::FromString(const FString& NewPayload,
                                         FAVVMPlayerResource& OutPlayerResource) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	FAVVMPlayerResource NewPlayerResource;
	NewPlayerResource.UniqueId = JsonData->GetIntegerField(TEXT("UniqueId"));
	NewPlayerResource.ResourceId = JsonData->GetStringField(TEXT("ResourceId"));
	NewPlayerResource.Options = JsonData->GetStringField(TEXT("Options"));

	OutPlayerResource = NewPlayerResource;
}

void UAVVMOnlineStringParser::ToString(const FAVVMPlayerResource& NewPlayerResource,
                                       FString& OutFormat) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetNumberField(TEXT("UniqueId"), NewPlayerResource.UniqueId);
	JsonData->SetStringField(TEXT("ResourceId"), NewPlayerResource.ResourceId);
	JsonData->SetStringField(TEXT("Options"), NewPlayerResource.Options);

	FString JsonOutput;

	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = JsonOutput;
}

void UAVVMOnlineStringParser::FromString(const FString& NewPayload,
                                         FAVVMPlayerPreset& OutPlayerPreset) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	FAVVMPlayerPreset NewPlayerPreset;
	NewPlayerPreset.UniqueId = JsonData->GetIntegerField(TEXT("UniqueId"));
	NewPlayerPreset.PresetId = JsonData->GetStringField(TEXT("PresetId"));

	const TArray<TSharedPtr<FJsonValue>> JsonValues = JsonData->GetArrayField(TEXT("EquippedItems"));
	for (const auto& JsonValue : JsonValues)
	{
		NewPlayerPreset.EquippedItems.Add(JsonValue->AsNumber());
	}

	OutPlayerPreset = NewPlayerPreset;
}

void UAVVMOnlineStringParser::ToString(const FAVVMPlayerPreset& NewPlayerPreset,
                                       FString& OutFormat) const
{
	TArray<TSharedPtr<FJsonValue>> EquippedItems;
	for (const int32 EquippedItem : NewPlayerPreset.EquippedItems)
	{
		EquippedItems.Add(MakeShareable(new FJsonValueNumber(EquippedItem)));
	}

	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetNumberField(TEXT("UniqueId"), NewPlayerPreset.UniqueId);
	JsonData->SetStringField(TEXT("PresetId"), NewPlayerPreset.PresetId);
	JsonData->SetArrayField(TEXT("EquippedItems"), EquippedItems);

	FString JsonOutput;

	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = JsonOutput;
}
