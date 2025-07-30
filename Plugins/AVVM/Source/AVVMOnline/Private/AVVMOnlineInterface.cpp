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
#include "AVVMOnlineInterface.h"

bool FAVVMStringPayload::operator==(const FAVVMStringPayload& Rhs) const
{
	return Payload.Equals(Rhs.Payload);
}

bool FAVVMCurrency::operator==(const FAVVMCurrency& Rhs) const
{
	return (CurrencyId.Equals(Rhs.CurrencyId))
			&& (TotalAmount == Rhs.TotalAmount);
}

bool FAVVMPlayerWallet::operator==(const FAVVMPlayerWallet& Rhs) const
{
	return (UniqueId == Rhs.UniqueId)
			&& (IrlMoneys == Rhs.IrlMoneys);
}

bool FAVVMPlayerResource::operator==(const FAVVMPlayerResource& Rhs) const
{
	return (UniqueId == Rhs.UniqueId)
			&& (ResourceId.Equals(Rhs.ResourceId))
			&& (Options.Equals(Rhs.Options));
}

bool FAVVMPlayerChallenge::operator==(const FAVVMPlayerChallenge& Rhs) const
{
	return (UniqueId == Rhs.UniqueId)
			&& (ChallengeId.Equals(Rhs.ChallengeId))
			&& (Goal == Rhs.Goal)
			&& (ResourceIds == Rhs.ResourceIds);
}

bool FAVVMPlayerPreset::operator==(const FAVVMPlayerPreset& Rhs) const
{
	return (UniqueId == Rhs.UniqueId)
			&& (PresetId.Equals(Rhs.PresetId))
			&& (EquippedItems == Rhs.EquippedItems);
}

bool FAVVMPlayerProfile::operator==(const FAVVMPlayerProfile& Rhs) const
{
	return (UniqueId == Rhs.UniqueId)
			&& (ProfileId.Equals(Rhs.ProfileId))
			&& (Progression.Equals(Rhs.Progression))
			&& (InventoryIds == Rhs.InventoryIds)
			&& (ChallengeIds == Rhs.ChallengeIds)
			&& (EquippedPresetId == Rhs.EquippedPresetId);
}

bool FAVVMPlayerAccount::operator==(const FAVVMPlayerAccount& Rhs) const
{
	return (UniqueId == Rhs.UniqueId)
			&& (Gamertag.Equals(Rhs.Gamertag))
			&& (WalletId == Rhs.WalletId)
			&& (ProfileIds == Rhs.ProfileIds)
			&& (PresetIds == Rhs.PresetIds);
}

bool FAVVMHostConfiguration::operator==(const FAVVMHostConfiguration& Rhs) const
{
	return (GameMode.Equals(Rhs.GameMode))
			&& (Options.Equals(Rhs.Options));
}

bool FAVVMPlayerConnection::operator==(const FAVVMPlayerConnection& Rhs) const
{
	return (UniqueId == Rhs.UniqueId)
			&& (UniqueNetId.Equals(Rhs.UniqueNetId))
			&& (PlayerStatus == Rhs.PlayerStatus)
			&& (ProfileId == Rhs.ProfileId);
}

bool FAVVMParty::operator==(const FAVVMParty& Rhs) const
{
	return (UniqueId == Rhs.UniqueId)
			&& (PartyId.Equals(Rhs.PartyId))
			&& (RegionId == Rhs.RegionId)
			&& (DistrictId == Rhs.DistrictId)
			&& (HostConfiguration.Equals(Rhs.HostConfiguration))
			&& (PlayerConnectionIds == Rhs.PlayerConnectionIds);
}

FAVVMPlayerRequest::FAVVMPlayerRequest(const FString& NewSrcPlayerUniqueNetId,
                                       const FString& NewDestPlayerUniqueNetId,
                                       const EAVVMPlayerRequestType NewRequestType,
                                       const FString& NewPayload)
	: SrcPlayerUniqueNetId(NewSrcPlayerUniqueNetId)
	, DestPlayerUniqueNetId(NewDestPlayerUniqueNetId)
	, RequestType(NewRequestType)
	, Payload(NewPayload)
{
}

bool FAVVMPlayerRequest::operator==(const FAVVMPlayerRequest& Rhs) const
{
	return (SrcPlayerUniqueNetId.Equals(Rhs.SrcPlayerUniqueNetId))
			&& (DestPlayerUniqueNetId.Equals(Rhs.DestPlayerUniqueNetId))
			&& (RequestType == Rhs.RequestType)
			&& (Payload.Equals(Rhs.Payload));
}

bool FAVVMLoginContext::operator==(const FAVVMLoginContext& Rhs) const
{
	return (Username.Equals(Rhs.Username))
			&& (Password.Equals(Rhs.Password));
}
