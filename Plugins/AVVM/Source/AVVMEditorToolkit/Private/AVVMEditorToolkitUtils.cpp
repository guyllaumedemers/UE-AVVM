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
#include "AVVMEditorToolkitUtils.h"

#include "DataRegistrySource_DataTable.h"
#include "DataRegistrySubsystem.h"

TArray<const UDataRegistrySource_DataTable*> UAVVMEditorToolkitUtils::GetDataTables(const FDataRegistryType& RegistryType)
{
	TArray<const UDataRegistrySource_DataTable*> OutResult{};

	const auto* Subsystem = UDataRegistrySubsystem::Get();
	if (!IsValid(Subsystem))
	{
		return OutResult;
	}

	const auto* DataRegistry = Subsystem->GetRegistryForType(RegistryType);
	if (!IsValid(DataRegistry))
	{
		return OutResult;
	}

	// @gdemers sadly, the DataRegistry class doesnt allow direct access to the DataSources property.
	TArray<FDataRegistrySourceItemId> OutElements{};
	DataRegistry->GetAllSourceItems(OutElements);

	for (const auto& Element : OutElements)
	{
		const auto* Source = Cast<UDataRegistrySource_DataTable>(Element.CachedSource);
		if (IsValid(Source))
		{
			OutResult.AddUnique(Source);
		}
	}

	return OutResult;
}
