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

#include "Misc/AutomationTest.h"

#include "AVVMOnline.h"
#include "AVVMOnlineInterface.h"
#include "AVVMOnlineStringParser.h"
#include "Backend/AVVMOnlinePlayer.h"
#include "Backend/AVVMOnlinePlayerProxy.h"

#if WITH_EDITOR && WITH_AUTOMATION_TESTS
#include "Tests/AutomationEditorCommon.h"
#endif

/**
 *	Class description:
 *
 *	AVVMOnlineTest is an Automated Test running validation on the online api.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(AVVMOnlineTest, "FunctionalTest.AVVMOnlineTest", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool AVVMOnlineTest::RunTest(const FString& Parameters)
{
#if WITH_EDITOR && WITH_AUTOMATION_TESTS
	const UAVVMOnlineStringParser* Parser = FAVVMOnlineModule::GetJsonParser();
	UTEST_NOT_NULL("UAVVMOnlineStringParser", Parser);

	{
		FString OutPayload;

		FAVVMPlayerLoginContext A;
		A.UniqueId = FMath::Rand32();
		A.Username = TEXT("Password");
		A.Password = TEXT("Password");
		Parser->ToString(A, OutPayload);

		FAVVMPlayerLoginContext B;
		Parser->FromString(OutPayload, B);

		UTEST_EQUAL("FAVVMPlayerLoginContext", A, B);
	}

	{
		FString OutPayload;

		FAVVMPlayerAccount A;
		A.UniqueId = FMath::Rand32();
		A.LoginId = FMath::Rand32();
		A.Gamertag = TEXT("Steve");
		A.WalletId = FMath::Rand32();
		A.ProfileIds = {FMath::Rand32(), FMath::Rand32()};
		A.PresetIds = {FMath::Rand32(), FMath::Rand32()};
		Parser->ToString(A, OutPayload);

		FAVVMPlayerAccount B;
		Parser->FromString(OutPayload, B);

		UTEST_EQUAL("FAVVMPlayerAccount", A, B);
	}

	{
		FString OutPayload;

		FAVVMPlayerWallet A;
		A.UniqueId = FMath::Rand32();
		A.IrlMoneys = {TEXT("A"), TEXT("B")};
		Parser->ToString(A, OutPayload);

		FAVVMPlayerWallet B;
		Parser->FromString(OutPayload, B);

		UTEST_EQUAL("FAVVMPlayerWallet", A, B);
	}

	{
		FString OutPayload;

		FAVVMCurrency A;
		A.CurrencyId = TEXT("Money");
		A.TotalAmount = FMath::Rand32();
		Parser->ToString(A, OutPayload);

		FAVVMCurrency B;
		Parser->FromString(OutPayload, B);

		UTEST_EQUAL("FAVVMCurrency", A, B);
	}

	{
		FString OutPayload;

		FAVVMPlayerProfile A;
		A.UniqueId = FMath::Rand32();
		A.ProfileId = TEXT("Secret");
		A.InventoryIds = {FMath::Rand32(), FMath::Rand32()};
		A.ChallengeIds = {FMath::Rand32(), FMath::Rand32()};
		A.EquippedPresetId = FMath::Rand32();
		Parser->ToString(A, OutPayload);

		FAVVMPlayerProfile B;
		Parser->FromString(OutPayload, B);

		UTEST_EQUAL("FAVVMPlayerProfile", A, B);
	}

	{
		FString OutPayload;

		FAVVMPlayerPreset A;
		A.UniqueId = FMath::Rand32();
		A.PresetId = TEXT("MyPreset");
		A.EquippedItems = {FMath::Rand32(), FMath::Rand32()};
		Parser->ToString(A, OutPayload);

		FAVVMPlayerPreset B;
		Parser->FromString(OutPayload, B);

		UTEST_EQUAL("FAVVMPlayerPreset", A, B);
	}

	{
		FAVVMPlayerResource PlayerResource1;
		PlayerResource1.UniqueId = FMath::Rand32();
		PlayerResource1.ResourceId = TEXT("MyResource1");

		FAVVMPlayerResource PlayerResource2;
		PlayerResource2.UniqueId = FMath::Rand32();
		PlayerResource2.ResourceId = TEXT("MyResource2");

		FString OutPayload;

		TArray<FAVVMPlayerResource> A = {PlayerResource1, PlayerResource2};
		Parser->ToString(A, OutPayload);

		TArray<FAVVMPlayerResource> B;
		Parser->FromString(FAVVMStringPayload{.Payload = OutPayload}, B);

		UTEST_EQUAL("TArray<FAVVMPlayerResource>", A, B);
	}

	{
		FAVVMPlayerChallenge Challenge1;
		Challenge1.UniqueId = FMath::Rand32();
		Challenge1.ChallengeId = TEXT("MyChallenge1");
		
		FAVVMPlayerChallenge Challenge2;
		Challenge2.UniqueId = FMath::Rand32();
		Challenge2.ChallengeId = TEXT("MyChallenge2");

		FString OutPayload;

		TArray<FAVVMPlayerChallenge> A = {Challenge1, Challenge2};
		Parser->ToString(A, OutPayload);

		TArray<FAVVMPlayerChallenge> B;
		Parser->FromString(FAVVMStringPayload{.Payload = OutPayload}, B);

		UTEST_EQUAL("TArray<FAVVMPlayerChallenge>", A, B);
	}

	{
		FAVVMParty Party1;
		Party1.UniqueId = FMath::Rand32();
		Party1.PartyId = TEXT("SecretParty1");
		Party1.RegionId = FMath::Rand32();
		Party1.DistrictId = FMath::Rand32();
		Party1.HostConfigurationId = FMath::Rand32();
		Party1.PlayerConnectionIds = {FMath::Rand32(), FMath::Rand32()};
		
		FAVVMParty Party2;
		Party2.UniqueId = FMath::Rand32();
		Party2.PartyId = TEXT("SecretParty2");
		Party2.RegionId = FMath::Rand32();
		Party2.DistrictId = FMath::Rand32();
		Party2.HostConfigurationId = FMath::Rand32();
		Party2.PlayerConnectionIds = {FMath::Rand32()};

		FString OutPayload;

		TArray<FAVVMParty> A = {Party1, Party2};
		Parser->ToString(A, OutPayload);

		TArray<FAVVMParty> B;
		Parser->FromString(FAVVMStringPayload{.Payload = OutPayload}, B);

		UTEST_EQUAL("TArray<FAVVMParty>", A, B);
	}

	{
		FAVVMPlayerConnection PlayerConnection1;
		PlayerConnection1.UniqueId = FMath::Rand32();
		PlayerConnection1.UniqueNetId = TEXT("MyUniqueNetId1");
		PlayerConnection1.PlayerStatus = EAVVMPlayerStatus::Ready;
		PlayerConnection1.ProfileId = FMath::Rand32();
		
		FAVVMPlayerConnection PlayerConnection2;
		PlayerConnection2.UniqueId = FMath::Rand32();
		PlayerConnection2.UniqueNetId = TEXT("MyUniqueNetId2");
		PlayerConnection2.PlayerStatus = EAVVMPlayerStatus::PendingAction;
		PlayerConnection2.ProfileId = FMath::Rand32();

		FString OutPayload;

		TArray<FAVVMPlayerConnection> A = {PlayerConnection1, PlayerConnection2};
		Parser->ToString(A, OutPayload);

		TArray<FAVVMPlayerConnection> B;
		Parser->FromString(FAVVMStringPayload{.Payload = OutPayload}, B);

		UTEST_EQUAL("TArray<FAVVMPlayerConnection>", A, B);
	}

	{
		FString OutPayload;

		FAVVMHostConfiguration A;
		A.UniqueId = FMath::Rand32();
		A.GameMode = TEXT("MyGameMode");
		A.Options = TEXT("OneShotKill");
		Parser->ToString(A, OutPayload);

		FAVVMHostConfiguration B;
		Parser->FromString(OutPayload, B);

		UTEST_EQUAL("FAVVMHostConfiguration", A, B);
	}

	{
		FString OutPayload;

		FAVVMPlayerAccountProxy A;
		A.UniqueId = FMath::Rand32();
		A.Login = TEXT("MyLogin");
		A.Gamertag = TEXT("Steve");
		A.Wallet = TEXT("Empty, cause steve is broke!");
		A.Profiles = {TEXT("ProfileA"), TEXT("ProfileB")};
		A.Presets = {TEXT("UniquePreset")};
		Parser->ToString(A, OutPayload);

		FAVVMPlayerAccountProxy B;
		Parser->FromString(OutPayload, B);

		UTEST_EQUAL("FAVVMPlayerAccountProxy", A, B);
	}

	{
		FString OutPayload;

		FAVVMPlayerWalletProxy A;
		A.UniqueId = FMath::Rand32();
		A.IrlMoneys = {TEXT("CAD"), TEXT("USD")};
		Parser->ToString(A, OutPayload);

		FAVVMPlayerWalletProxy B;
		Parser->FromString(OutPayload, B);

		UTEST_EQUAL("FAVVMPlayerWalletProxy", A, B);
	}

	{
		FString OutPayload;

		FAVVMPlayerProfileProxy A;
		A.UniqueId = FMath::Rand32();
		A.ProfileId = TEXT("ProfileA");
		A.Progression = TEXT("ProfileProgression");
		A.Inventories = {TEXT("CharacterInventoryA")};
		A.Challenges = {TEXT("ChallengeA"), TEXT("ChallengeB")};
		A.EquippedPreset = TEXT("GenericPreset");
		Parser->ToString(A, OutPayload);

		FAVVMPlayerProfileProxy B;
		Parser->FromString(OutPayload, B);

		UTEST_EQUAL("FAVVMPlayerProfileProxy", A, B);
	}

	{
		FString OutPayload;

		FAVVMPlayerPresetProxy A;
		A.UniqueId = FMath::Rand32();
		A.PresetId = TEXT("PresetA");
		A.EquippedItems = {TEXT("ItemA"), TEXT("ItemB")};
		Parser->ToString(A, OutPayload);

		FAVVMPlayerPresetProxy B;
		Parser->FromString(OutPayload, B);

		UTEST_EQUAL("FAVVMPlayerPresetProxy", A, B);
	}

	{
		FAVVMPartyProxy PartyProxy1;
		PartyProxy1.UniqueId = FMath::Rand32();
		PartyProxy1.PartyId = TEXT("MyParty1");
		PartyProxy1.Region = TEXT("RegionD");
		PartyProxy1.District = TEXT("DistrictF");
		PartyProxy1.HostConfiguration = TEXT("ConfigC");
		PartyProxy1.PlayerConnections = {TEXT("ConnectionA")};
		
		FAVVMPartyProxy PartyProxy2;
		PartyProxy2.UniqueId = FMath::Rand32();
		PartyProxy2.PartyId = TEXT("MyParty2");
		PartyProxy2.Region = TEXT("RegionH");
		PartyProxy2.District = TEXT("DistrictY");
		PartyProxy2.HostConfiguration = TEXT("ConfigF");
		PartyProxy2.PlayerConnections = {TEXT("ConnectionB")};

		FString OutPayload;

		TArray<FAVVMPartyProxy> A = {PartyProxy1, PartyProxy2};
		Parser->ToString(A, OutPayload);

		TArray<FAVVMPartyProxy> B;
		Parser->FromString(FAVVMStringPayload{.Payload = OutPayload}, B);

		UTEST_EQUAL("TArray<FAVVMPartyProxy>", A, B);
	}

	{
		FAVVMPlayerConnectionProxy PlayerConnectionProxy1;
		PlayerConnectionProxy1.UniqueId = FMath::Rand32();
		PlayerConnectionProxy1.UniqueNetId = TEXT("UniqueNetIdA");
		PlayerConnectionProxy1.PlayerStatus = EAVVMPlayerStatus::PendingAction;
		PlayerConnectionProxy1.Profile = TEXT("ProfileP");

		FAVVMPlayerConnectionProxy PlayerConnectionProxy2;
		PlayerConnectionProxy2.UniqueId = FMath::Rand32();
		PlayerConnectionProxy2.UniqueNetId = TEXT("UniqueNetIdB");
		PlayerConnectionProxy2.PlayerStatus = EAVVMPlayerStatus::Ready;
		PlayerConnectionProxy2.Profile = TEXT("ProfileP");
		
		FString OutPayload;

		TArray<FAVVMPlayerConnectionProxy> A = {PlayerConnectionProxy1, PlayerConnectionProxy2};
		Parser->ToString(A, OutPayload);

		TArray<FAVVMPlayerConnectionProxy> B;
		Parser->FromString(FAVVMStringPayload{.Payload = OutPayload}, B);

		UTEST_EQUAL("TArray<FAVVMPlayerConnectionProxy>", A, B);
	}

	{
		FString OutPayload;

		FAVVMHostConfigurationProxy A;
		A.UniqueId = FMath::Rand32();
		A.GameMode = TEXT("MyGameMode");
		A.Options = TEXT("Free-For-All");
		Parser->ToString(A, OutPayload);

		FAVVMHostConfigurationProxy B;
		Parser->FromString(OutPayload, B);

		UTEST_EQUAL("FAVVMHostConfigurationProxy", A, B);
	}

#endif
	return true;
}
