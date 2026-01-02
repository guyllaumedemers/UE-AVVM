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
#include "Backend/AVVMOnlineBackendUtils.h"

TArray<int32> UAVVMOnlineBackendUtils::GetElementDependencies(const UObject* Outer,
                                                              const int32 ElementUniqueId,
                                                              const TInstancedStruct<FAVVMDataResolverHelper>& DataResolverHelper)
{
	TArray<int32> OutResults;

	const auto* Helper = DataResolverHelper.GetPtr<FAVVMDataResolverHelper>();
	if (ensureAlwaysMsgf(Helper != nullptr, TEXT("Invalid Helper.")))
	{
		OutResults = Helper->GetElementDependencies(Outer, ElementUniqueId);
	}

	return OutResults;
}

bool UAVVMOnlineBackendUtils::CompareSet(const TArray<int32>& Lhs,
                                         const TArray<int32>& Rhs)
{
	// TODO @gdemers missing comparison check
	return false;
}

void UAVVMOnlineBackendUtils::Submit(const UObject* WorldContextObject,
                                     const int32 TargetUniqueId,
                                     const FString& NewProfile)
{
	// TODO @gdemers Update backend profile. (valid for any actor type referenced on backend)
}
