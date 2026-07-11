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
#include "Data/AVVMActorIdentifierTableRow.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "UObject/Object.h"

#include "AVVMGameModeAdditive.generated.h"

/**
 *	Class description:
 *	
 *	UAVVMGameModeAdditive is a UObject class that modify the runtime behaviour
 *	of the World referenced AGameMode.
 *	
 *	In comparison to a UAVVMWorldRule, which define the AVVMGameMode base behaviour, the UAVVMGameModeAdditive allows
 *	runtime changes to happen. i.e It extends the basic game loop that the AGameMode define via the ReadyStartMatch/ReadyEndMatch/StartMatch/& EndMatch api.
 *	
 *	MORE IMPORTANTLY, it EXTENDS the referenced AGameMode gameplay loop with transient systems
 *	(*require proper user setup (example : loading a GFP, or registering a component, etc...!)), and provide BP hooks for executing latent Gameplay Events.
 *	
 *	Additionally, UAVVMGameModeAdditive can be tested separately, without tied coupling, by piping cmd line parameters to the server executable,
 *	and allow direct initialization of required UAVVMGameModeAdditive derived type listed.
 *	
 *		* Cmdline parameters to used : GameOptions=
 *		* For referencing a UAVVMGameModeAdditive, we expect user to name the BP class type as follow : BP_{ProjectName}_{AssetName}.
 *		* Use the {AssetName} as parameter flag to provide the server with valid instance. The parsing api will handle retrieving command line parameter flag.
 */
UCLASS(BlueprintType, Blueprintable)
class AVVMGAMEPLAY_API UAVVMGameModeAdditive : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	bool IsMatchInProgress() const;
	virtual bool IsMatchInProgress_Implementation() const PURE_VIRTUAL(IsMatchInProgress_Implementation, return false;);

	UFUNCTION(BlueprintNativeEvent)
	bool ReadyToStartMatch();
	virtual bool ReadyToStartMatch_Implementation() PURE_VIRTUAL(ReadyToStartMatch_Implementation, return false;);

	UFUNCTION(BlueprintNativeEvent)
	bool ReadyToEndMatch();
	virtual bool ReadyToEndMatch_Implementation() PURE_VIRTUAL(ReadyToEndMatch_Implementation, return false;);

	UFUNCTION(BlueprintNativeEvent)
	void StartMatch();
	virtual void StartMatch_Implementation() PURE_VIRTUAL(StartMatch_Implementation, return;);

	UFUNCTION(BlueprintNativeEvent)
	void EndMatch();
	virtual void EndMatch_Implementation() PURE_VIRTUAL(EndMatch_Implementation, return;);

	UFUNCTION(BlueprintNativeEvent)
	void RestartGame();
	virtual void RestartGame_Implementation() PURE_VIRTUAL(RestartGame_Implementation, return;);
};

/**
 *	Class description:
 *	
 *	FAVVMGameModeAdditiveDefinitionDataTableRow is a table row entry thet reference a UAVVMGameModeAdditive class object
 *	to extend AGameMode. UAVVMGameModeAdditive referenced in GFP can be dynamically added via Unreal Registry system, resolving the
 *	cross Dll boundaries problem. 
 */
USTRUCT(BlueprintType)
struct AVVMGAMEPLAY_API FAVVMGameModeAdditiveDefinitionDataTableRow : public FTableRowBase
{
	GENERATED_BODY()

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	TSoftClassPtr<UAVVMGameModeAdditive> GameModeAdditiveClass = nullptr;
};

/**
 *	Class description:
 *	
 *	UAVVMGameModeAdditiveUtils is a blueprint function library for reusable api specific to UAVVMGameModeAdditive.
 */
UCLASS()
class AVVMGAMEPLAY_API UAVVMGameModeAdditiveUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static const TArray<FString> ParseCmdOptions(const FString& GameModeOptions);

	UFUNCTION(BlueprintCallable)
	static TMap<FName, UAVVMGameModeAdditive*> LoadSynchronous(const TArray<FString>& SplitOptions,
	                                                           UObject* Outer,
	                                                           TArray<FString>& OutFailedOrPluginSpecificOptions);

	UFUNCTION(BlueprintCallable)
	static TSoftClassPtr<UAVVMGameModeAdditive> GetGameModeAdditiveSoftClass(const TArray<TSoftClassPtr<UAVVMGameModeAdditive>>& OutGameModeAdditives,
	                                                                         const FString& CmdLineFlagName);
};