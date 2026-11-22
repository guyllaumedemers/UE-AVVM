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

#include "AVVMGameMode.h"
#include "AVVMGameplayTagStackObject.h"
#include "AVVMPlayerState.h"

#include "AVVMAutomatedTestPlayerState.generated.h"

/**
 *	Class description:
 *
 *	ANetTestReplicatedPlayerState is an Actor class that test replicated system feature
 *	tied to PlayerState ownership.
 */
UCLASS(NotBlueprintable)
class ANetTestGameMode : public AAVVMGameMode
{
	GENERATED_BODY()

public:
	ANetTestGameMode(const FObjectInitializer& ObjectInitializer);
};

/**
 *	Class description:
 *
 *	ANetTestReplicatedPlayerState is an Actor class that test replicated system feature
 *	tied to PlayerState ownership.
 */
UCLASS(NotBlueprintable)
class ANetTestReplicatedPlayerState : public AAVVMPlayerState
{
	GENERATED_BODY()

public:
	ANetTestReplicatedPlayerState(const FObjectInitializer& ObjectInitializer);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Transient, Replicated)
	FAVVMGameplayTagStackObjectFastArray ReplicatedTags{};
};
