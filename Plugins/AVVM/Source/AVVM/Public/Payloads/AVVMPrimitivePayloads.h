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

#include "AVVMNotificationSubsystem.h"

#include "AVVMPrimitivePayloads.generated.h"

/**
 *	Class description:
 *	
 *	FAVVMFloatPayload is an FAVVMNotificationPayload type carrying a float property.
 */
USTRUCT()
struct AVVM_API FAVVMFloatPayload : public FAVVMNotificationPayload
{
	GENERATED_BODY()
	
	FAVVMFloatPayload() = default;
	FAVVMFloatPayload(const FAVVMFloatPayload&) = default;
	FAVVMFloatPayload(FAVVMFloatPayload&&) noexcept = default;
	FAVVMFloatPayload& operator=(const FAVVMFloatPayload&) = default;
	FAVVMFloatPayload& operator=(FAVVMFloatPayload&&) noexcept = default;
	bool operator==(const FAVVMFloatPayload& Rhs) const;
	explicit FAVVMFloatPayload(const float NewValue);

	UPROPERTY(Transient, BlueprintReadWrite)
	float Value{0.f};
};

/**
 *	Class description:
 *	
 *	FAVVMInt32Payload is an FAVVMNotificationPayload type carrying an int32 property.
 */
USTRUCT()
struct AVVM_API FAVVMInt32Payload : public FAVVMNotificationPayload
{
	GENERATED_BODY()
	
	FAVVMInt32Payload() = default;
	FAVVMInt32Payload(const FAVVMInt32Payload&) = default;
	FAVVMInt32Payload(FAVVMInt32Payload&&) noexcept = default;
	FAVVMInt32Payload& operator=(const FAVVMInt32Payload&) = default;
	FAVVMInt32Payload& operator=(FAVVMInt32Payload&&) noexcept = default;
	bool operator==(const FAVVMInt32Payload& Rhs) const;
	explicit FAVVMInt32Payload(const int32 NewValue);

	UPROPERTY(Transient, BlueprintReadWrite)
	int32 Value{INDEX_NONE};
};

/**
 *	Class description:
 *	
 *	FAVVMBoolPayload is an FAVVMNotificationPayload type carrying a bool property.
 */
USTRUCT()
struct AVVM_API FAVVMBoolPayload : public FAVVMNotificationPayload
{
	GENERATED_BODY()

	FAVVMBoolPayload() = default;
	FAVVMBoolPayload(const FAVVMBoolPayload&) = default;
	FAVVMBoolPayload(FAVVMBoolPayload&&) noexcept = default;
	FAVVMBoolPayload& operator=(const FAVVMBoolPayload&) = default;
	FAVVMBoolPayload& operator=(FAVVMBoolPayload&&) noexcept = default;
	bool operator==(const FAVVMBoolPayload& Rhs) const;
	explicit FAVVMBoolPayload(const bool bNewValue);

	UPROPERTY(Transient, BlueprintReadWrite)
	bool bValue{false};
};
