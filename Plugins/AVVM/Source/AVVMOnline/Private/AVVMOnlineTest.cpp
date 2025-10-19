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
#include "AVVMOnlineStringParser.h"
#include "Backend/AVVMOnlinePlayer.h"

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
		A.Username = TEXT("Password");
		A.Password = TEXT("Password");
		A.UniqueId = FMath::Rand32();
		Parser->ToString(A, OutPayload);
		
		FAVVMPlayerLoginContext B;
		Parser->FromString(OutPayload, B);

		UTEST_EQUAL("FAVVMPlayerLoginContext", A, B);
	}
	
#endif
	return true;
}
