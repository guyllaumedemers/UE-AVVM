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
#pragma once

#include "CoreMinimal.h"

#include "GameplayTagContainer.h"
#include "Templates/SubclassOf.h"
#include "UObject/Object.h"
#include "UObject/StrongObjectPtr.h"

#include "AVVMOnlineStubDataProvider.generated.h"

/**
 *	Class description:
 *	
 *	UAVVMOnlineStubDataProvider is an impl UObject that return stub information about a property
 *	specific to {FAVVMPlayerProfile}, and initializing cached information within AVVMGameSession to simulate
 *	running a backend without the required hooks in place.
 */
UCLASS(BlueprintType, NotBlueprintable)
class AVVMONLINE_API UAVVMOnlineStubDataProvider : public UObject
{
	GENERATED_BODY()

public:
	virtual TArray<int32> MakePropertyStubData() const PURE_VIRTUAL(MakePropertyStubData, return TArray<int32>{};);
};

/**
 *	Class description:
 *	
 *	UAVVMOnlinePresetStubDataProvider is an impl UObject that return stub information about a KVP property
 *	specific to {FAVVMPlayerPreset}, and initializing cached information within AVVMGameSession to simulate
 *	running a backend without the required hooks in place.
 *	
 *	Note : We may later require KVP information about user defined preset for custom skill/build, 
 *	cosmetics, etc...
 */
UCLASS(BlueprintType, NotBlueprintable)
class AVVMONLINE_API UAVVMOnlinePresetStubDataProvider : public UObject
{
	GENERATED_BODY()

public:
	virtual TMap<FGameplayTag/*Slot Tag*/, int32> MakePropertyStubData() const PURE_VIRTUAL(MakePropertyStubData, return TMap<FGameplayTag, int32>{};);
};

/**
 *	Class description:
 *	
 *	UAVVMOnlineStubDataHelper is a Singleton Helper to allow registering Stub Data Provider across multiple Dlls.
 */
UCLASS(NotBlueprintType, NotBlueprintable)
class AVVMONLINE_API UAVVMOnlineStubDataHelper final : public UObject
{
	GENERATED_BODY()

public:
	static void Static_RegisterPresetPropertyProvider(const FGameplayTag& PropertyTag,
	                                                  const TSubclassOf<UAVVMOnlinePresetStubDataProvider>& ProviderClass);

	static void Static_RegisterPropertyProvider(const FGameplayTag& PropertyTag,
	                                            const TSubclassOf<UAVVMOnlineStubDataProvider>& ProviderClass);
	
	static TMap<FGameplayTag, int32> Static_MakePresetPropertyData(const FGameplayTag& PropertyTag);
	static TArray<int32> Static_MakePropertyData(const FGameplayTag& PropertyTag);

protected:
	static UAVVMOnlineStubDataHelper* Get();

	static TStrongObjectPtr<UAVVMOnlineStubDataHelper> gStubDataHelper;
	TMap<FGameplayTag, TSubclassOf<UAVVMOnlinePresetStubDataProvider>> PresetStubDataProviders;
	TMap<FGameplayTag, TSubclassOf<UAVVMOnlineStubDataProvider>> StubDataProviders;
};
