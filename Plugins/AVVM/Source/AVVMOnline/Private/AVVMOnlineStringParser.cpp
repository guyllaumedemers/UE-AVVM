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
	NewPlayerWallet.Options = JsonData->GetStringField(TEXT("Options"));

	OutPlayerWallet = NewPlayerWallet;
}

void UAVVMOnlineStringParser::ToString(const FAVVMPlayerWallet& NewPlayerWallet,
                                       FString& OutFormat) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetStringField(TEXT("Options"), NewPlayerWallet.Options);

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
	NewPlayerProfile.Gamertag = JsonData->GetStringField(TEXT("Gamertag"));
	NewPlayerProfile.Xp = JsonData->GetStringField(TEXT("Xp"));
	NewPlayerProfile.Wallet = JsonData->GetStringField(TEXT("Wallet"));
	NewPlayerProfile.Achievements = JsonData->GetStringField(TEXT("Achievements"));
	NewPlayerProfile.Presets = JsonData->GetStringField(TEXT("Presets"));

	OutPlayerProfile = NewPlayerProfile;
}

void UAVVMOnlineStringParser::ToString(const FAVVMPlayerProfile& NewPlayerProfile,
                                       FString& OutFormat) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetStringField(TEXT("Gamertag"), NewPlayerProfile.Gamertag);
	JsonData->SetStringField(TEXT("Xp"), NewPlayerProfile.Xp);
	JsonData->SetStringField(TEXT("Wallet"), NewPlayerProfile.Wallet);
	JsonData->SetStringField(TEXT("Achievements"), NewPlayerProfile.Achievements);
	JsonData->SetStringField(TEXT("Presets"), NewPlayerProfile.Presets);

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
	NewHostConfiguration.GameplayGameMode = JsonData->GetStringField(TEXT("GameplayGameMode"));
	NewHostConfiguration.Options = JsonData->GetStringField(TEXT("Options"));

	OutHostConfiguration = NewHostConfiguration;
}

void UAVVMOnlineStringParser::ToString(const FAVVMHostConfiguration& NewHostConfiguration,
                                       FString& OutFormat) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetStringField(TEXT("GameplayGameMode"), NewHostConfiguration.GameplayGameMode);
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
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	TArray<TSharedPtr<FJsonValue>> JsonValues;
	for (const FAVVMParty& Party : NewParties)
	{
		FString OutValue;
		ToString(Party, OutValue);

		JsonValues.Add(MakeShareable(new FJsonValueString(OutValue)));
	}

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
	NewParty.PartyUniqueId = JsonData->GetStringField(TEXT("PartyUniqueId"));
	NewParty.HostConfiguration = JsonData->GetStringField(TEXT("HostConfiguration"));
	NewParty.PlayerConnections = JsonData->GetStringField(TEXT("PlayerConnections"));

	OutParty = NewParty;
}

void UAVVMOnlineStringParser::ToString(const FAVVMParty& NewParty,
                                       FString& OutFormat) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetStringField(TEXT("PartyUniqueId"), NewParty.PartyUniqueId);
	JsonData->SetStringField(TEXT("HostConfiguration"), NewParty.HostConfiguration);
	JsonData->SetStringField(TEXT("PlayerConnections"), NewParty.PlayerConnections);

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
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	TArray<TSharedPtr<FJsonValue>> JsonValues;
	for (const FAVVMPlayerConnection& PlayerConnection : NewPlayerConnections)
	{
		FString OutValue;
		ToString(PlayerConnection, OutValue);

		JsonValues.Add(MakeShareable(new FJsonValueString(OutValue)));
	}

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
	NewPlayerConnection.PlayerProfile = JsonData->GetStringField(TEXT("PlayerProfile"));
	NewPlayerConnection.PlayerStatus = StaticCast<EAVVMPlayerStatus>(JsonData->GetIntegerField(TEXT("PlayerStatus")));
	NewPlayerConnection.UniqueNetId = JsonData->GetStringField(TEXT("UniqueNetId"));
	NewPlayerConnection.RuntimeChallenges = JsonData->GetStringField(TEXT("RuntimeChallenges"));
	NewPlayerConnection.RuntimeResources = JsonData->GetStringField(TEXT("RuntimeResources"));

	OutPlayerConnection = NewPlayerConnection;
}

