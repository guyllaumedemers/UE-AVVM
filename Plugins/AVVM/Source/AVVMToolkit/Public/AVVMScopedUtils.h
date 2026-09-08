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

struct FAVVMGameThreadLock;

/**
 *	Class description:
 *
 *	FAVVMScopedDelegate is a scoped object that execute a laten delegate during destruction.
 */
struct AVVMTOOLKIT_API FAVVMScopedDelegate : public FNoncopyable
{
	FAVVMScopedDelegate(FSimpleDelegate&& NewCallback);
	FAVVMScopedDelegate() = default;
	FAVVMScopedDelegate(FAVVMScopedDelegate&&) noexcept = default;
	FAVVMScopedDelegate& operator=(FAVVMScopedDelegate&&) noexcept = default;
	~FAVVMScopedDelegate();

private:
	FSimpleDelegate OutOfScopeDelegate{};
};

/**
 *	Class description:
 *
 *	FAVVMScopedLock is a utility offering scope safety for locking/unlocking behaviour.
 */
struct AVVMTOOLKIT_API FAVVMScopedLock : public FNoncopyable
{
	FAVVMScopedLock(FAVVMGameThreadLock* NewHandle);
	FAVVMScopedLock(FAVVMGameThreadLock* NewHandle, FSimpleDelegate&& NewCallback);
	FAVVMScopedLock() = default;
	FAVVMScopedLock(FAVVMScopedLock&&) noexcept = default;
	FAVVMScopedLock& operator=(FAVVMScopedLock&&) noexcept = default;
	~FAVVMScopedLock();

private:
	FAVVMGameThreadLock* Handle = nullptr;
	FSimpleDelegate Callback{};
};

/**
 *	Class description:
 *
 *	FAVVMGameThreadLock is a utility allowing management of async processes request, and should be used
 *	to prevent racing condition between game related updates and data received from backend.
 */
struct AVVMTOOLKIT_API FAVVMGameThreadLock : public FNoncopyable
{
	FAVVMGameThreadLock() = default;
	FAVVMGameThreadLock(FAVVMGameThreadLock&&) noexcept = default;
	FAVVMGameThreadLock& operator=(FAVVMGameThreadLock&&) noexcept = default;
	
	FAVVMScopedLock Make();
	FAVVMScopedLock Make(FSimpleDelegate&& NewCallback);

	void Lock();
	void UnLock();
	bool IsLocked() const;

private:
	bool bIsRunning = false;
};
