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

#include "AVVMAutomatedTestActor.h"
#include "AVVMAutomatedTestPresenter.h"
#include "AVVMNotificationSubsystem.h"
#include "AVVMSubsystem.h"
#include "NativeGameplayTags.h"
#include "Archetypes/AVVMPresenter.h"

#if WITH_EDITOR && WITH_AUTOMATION_TESTS
#include "Tests/AutomationEditorCommon.h"
#endif

// @gdemers WARNING : Careful about Server-Client mismatch. Server grants tags so this module has to be available there.
UE_DEFINE_GAMEPLAY_TAG(TAG_FUNCTIONAL_TEST_AVVM_CHANNELTAG, "FunctionalTest.NotificationSubsystem.TestChannel");

/**
 *	Class description:
 *
 *	AVVMNotificationSubsystemTest is an Automated Test running validation on system feature.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(AVVMNotificationSubsystemTest, "AutomatedTest.CustomGroup.AVVMNotificationSubsystemTest", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool AVVMNotificationSubsystemTest::RunTest(const FString& Parameters)
{
#if WITH_EDITOR && WITH_AUTOMATION_TESTS
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	UTEST_NOT_NULL("UWorld.", World)

	// @gdemers create test actors.
	TArray<AAVVMAutomatedTestActor*> TestActors;
	TestActors.Add(World->SpawnActor<AAVVMAutomatedTestActor>());
	TestActors.Add(World->SpawnActor<AAVVMAutomatedTestActor>());

	// @gdemers scoped instance.
	FScopedCounterNotify ScopedInstance(TestActors.Num());

	for (auto* TestActor : TestActors)
	{
		FAVVMObserverContextArgs ObserverContextArgs;
		ObserverContextArgs.ChannelTag = TAG_FUNCTIONAL_TEST_AVVM_CHANNELTAG;
		TestActor->GetSetDelegate(&ScopedInstance, ObserverContextArgs.Callback);
		UAVVMNotificationSubsystem::Static_RegisterObserver(TestActor, ObserverContextArgs);
	}

	// @gdemers test register.
	UTEST_EQUAL("TMap<const FGameplayTag, FAVVMObservers> Collection Changed {Post-Registration}.", UAVVMNotificationSubsystem::Static_GetChannelsCount(World), 1)
	UTEST_EQUAL("TMap<const FGameplayTag, FAVVMObservers>::ValueType Collection Changed {Post-Registration}.", UAVVMNotificationSubsystem::Static_GetChannelCount(World, TAG_FUNCTIONAL_TEST_AVVM_CHANNELTAG), 2)

	FAVVMNotificationContextArgs NotifierContextArgs_WithoutTarget;
	NotifierContextArgs_WithoutTarget.ChannelTag = TAG_FUNCTIONAL_TEST_AVVM_CHANNELTAG;
	NotifierContextArgs_WithoutTarget.Payload = FAVVMNotificationPayload::Empty;
	UAVVMNotificationSubsystem::Static_BroadcastChannel(World, NotifierContextArgs_WithoutTarget);

	// @gdemers test broadcast.
	UTEST_EQUAL("FScopedCounterNotify Count Changed {Post-generic notify}.", ScopedInstance.GetCount(), 0)

	ScopedInstance.Reset();

	FAVVMNotificationContextArgs NotifierContextArgs_WithTarget;
	NotifierContextArgs_WithTarget.ChannelTag = TAG_FUNCTIONAL_TEST_AVVM_CHANNELTAG;
	NotifierContextArgs_WithTarget.Payload = FAVVMNotificationPayload::Empty;
	NotifierContextArgs_WithTarget.Target = TestActors[0];
	UAVVMNotificationSubsystem::Static_BroadcastChannel(World, NotifierContextArgs_WithTarget);

	// @gdemers test broadcast.
	UTEST_EQUAL("FScopedCounterNotify Count {Post-targeted notify}.", ScopedInstance.GetCount(), TestActors.Num() - 1)

	for (auto* TestActor : TestActors)
	{
		FAVVMObserverContextArgs ObserverContextArgs;
		ObserverContextArgs.ChannelTag = TAG_FUNCTIONAL_TEST_AVVM_CHANNELTAG;
		UAVVMNotificationSubsystem::Static_UnregisterObserver(TestActor, ObserverContextArgs);
	}

	// @gdemers test unregister.
	UTEST_EQUAL("TMap<const FGameplayTag, FAVVMObservers> Collection Changed {Post-Unregistration}.", UAVVMNotificationSubsystem::Static_GetChannelsCount(World), 0)
	UTEST_EQUAL("TMap<const FGameplayTag, FAVVMObservers>::ValueType Collection Changed {Post-Unregistration}.", UAVVMNotificationSubsystem::Static_GetChannelCount(World, TAG_FUNCTIONAL_TEST_AVVM_CHANNELTAG), INDEX_NONE)

	World->DestroyWorld(true);
#endif
	return true;
}

/**
 *	Class description:
 *
 *	AVVMSubsystemTest is an Automated Test running validation on system feature.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(AVVMSubsystemTest, "AutomatedTest.CustomGroup.AVVMSubsystemTest", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool AVVMSubsystemTest::RunTest(const FString& Parameters)
{
#if WITH_EDITOR && WITH_AUTOMATION_TESTS
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	UTEST_NOT_NULL("UWorld.", World)

	// @gdemers create test actors.
	TArray<AAVVMAutomatedTestActor*> TestActors;
	TestActors.Add(World->SpawnActor<AAVVMAutomatedTestActor>());
	TestActors.Add(World->SpawnActor<AAVVMAutomatedTestActor>());

	for (auto* TestActor : TestActors)
	{
		FAVVMPresenterContextArgs PresenterContextArgs;
		PresenterContextArgs.WorldContext = World;
		PresenterContextArgs.Presenter = UAVVMAutomatedTestUtils::GetSetPresenter(UAVVMAutomatedTestPresenter::StaticClass(), TestActor);
		UAVVMSubsystem::Static_RegisterPresenter(PresenterContextArgs);
	}

	// @gdemers test register.
	UTEST_EQUAL("TMap<TWeakObjectPtr<const AActor>, FAVVMViewModelKVP> Collection Changed {Post-Registration}.", UAVVMSubsystem::Static_GetActorCount(World), 2)
	UTEST_EQUAL("TMap<TWeakObjectPtr<const AActor>, FAVVMViewModelKVP>::ValueType Collection Changed {Post-Registration}.", UAVVMSubsystem::Static_GetPresentersCount(World), 2)

	for (auto* TestActor : TestActors)
	{
		FAVVMPresenterContextArgs PresenterContextArgs;
		PresenterContextArgs.WorldContext = World;
		PresenterContextArgs.Presenter = UAVVMAutomatedTestUtils::GetSetPresenter(UAVVMAutomatedTestPresenter::StaticClass(), TestActor);
		UAVVMSubsystem::Static_UnregisterPresenter(PresenterContextArgs);
	}

	// @gdemers test unregister.
	UTEST_EQUAL("TMap<TWeakObjectPtr<const AActor>, FAVVMViewModelKVP> Collection Changed {Post-Unregistration}.", UAVVMSubsystem::Static_GetActorCount(World), 0)
	UTEST_EQUAL("TMap<TWeakObjectPtr<const AActor>, FAVVMViewModelKVP>::ValueType Collection Changed {Post-Unregistration}.", UAVVMSubsystem::Static_GetPresentersCount(World), 0)

	World->DestroyWorld(true);
#endif
	return true;
}
