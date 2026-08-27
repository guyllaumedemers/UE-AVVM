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

#include "Engine/World.h"
#include "GameFramework/Info.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Templates/SubclassOf.h"

#include "AVVMClusterSystem.generated.h"

/**
 *	Class description:
 *	
 *	FAVVMClusterObjectHandle is a context handle object caching information about the owning cluster.
 */
USTRUCT(BlueprintType)
struct AVVMTOOLKIT_API FAVVMClusterObjectHandle
{
	GENERATED_BODY()

	UPROPERTY(Transient, BlueprintReadOnly)
	int32 ClusterId{INDEX_NONE};

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<const AActor> OwnedActor = nullptr;

	static FAVVMClusterObjectHandle InvalidHandle;
};

/**
 *	Class description:
 *	
 *	AAVVMBeaconClusterActor is an Actor class managing actor priority within cluster. i.e This tracks actor that are
 *	considered for action by the local client during multiple overlap intersections.
 *	
 *	Note : This Actor class can be used to resolved target priority based on weight functions,
 *	and/or display cluster location in-world/or on minimap.
 */
UCLASS(BlueprintType, Blueprintable)
class AVVMTOOLKIT_API AAVVMBeaconClusterActor : public AActor
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	int32 GetClusterId() const;

	UFUNCTION(BlueprintCallable)
	void SetClusterId(const int32 NewClusterId);

	UFUNCTION(BlueprintCallable)
	bool IsClusterEmpty() const;

	UFUNCTION(BlueprintCallable)
	void AddToCluster(const AActor* Target);

	UFUNCTION(BlueprintCallable)
	void RemoveFromCluster(const AActor* Target);

protected:
	void UpdateBeaconTransform();
	
	UPROPERTY(Transient, BlueprintReadOnly)
	int32 ClusterId{INDEX_NONE};

	UPROPERTY(Transient)
	TSet<TWeakObjectPtr<const AActor>> Cluster{};
};

/**
 *	Class description:
 *	
 *	FAVVMClusterSystem is a context object storing information about all clusters in play.
 */
USTRUCT(BlueprintType)
struct AVVMTOOLKIT_API FAVVMClusterSystem
{
	GENERATED_BODY()

	virtual ~FAVVMClusterSystem() = default;

	FAVVMClusterObjectHandle AppendOrCreateCluster(const AActor* Actor);
	bool RemoveFromCluster(const FAVVMClusterObjectHandle& Handle);

protected:
	virtual AAVVMBeaconClusterActor* Factory(UWorld* World, const FTransform& SpawnTransform, const FActorSpawnParameters& SpawnParams);
	virtual TSubclassOf<AAVVMBeaconClusterActor> GetBeaconActorClass() const;
	virtual double GetMaximumBeaconRadius() const;

	UPROPERTY(Transient)
	TMap<TWeakObjectPtr<const AActor>, int32/*ClusterId*/> MemoizationMap{};

	UPROPERTY(Transient)
	TMap<int32/*ClusterId*/, TObjectPtr<AAVVMBeaconClusterActor>> Beacons{};
};
