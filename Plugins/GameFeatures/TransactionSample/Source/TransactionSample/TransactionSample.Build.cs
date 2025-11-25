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

using UnrealBuildTool;

public class TransactionSample : ModuleRules
{
	public TransactionSample(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		SetupIrisSupport(Target);

		if (Target.bBuildEditor)
		{
			PublicDependencyModuleNames.AddRange(new string[] { "UnrealEd", });
		}

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"AVVM",
				"AVVMGameplay",
				"Core",
				"CoreUObject",
				"DeveloperSettings",
				"Engine",
			}
		);


		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"GameplayTags",
				"Json"
			}
		);

		if (Target.bBuildDeveloperTools)
		{
			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"AVVMDebugger",
				});

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"ImGui"
				});

			PrivateDefinitions.Add(
				string.Format("IMPLOT_API=DLLIMPORT")
			);
		}
	}
}