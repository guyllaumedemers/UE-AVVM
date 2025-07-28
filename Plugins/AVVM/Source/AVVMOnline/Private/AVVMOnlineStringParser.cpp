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

void UAVVMOnlineStringParser::ToString(const FAVVMPlayerWallet& PlayerWallet,
                                       FString& OutFormat) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetStringField(TEXT("Options"), PlayerWallet.Options);

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

void UAVVMOnlineStringParser::ToString(const FAVVMPlayerProfile& PlayerProfile,
                                       FString& OutFormat) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetStringField(TEXT("Gamertag"), PlayerProfile.Gamertag);
	JsonData->SetStringField(TEXT("Xp"), PlayerProfile.Xp);
	JsonData->SetStringField(TEXT("Wallet"), PlayerProfile.Wallet);
	JsonData->SetStringField(TEXT("Achievements"), PlayerProfile.Achievements);
	JsonData->SetStringField(TEXT("Presets"), PlayerProfile.Presets);

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

void UAVVMOnlineStringParser::ToString(const FAVVMHostConfiguration& HostConfiguration,
                                       FString& OutFormat) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetStringField(TEXT("GameplayGameMode"), HostConfiguration.GameplayGameMode);
	JsonData->SetStringField(TEXT("Options"), HostConfiguration.Options);

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

void UAVVMOnlineStringParser::ToString(const FAVVMParty& Party,
                                       FString& OutFormat) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetStringField(TEXT("PartyUniqueId"), Party.PartyUniqueId);
	JsonData->SetStringField(TEXT("HostConfiguration"), Party.HostConfiguration);
	JsonData->SetStringField(TEXT("PlayerConnections"), Party.PlayerConnections);

	FString JsonOutput;

	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = JsonOutput;
}

void UAVVMOnlineStringParser::FromString(const FAVVMStringPayload& Payload,
                                         TArray<FAVVMPlayerConnection>& OutPlayerConnections) const
{
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

void UAVVMOnlineStringParser::ToString(const FAVVMPlayerConnection& PlayerConnection,
                                       FString& OutFormat) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetStringField(TEXT("PlayerProfile"), PlayerConnection.PlayerProfile);
	JsonData->SetStringField(TEXT("PlayerStatus"), EnumToString(PlayerConnection.PlayerStatus));
	JsonData->SetStringField(TEXT("UniqueNetId"), PlayerConnection.UniqueNetId);
	JsonData->SetStringField(TEXT("RuntimeChallenges"), PlayerConnection.RuntimeChallenges);
	JsonData->SetStringField(TEXT("RuntimeResources"), PlayerConnection.RuntimeResources);

	FString JsonOutput;

	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = JsonOutput;
}

void UAVVMOnlineStringParser::FromString(const FAVVMStringPayload& Payload,
                                         TArray<FAVVMRuntimeChallenge>& OutPlayerChallenges) const
{
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

void UAVVMOnlineStringParser::ToString(const FAVVMRuntimeChallenge& PlayerChallenge,
                                       FString& OutFormat) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetStringField(TEXT("Options"), PlayerChallenge.Options);

	FString JsonOutput;

	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = JsonOutput;
}

void UAVVMOnlineStringParser::FromString(const FAVVMStringPayload& Payload,
                                         TArray<FAVVMRuntimeResource>& OutPlayerResources) const
{
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

void UAVVMOnlineStringParser::ToString(const FAVVMRuntimeResource& PlayerResource,
                                       FString& OutFormat) const
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetStringField(TEXT("Options"), PlayerResource.Options);

	FString JsonOutput;

	auto JsonWriterRef = TJsonWriterFactory<TCHAR>::Create(&JsonOutput);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), JsonWriterRef))
	{
		return;
	}

	OutFormat = JsonOutput;
}
