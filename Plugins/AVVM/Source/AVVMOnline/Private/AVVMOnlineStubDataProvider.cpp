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
#include "AVVMOnlineStubDataProvider.h"

TStrongObjectPtr<UAVVMOnlineStubDataHelper> UAVVMOnlineStubDataHelper::gStubDataHelper = nullptr;

void UAVVMOnlineStubDataHelper::Static_RegisterPresetPropertyProvider(const FGameplayTag& PropertyTag,
                                                                      const TSubclassOf<UAVVMOnlinePresetStubDataProvider>& ProviderClass)
{
	auto* StubDataHelper = Get();
	if (ensureAlwaysMsgf(IsValid(StubDataHelper), TEXT("Invalid Stub Data Helper")))
	{
		auto& SubClassOf = StubDataHelper->PresetStubDataProviders.FindOrAdd(PropertyTag);
		SubClassOf = ProviderClass;
	}
}

void UAVVMOnlineStubDataHelper::Static_RegisterPropertyProvider(const FGameplayTag& PropertyTag,
                                                                const TSubclassOf<UAVVMOnlineStubDataProvider>& ProviderClass)
{
	auto* StubDataHelper = Get();
	if (ensureAlwaysMsgf(IsValid(StubDataHelper), TEXT("Invalid Stub Data Helper")))
	{
		auto& SubClassOf = StubDataHelper->StubDataProviders.FindOrAdd(PropertyTag);
		SubClassOf = ProviderClass;
	}
}

TMap<FGameplayTag, int32> UAVVMOnlineStubDataHelper::Static_MakePresetPropertyData(const FGameplayTag& PropertyTag)
{
	auto* StubDataHelper = Get();
	if (!ensureAlwaysMsgf(IsValid(StubDataHelper), TEXT("Invalid Stub Data Helper")))
	{
		return TMap<FGameplayTag, int32>{};
	}

	if (!ensureAlwaysMsgf(StubDataHelper->PresetStubDataProviders.Contains(PropertyTag),
	                      TEXT("Invalid Provider")))
	{
		return TMap<FGameplayTag, int32>{};
	}

	UAVVMOnlinePresetStubDataProvider* CDO = StubDataHelper->PresetStubDataProviders[PropertyTag].GetDefaultObject();
	if (!IsValid(CDO))
	{
		return TMap<FGameplayTag, int32>{};
	}
	else
	{
		return CDO->MakePropertyStubData();
	}
}

TArray<int32> UAVVMOnlineStubDataHelper::Static_MakePropertyData(const FGameplayTag& PropertyTag)
{
	auto* StubDataHelper = Get();
	if (!ensureAlwaysMsgf(IsValid(StubDataHelper), TEXT("Invalid Stub Data Helper")))
	{
		return TArray<int32>{};
	}

	if (!ensureAlwaysMsgf(StubDataHelper->StubDataProviders.Contains(PropertyTag),
	                      TEXT("Invalid Provider")))
	{
		return TArray<int32>{};
	}

	UAVVMOnlineStubDataProvider* CDO = StubDataHelper->StubDataProviders[PropertyTag].GetDefaultObject();
	if (!IsValid(CDO))
	{
		return TArray<int32>{};
	}
	else
	{
		return CDO->MakePropertyStubData();
	}
}

UAVVMOnlineStubDataHelper* UAVVMOnlineStubDataHelper::Get()
{
	if (!gStubDataHelper.IsValid())
	{
		auto* Instance = NewObject<UAVVMOnlineStubDataHelper>();
		gStubDataHelper.Reset(Instance);
	}

	return gStubDataHelper.Get();
}
