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

public class AVVMGameplaySampleRuntime : ModuleRules
{
	public AVVMGameplaySampleRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"AVVM",
				"Core",
				"CoreUObject",
				"Engine",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
			}
		);

		// @gdemers ImGui & AVVMDebugger being DeveloperTool only load on targets where bBuildDeveloperTools
		// is enabled. Set your project editor .target.cs to enable it!
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

			// Tell the compiler we want to import the ImPlot symbols when linking against ImGui plugin 
			PrivateDefinitions.Add(
				string.Format("IMPLOT_API=DLLIMPORT")
			);
		}
	}
}