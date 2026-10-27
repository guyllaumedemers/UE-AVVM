import os
import json
import re
import pathlib
import unreal

def get_project_paths():
    project_dir = pathlib.Path(unreal.Paths.project_dir()).resolve()
    engine_dir = pathlib.Path(unreal.Paths.engine_dir()).resolve()
    return project_dir, engine_dir

def parse_build_cs(build_cs_path):
    """Parses a .Build.cs file to extract dependency module names."""
    dependencies = set()
    if not build_cs_path.exists():
        return dependencies
    
    try:
        content = build_cs_path.read_text(encoding='utf-8', errors='ignore')
        
        # Regex patterns to capture dependency additions in C# Build.cs files
        patterns = [
            r'(?:Public|Private)DependencyModuleNames\s*\.\s*Add\s*\(\s*"([^"]+)"\s*\)',
            r'(?:Public|Private)DependencyModuleNames\s*\.\s*AddRange\s*\(\s*new\s+string\s*\[\s*\]\s*\{([^}]+)\}\s*\)',
            r'(?:Public|Private)DependencyModuleNames\s*[\+\-]?=\s*new\s+string\s*\[\s*\]\s*\{([^}]+)\}'
        ]
        
        for pattern in patterns[:1]:
            for m in re.findall(pattern, content):
                dependencies.add(m)
                
        for pattern in patterns[1:]:
            for block in re.findall(pattern, content):
                for mod in re.findall(r'"([^"]+)"', block):
                    dependencies.add(mod)
                    
    except Exception as e:
        unreal.log_warning(f"Failed to parse {build_cs_path}: {e}")
        
    return dependencies

def scan_engine_modules(engine_dir):
    """Scans engine source and engine plugins to build a valid engine module registry."""
    engine_modules = set()
    
    # Essential engine core modules defaults to ensure baseline coverage
    engine_modules.update({
        "Core", "CoreUObject", "Engine", "InputCore", "Slate", "SlateCore", 
        "EditorSubsystem", "UnrealEd", "Projects", "RenderCore", "RHI", "ApplicationCore",
        "GameplayTags", "GameplayTasks", "AIModule", "UMG", "NavigationSystem"
    })
    
    # Scan Engine/Source
    engine_source = engine_dir / "Source"
    if engine_source.exists():
        for build_file in engine_source.glob("**/*.Build.cs"):
            engine_modules.add(build_file.stem)
            
    # Scan Engine/Plugins
    engine_plugins = engine_dir / "Plugins"
    if engine_plugins.exists():
        for plugin_file in engine_plugins.glob("**/*.uplugin"):
            try:
                data = json.loads(plugin_file.read_text(encoding='utf-8', errors='ignore'))
                for mod in data.get("Modules", []):
                    if "Name" in mod:
                        engine_modules.add(mod["Name"])
            except Exception:
                pass
                
    return engine_modules

def find_project_and_plugin_modules(project_dir):
    """Finds all project modules and project plugin modules."""
    modules = {}
    
    # 1. Parse .uproject
    uproject_files = list(project_dir.glob("*.uproject"))
    if uproject_files:
        try:
            uproject_data = json.loads(uproject_files[0].read_text(encoding='utf-8', errors='ignore'))
            for mod in uproject_data.get("Modules", []):
                mod_name = mod.get("Name")
                if mod_name:
                    build_cs_path = project_dir / "Source" / mod_name / f"{mod_name}.Build.cs"
                    modules[mod_name] = {
                        "type": "Project Module",
                        "build_cs": build_cs_path
                    }
        except Exception as e:
            unreal.log_error(f"Failed to parse .uproject file: {e}")
            
    # Fallback/broad scan of project Source folder
    source_dir = project_dir / "Source"
    if source_dir.exists():
        for build_file in source_dir.glob("**/*.Build.cs"):
            mod_name = build_file.stem
            if mod_name not in modules:
                modules[mod_name] = {
                    "type": "Project Module",
                    "build_cs": build_file
                }

    # 2. Scan Project Plugins (Plugins/ folder)
    plugins_dir = project_dir / "Plugins"
    if plugins_dir.exists():
        for plugin_file in plugins_dir.glob("**/*.uplugin"):
            try:
                plugin_data = json.loads(plugin_file.read_text(encoding='utf-8', errors='ignore'))
                for mod in plugin_data.get("Modules", []):
                    mod_name = mod.get("Name")
                    if mod_name:
                        build_cs_path = plugin_file.parent / "Source" / mod_name / f"{mod_name}.Build.cs"
                        if not build_cs_path.exists():
                            matching_builds = list((plugin_file.parent / "Source").glob(f"**/{mod_name}.Build.cs"))
                            if matching_builds:
                                build_cs_path = matching_builds[0]
                                
                        modules[mod_name] = {
                            "type": f"Plugin Module ({plugin_file.stem})",
                            "build_cs": build_cs_path
                        }
            except Exception as e:
                unreal.log_warning(f"Failed to parse plugin {plugin_file}: {e}")
                
    return modules

def audit_module_dependencies():
    project_dir, engine_dir = get_project_paths()
    unreal.log(f"Starting module dependency audit for project path: {project_dir}")
    
    target_modules = find_project_and_plugin_modules(project_dir)
    unreal.log(f"Auditing {len(target_modules)} target project/plugin module(s).")
    
    valid_modules = scan_engine_modules(engine_dir)
    for mod_name in target_modules.keys():
        valid_modules.add(mod_name)
        
    modules_with_missing_deps = {}
    
    for mod_name, info in target_modules.items():
        build_cs = info["build_cs"]
        dependencies = parse_build_cs(build_cs)
        
        missing = [dep for dep in dependencies if dep not in valid_modules]
        if missing:
            modules_with_missing_deps[mod_name] = {
                "type": info["type"],
                "build_cs": str(build_cs),
                "missing_dependencies": missing
            }
            
    unreal.log("==================================================")
    unreal.log("      PROJECT & PLUGIN MODULE AUDIT RESULTS       ")
    unreal.log("==================================================")
    
    if not modules_with_missing_deps:
        unreal.log("SUCCESS: All project and plugin modules have valid dependencies!")
    else:
        unreal.log_error(f"FAILURE: Found {len(modules_with_missing_deps)} module(s) with missing dependency references:")
        for mod_name, data in modules_with_missing_deps.items():
            unreal.log_error(f"  - [{data['type']}] {mod_name}")
            unreal.log_error(f"    Path: {data['build_cs']}")
            unreal.log_error(f"    Missing References: {', '.join(data['missing_dependencies'])}")
            
    unreal.log("==================================================")
    return modules_with_missing_deps

# ==========================================
# TOOL MENU INTEGRATION
# ==========================================

def register_menus():
    tool_menus = unreal.ToolMenus.get()
    main_menu = tool_menus.extend_menu("LevelEditor.MainMenu.Tools")
    if main_menu:
        main_menu.add_section("ModuleAuditingToolsSection", "Module Auditing")
        
        entry = unreal.ToolMenuEntry(
            name="AuditModuleDependenciesMenuEntry",
            type=unreal.MultiBlockType.MENU_ENTRY
        )
        entry.set_label("Audit Module Dependencies")
        entry.set_tool_tip("Scans project and plugin modules for broken or missing .Build.cs dependency references.")
        entry.set_string_command(
            unreal.ToolMenuStringCommandType.PYTHON,
            "",
            "import AuditModuleDependencies; AuditModuleDependencies.audit_module_dependencies()"
        )
        main_menu.add_menu_entry("ModuleAuditingToolsSection", entry)

    tool_menus.refresh_all_widgets()