void UAVVMOnlineStringParser::ToString(const FAVVMPlayerConnection& NewPlayerConnection,
                                       FString& OutFormat) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetStringField(TEXT("PlayerProfile"), NewPlayerConnection.PlayerProfile);
	JsonData->SetStringField(TEXT("PlayerStatus"), EnumToString(NewPlayerConnection.PlayerStatus));
	JsonData->SetStringField(TEXT("UniqueNetId"), NewPlayerConnection.UniqueNetId);
	JsonData->SetStringField(TEXT("RuntimeChallenges"), NewPlayerConnection.RuntimeChallenges);
	JsonData->SetStringField(TEXT("RuntimeResources"), NewPlayerConnection.RuntimeResources);

	FString JsonOutput;

	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = JsonOutput;
}

void UAVVMOnlineStringParser::FromString(const FAVVMStringPayload& NewPayload,
                                         TArray<FAVVMRuntimeChallenge>& OutPlayerChallenges) const
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
		FAVVMRuntimeChallenge OutPlayerChallenge;
		FromString(JsonValue->AsString(), OutPlayerChallenge);
		OutPlayerChallenges.Add(OutPlayerChallenge);
	}
}

void UAVVMOnlineStringParser::ToString(const TArray<FAVVMRuntimeChallenge>& NewPlayerChallenges,
                                       FString& OutFormat) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	TArray<TSharedPtr<FJsonValue>> JsonValues;
	for (const FAVVMRuntimeChallenge& PlayerChallenge : NewPlayerChallenges)
	{
		FString OutValue;
		ToString(PlayerChallenge, OutValue);

		JsonValues.Add(MakeShareable(new FJsonValueString(OutValue)));
	}

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
                                         FAVVMRuntimeChallenge& OutPlayerChallenge) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	FAVVMRuntimeChallenge NewRuntimeChallenge;
	NewRuntimeChallenge.Options = JsonData->GetStringField(TEXT("Options"));

	OutPlayerChallenge = NewRuntimeChallenge;
}

void UAVVMOnlineStringParser::ToString(const FAVVMRuntimeChallenge& NewPlayerChallenge,
                                       FString& OutFormat) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetStringField(TEXT("Options"), NewPlayerChallenge.Options);

	FString JsonOutput;

	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = JsonOutput;
}

void UAVVMOnlineStringParser::FromString(const FAVVMStringPayload& NewPayload,
                                         TArray<FAVVMRuntimeResource>& OutPlayerResources) const
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
		FAVVMRuntimeResource OutResource;
		FromString(JsonValue->AsString(), OutResource);
		OutPlayerResources.Add(OutResource);
	}
}

void UAVVMOnlineStringParser::ToString(const TArray<FAVVMRuntimeResource>& NewPlayerResources,
                                       FString& OutFormat) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	TArray<TSharedPtr<FJsonValue>> JsonValues;
	for (const FAVVMRuntimeResource& PlayerResource : NewPlayerResources)
	{
		FString OutValue;
		ToString(PlayerResource, OutValue);

		JsonValues.Add(MakeShareable(new FJsonValueString(OutValue)));
	}

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
                                         FAVVMRuntimeResource& OutPlayerResource) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);

	auto JsonReaderRef = TJsonReaderFactory<TCHAR>::Create(NewPayload);
	if (!FJsonSerializer::Deserialize(JsonReaderRef, JsonData))
	{
		return;
	}

	FAVVMRuntimeResource NewRuntimeResource;
	NewRuntimeResource.Options = JsonData->GetStringField(TEXT("Options"));

	OutPlayerResource = NewRuntimeResource;
}

void UAVVMOnlineStringParser::ToString(const FAVVMRuntimeResource& NewPlayerResource,
                                       FString& OutFormat) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetStringField(TEXT("Options"), NewPlayerResource.Options);

	FString JsonOutput;

	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = JsonOutput;
}
