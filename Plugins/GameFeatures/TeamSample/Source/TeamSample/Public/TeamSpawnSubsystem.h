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

#include "Subsystems/WorldSubsystem.h"

#include "TeamSpawnSubsystem.generated.h"

class UTeamStartComponent;

/**
 *	Class description:
 *
 *	UTeamSpawnSubsystem is a world subsystem that run conditional branching on world state, and
 *	aim at selecting a valid location to spawn a player.
 *
 *	Note : This can be used both for gameplay or menus, or after-action-report and should be
 *	used to either spawn your players at a safe location or in a defined order, maybe based on scoring
 *	etc...
 */
UCLASS()
class TEAMSAMPLE_API UTeamSpawnSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable)
	static void Static_UnRegisterPlayerStart(const UWorld* World,
	                                         const UTeamStartComponent* Component);

	UFUNCTION(BlueprintCallable)
	static void Static_RegisterPlayerStart(const UWorld* World,
	                                       const UTeamStartComponent* Component);

protected:
	UFUNCTION(BlueprintCallable)
	static UTeamSpawnSubsystem* Get(const UWorld* World);

	UFUNCTION(BlueprintCallable)
	void UnRegister(const UTeamStartComponent* Component);

	UFUNCTION(BlueprintCallable)
	void Register(const UTeamStartComponent* Component);

	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<const UTeamStartComponent>> PlayerStarts;
};
