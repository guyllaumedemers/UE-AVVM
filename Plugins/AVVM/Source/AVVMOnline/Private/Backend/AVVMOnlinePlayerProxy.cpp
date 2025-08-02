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
#include "Backend/AVVMOnlinePlayerProxy.h"

bool FAVVMPlayerAccountProxy::operator==(const FAVVMPlayerAccountProxy& Rhs) const
{
	return (UniqueId == Rhs.UniqueId)
			&& (Login.Equals(Rhs.Login))
			&& (Gamertag.Equals(Rhs.Gamertag))
			&& (Wallet.Equals(Rhs.Wallet))
			&& (Profiles == Rhs.Profiles)
			&& (Presets == Rhs.Presets);
}

bool FAVVMPlayerProfileProxy::operator==(const FAVVMPlayerProfileProxy& Rhs) const
{
	return (UniqueId == Rhs.UniqueId)
			&& (ProfileId.Equals(Rhs.ProfileId))
			&& (Progression == Rhs.Progression)
			&& (Inventories == Rhs.Inventories)
			&& (Challenges == Rhs.Challenges)
			&& (EquippedPreset.Equals(Rhs.EquippedPreset));
}

bool FAVVMPlayerPresetProxy::operator==(const FAVVMPlayerPresetProxy& Rhs) const
{
	return (UniqueId == Rhs.UniqueId)
			&& (PresetId.Equals(Rhs.PresetId))
			&& (EquippedItems == Rhs.EquippedItems);
}

bool FAVVMPartyProxy::operator==(const FAVVMPartyProxy& Rhs) const
{
	return (UniqueId == Rhs.UniqueId)
			&& (PartyId.Equals(Rhs.PartyId))
			&& (Region.Equals(Rhs.Region))
			&& (District.Equals(Rhs.District))
			&& (HostConfiguration.Equals(Rhs.HostConfiguration))
			&& (PlayerConnections == Rhs.PlayerConnections);
}

bool FAVVMPlayerConnectionProxy::operator==(const FAVVMPlayerConnectionProxy& Rhs) const
{
	return (UniqueId == Rhs.UniqueId)
			&& (UniqueNetId.Equals(Rhs.UniqueNetId))
			&& (PlayerStatus == Rhs.PlayerStatus)
			&& (Profile.Equals(Rhs.Profile));
}
