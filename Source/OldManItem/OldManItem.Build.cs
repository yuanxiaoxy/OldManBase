using UnrealBuildTool;

public class OldManItem : ModuleRules
{
    public OldManItem(ReadOnlyTargetRules Target) : base(Target)
    {
        //Public module names that this module uses.
        //In case you would like to add various classes that you're going to use in your game
        //you should add the core,coreuobject and engine dependencies.
        //Don't forget to add your project's name as a dependency (GDBlogPost in my case)
        PublicDependencyModuleNames.AddRange(new string[] 
        { 
            "Core", 
            "CoreUObject", 
            "Engine", 
        });

        //The path for the header files
        PublicIncludePaths.AddRange(new string[] { "OldManItem/Public" });

        //The path for the source files
        PrivateIncludePaths.AddRange(new string[] { "OldManItem/Private" });
    }
}