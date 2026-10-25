import unreal
import importlib

def validate_all_asset_engine_versions(auto_fix=True):
    unreal.log("==================================================")
    unreal.log("Starting Bulk Engine Version Stamp via Root Directories...")
    unreal.log("Scanning /Game, Plugins, GameFeatures, and Custom Mounts...")
    unreal.log("==================================================")
    
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    asset_data_list = asset_registry.get_all_assets()
    
    if not asset_data_list:
        unreal.log_warning("No assets found in the project asset registry.")
        return []

    # Dynamically extract unique root mount folders (e.g., /Game/, /MyPlugin/, /TeamSample/)
    root_folders = set()
    for asset_data in asset_data_list:
        package_name = str(asset_data.package_name)
        if not package_name.startswith("/"):
            continue
        
        parts = package_name.lstrip("/").split("/")
        if parts:
            root_folder = "/" + parts[0] + "/"
            lower_root = root_folder.lower()
            
            # Skip engine and starter content directories and their children entirely
            if "engine" in lower_root or "startercontent" in lower_root:
                continue
                
            root_folders.add(root_folder)

    unreal.log(f"Discovered project root directories to process: {list(root_folders)}")

    success_count = 0
    for root_folder in sorted(root_folders):
        unreal.log(f"Resaving and updating engine version for directory: {root_folder}")
        try:
            # Native C++ batch directory save/resave (recursive)
            # only_if_is_dirty=False forces a complete rewrite to stamp the current engine version
            success = unreal.EditorAssetLibrary.save_directory(root_folder, only_if_is_dirty=False, recursive=True)
            if success:
                success_count += 1
                unreal.log(f"Successfully processed root directory: {root_folder}")
            else:
                unreal.log_warning(f"Directory save returned false for: {root_folder}")
        except Exception as e:
            unreal.log_error(f"Exception while saving directory {root_folder}: {str(e)}")

    unreal.log("==================================================")
    unreal.log(f"Finished processing {success_count} / {len(root_folders)} root directories.")
    unreal.log("==================================================")
    return []

def register_menus():
    tool_menus = unreal.ToolMenus.get()
    
    main_menu = tool_menus.extend_menu("LevelEditor.MainMenu.Tools")
    if main_menu:
        main_menu.add_section("AssetValidationToolsSection", "Asset Validation Tools")
        
        entry = unreal.ToolMenuEntry(
            name="ValidateAssetEngineVersionsMenuEntry",
            type=unreal.MultiBlockType.MENU_ENTRY
        )
        entry.set_label("Validate & Repair Asset Engine Versions")
        entry.set_tool_tip("Scans project content, plugins, and GameFeatures, batch-resaving packages to stamp valid engine versions")
        
        # Auto-reloads the module on click so you never run into caching errors
        entry.set_string_command(
            unreal.ToolMenuStringCommandType.PYTHON,
            "",
            "import importlib; import validate_asset_versions; importlib.reload(validate_asset_versions); validate_asset_versions.validate_all_asset_engine_versions(auto_fix=True)"
        )
        main_menu.add_menu_entry("AssetValidationToolsSection", entry)

    tool_menus.refresh_all_widgets()