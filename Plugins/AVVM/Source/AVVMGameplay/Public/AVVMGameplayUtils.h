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

#include "Kismet/BlueprintFunctionLibrary.h"

#include "AVVMGameplayUtils.generated.h"

/**
 *	Class Description :
 *
 *	UAVVMGameplayUtils expose a set of utility function relevant for gameplay.
 */
UCLASS()
class AVVMGAMEPLAY_API UAVVMGameplayUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// @gdemers check if the actor has authority on the local machine. i.e can it modify the state of the actor locally.
	UFUNCTION(BlueprintCallable, Category="AVVM|Utils")
	static bool CheckActorAuthority(const AActor* Actor);

	// @gdemers other than the APlayerController, all Actors are ROLE_Authority if non-replicated or if existing on the Server.
	UFUNCTION(BlueprintCallable, Category="AVVM|Utils")
	static bool HasNetworkAuthority(const AActor* Actor);

	UFUNCTION(BlueprintCallable, Category="AVVM|Utils")
	static FString BP_PrintNetSource(const AActor* Actor);

	static FStringView PrintNetSource(const AActor* Actor);

	UFUNCTION(BlueprintCallable, Category="AVVM|Utils")
	static FString PrintConnectionInfo(const UNetConnection* Connection);
	
	UFUNCTION(BlueprintCallable)
	static int32 GetUniqueIdentifier(const AActor* Actor);
};
