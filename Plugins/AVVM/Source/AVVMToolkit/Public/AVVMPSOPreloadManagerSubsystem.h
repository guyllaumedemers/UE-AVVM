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

#include "Subsystems/GameInstanceSubsystem.h"
#include "UObject/SoftObjectPtr.h"

#include "AVVMPSOPreloadManagerSubsystem.generated.h"

struct FStreamableHandle;

/**
 *	Class description:
 *
 *	FAVVMPSOPreloadToken describe a unique identifier that increment only when default construct. Can be safely
 *	passed by copy around.
 */
USTRUCT(BlueprintType)
struct FAVVMPSOPreloadToken
{
	GENERATED_BODY()
	
	static FAVVMPSOPreloadToken Make()
	{
		static uint32 GlobalUniqueId = 0;

		FAVVMPSOPreloadToken NewToken;
		NewToken.UniqueId = ++GlobalUniqueId;

		return NewToken;
	}
	
	UPROPERTY(Transient, BlueprintReadOnly)
	int32 UniqueId = INDEX_NONE;
};

/**
 *	Class description:
 *	
 *	UAVVMPSOPreloadManagerSubsystem is a subsystem requesting PSO compilation on World target before client travel query.
 *	
 *	Note : It's expected that this task be executed in the background while the user navigate menus, etc...
 */
UCLASS()
class AVVMTOOLKIT_API UAVVMPSOPreloadManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static FAVVMPSOPreloadToken Static_PreloadPSO(const UGameInstance* GameInstance,
	                                              const TArray<TSoftObjectPtr<UWorld>>& LevelInstances);

	UFUNCTION(BlueprintCallable)
	static void Static_UnloadPSOHandle(const UGameInstance* GameInstance,
	                                   const FAVVMPSOPreloadToken& Token);

protected:
	static UAVVMPSOPreloadManagerSubsystem* Get(const UGameInstance* GameInstance);
	FAVVMPSOPreloadToken PreloadPSO(const TArray<TSoftObjectPtr<UWorld>>& LevelInstances);
	void UnloadPSOHandle(const FAVVMPSOPreloadToken& Token);
	
	TMap<int32/*PSOPreload.UniqueId*/, TSharedPtr<FStreamableHandle>> PSOHandles;
};
