import unreal
import uuid
import os

def check_out_file_safely(file_path):
    """Helper to check out a file safely if source control is active."""
    if not unreal.SourceControl.is_enabled():
        return True
        
    disk_path = unreal.PackageTools.package_name_to_filename(file_path) if not file_path.endswith((".uasset", ".uexp")) else file_path
    if not disk_path or not os.path.exists(disk_path):
        return True

    sc_state = unreal.SourceControl.query_file_state(disk_path)
    if sc_state and sc_state.is_valid:
        if sc_state.is_checked_out_other:
            unreal.log_error(f"VCS ERROR: File is checked out by another user: {disk_path}")
            return False
        if sc_state.is_source_controlled and not sc_state.is_checked_out:
            success = unreal.SourceControl.check_out_file(disk_path)
            if not success:
                unreal.log_error(f"VCS ERROR: Failed to check out file: {disk_path}")
                return False
    return True

def convert_registries():
    unreal.log("Running Data Registry safe migration (Staged New Asset + Sub-object Duplication workflow)...")
    
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    editor_asset_lib = unreal.EditorAssetLibrary
    
    target_class_path = "/Script/AVVMGameplay.AVVMDataRegistry"
    custom_registry_class = unreal.load_class(None, target_class_path)
    
    if not custom_registry_class:
        unreal.log_error(f"Failed to load custom registry class: {target_class_path}")
        return False

    ar_filter = unreal.ARFilter(
        class_paths=[unreal.TopLevelAssetPath("/Script/DataRegistry", "DataRegistry")],
        recursive_paths=True
    )
    
    asset_data_list = asset_registry.get_assets(ar_filter)
    if not asset_data_list:
        unreal.log_warning("No legacy DataRegistry assets found to convert.")
        return True

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    project_root_dir = os.path.abspath(os.path.join(unreal.Paths.project_dir()))
    locked_files_for_batch = []

    for asset_data in asset_data_list:
        asset_path = str(asset_data.package_name)
        source_asset = editor_asset_lib.load_asset(asset_path)
        
        if not source_asset:
            continue

        package_path = str(asset_data.package_path)
        asset_name = unreal.SystemLibrary.get_object_name(source_asset)
        
        # Skip if already converted or archived
        if source_asset.get_class() == custom_registry_class or "_Archived" in asset_name:
            source_asset = None
            continue

        unreal.log(f"Processing asset: {asset_name}")

        # Step 1: Extract and cache configuration properties from original asset
        item_struct = None
        data_sources = None
        registry_type = None
        
        for prop_name in ["item_struct", "ItemStruct"]:
            try:
                item_struct = source_asset.get_editor_property(prop_name)
                if item_struct is not None: break
            except Exception:
                pass

        for prop_name in ["data_sources", "DataSources"]:
            try:
                raw_ds = source_asset.get_editor_property(prop_name)
                if raw_ds is not None:
                    data_sources = list(raw_ds) if isinstance(raw_ds, (list, tuple, unreal.Array)) else raw_ds
                    break
            except Exception:
                pass

        for prop_name in ["registry_type", "RegistryType", "registry_id_format", "RegistryIdFormat"]:
            try:
                registry_type = source_asset.get_editor_property(prop_name)
                if registry_type is not None: break
            except Exception:
                pass

        # Step 2: Create the NEW custom asset with a temporary unique name (ZERO overwrite prompts!)
        unique_suffix = str(uuid.uuid4())[:6]
        temp_new_name = f"{asset_name}_New_{unique_suffix}"
        
        new_asset = asset_tools.create_asset(temp_new_name, package_path, custom_registry_class, None)
        if not new_asset:
            unreal.log_error(f"Failed to create staged new asset for {asset_name}")
            source_asset = None
            continue

        # Step 3: Populate cached properties into the new asset (with sub-object duplication for DataSources)
        try:
            if item_struct is not None:
                for struct_prop in ["item_struct", "ItemStruct"]:
                    try:
                        new_asset.set_editor_property(struct_prop, item_struct)
                        break
                    except Exception:
                        pass
            
            if data_sources is not None:
                assigned_ds = False
                for ds_prop in ["data_sources", "DataSources"]:
                    try:
                        # Duplicate instanced objects so they belong to the new asset package outer
                        new_data_sources = []
                        for ds in data_sources:
                            if isinstance(ds, unreal.Object):
                                duplicated_ds = unreal.SystemLibrary.duplicate_object(ds, new_asset, "")
                                new_data_sources.append(duplicated_ds)
                            else:
                                new_data_sources.append(ds)

                        new_asset.set_editor_property(ds_prop, new_data_sources)
                        assigned_ds = True
                        unreal.log(f"Successfully duplicated and assigned data_sources for {asset_name}")
                        break
                    except Exception as e:
                        try:
                            target_array = new_asset.get_editor_property(ds_prop)
                            for ds in data_sources:
                                if isinstance(ds, unreal.Object):
                                    duplicated_ds = unreal.SystemLibrary.duplicate_object(ds, new_asset, "")
                                    target_array.append(duplicated_ds)
                                else:
                                    target_array.append(ds)
                            new_asset.set_editor_property(ds_prop, target_array)
                            assigned_ds = True
                            unreal.log(f"Successfully assigned data_sources via item duplication for {asset_name}")
                            break
                        except Exception:
                            pass
                if not assigned_ds:
                    unreal.log_error(f"CRITICAL: Failed to assign data_sources for {asset_name}!")

            if registry_type is not None:
                for prop_name in ["registry_type", "RegistryType", "registry_id_format", "RegistryIdFormat"]:
                    try:
                        new_asset.set_editor_property(prop_name, registry_type)
                        break
                    except Exception:
                        pass
        except Exception as e:
            unreal.log_error(f"Error setting properties on {asset_name}: {e}")

        editor_asset_lib.save_loaded_asset(new_asset)
        new_asset = None
        source_asset = None
        unreal.SystemLibrary.collect_garbage()

        # Step 4: Handle original asset AFTER new asset is fully created and saved
        archive_name = f"{asset_name}_Archived"
        archive_path = f"{package_path}/{archive_name}"
        if editor_asset_lib.does_asset_exist(archive_path):
            archive_path = f"{package_path}/{asset_name}_Archived_{unique_suffix}"

        if check_out_file_safely(asset_path):
            success_rename = editor_asset_lib.rename_asset(asset_path, archive_path)
            if not success_rename:
                raw_disk_path = unreal.PackageTools.package_name_to_filename(asset_path)
                if raw_disk_path:
                    base_disk_path = os.path.splitext(os.path.normpath(raw_disk_path))[0]
                    for ext in [".uasset", ".uexp", ".ubulk"]:
                        f_path = base_disk_path + ext
                        if os.path.exists(f_path):
                            locked_files_for_batch.append(f_path)
        else:
            raw_disk_path = unreal.PackageTools.package_name_to_filename(asset_path)
            if raw_disk_path:
                base_disk_path = os.path.splitext(os.path.normpath(raw_disk_path))[0]
                for ext in [".uasset", ".uexp", ".ubulk"]:
                    f_path = base_disk_path + ext
                    if os.path.exists(f_path):
                        locked_files_for_batch.append(f_path)

        unreal.SystemLibrary.collect_garbage()

        # Step 5: Rename the temporary new asset into the final original path
        temp_new_path = f"{package_path}/{temp_new_name}"
        final_asset_path = f"{package_path}/{asset_name}"
        
        if editor_asset_lib.does_asset_exist(final_asset_path):
            editor_asset_lib.delete_asset(final_asset_path)

        success_final_rename = editor_asset_lib.rename_asset(temp_new_path, final_asset_path)
        if not success_final_rename:
            unreal.log_error(f"CRITICAL: Failed to move new asset into final path for {asset_name}")
            continue

        # Final load and save
        final_loaded = editor_asset_lib.load_asset(final_asset_path)
        if final_loaded:
            editor_asset_lib.save_loaded_asset(final_loaded)
            if unreal.SourceControl.is_enabled():
                new_file_disk = unreal.PackageTools.package_name_to_filename(final_asset_path)
                if new_file_disk and os.path.exists(new_file_disk):
                    try:
                        os.chmod(new_file_disk, 0o777)
                        unreal.SourceControl.mark_file_for_add(new_file_disk)
                    except Exception:
                        pass
            final_loaded = None

        unreal.SystemLibrary.collect_garbage()
        unreal.log(f"Successfully converted, duplicated sub-objects, and swapped {asset_name}!")

    # Generate batch script if any files were locked by the session
    if locked_files_for_batch:
        batch_script_path = os.path.join(project_root_dir, "PurgeArchivedAssets.bat")
        try:
            with open(batch_script_path, "w") as f:
                f.write("@echo off\n")
                f.write("echo Purging locked Unreal legacy assets...\n")
                for locked_file in locked_files_for_batch:
                    f.write(f'del /f /q "{locked_file}"\n')
                f.write("echo Cleanup complete!\n")
                f.write("pause\n")
            unreal.log_warning(f"-> Generated absolute batch script at: {batch_script_path}")
            unreal.log_warning("-> Close the editor and double-click 'PurgeArchivedAssets.bat' to wipe remaining locked legacy files!")
        except Exception as e:
            unreal.log_error(f"Failed to generate cleanup batch script: {e}")
            
    unreal.log("Data Registry batch conversion complete!")
    return True

def cleanup_archived_assets():
    unreal.log("Cleaning up archived legacy DataRegistry assets...")
    editor_asset_lib = unreal.EditorAssetLibrary
    asset_data_list = editor_asset_lib.list_assets("/Game", recursive=True)
    deleted_count = 0

    for asset_path in asset_data_list:
        if "_Archived" in asset_path:
            if not check_out_file_safely(asset_path):
                continue
            try:
                if editor_asset_lib.delete_asset(asset_path):
                    deleted_count += 1
                    unreal.log(f"Successfully deleted archived asset: {asset_path}")
            except Exception as e:
                unreal.log_warning(f"Could not delete archived asset {asset_path}: {e}")
                
    unreal.log(f"Archive cleanup pass complete! Removed {deleted_count} assets.")
    return True

def run_full_migration():
    unreal.log("==================================================")
    unreal.log("Starting Full Data Registry Migration & Cleanup...")
    unreal.log("==================================================")
    
    conversion_success = convert_registries()
    
    if conversion_success:
        unreal.log("Conversion phase completed successfully. Proceeding to archive cleanup...")
        cleanup_archived_assets()
    else:
        unreal.log_error("Conversion phase encountered errors. Skipping automatic cleanup.")
        
    unreal.log("==================================================")
    unreal.log("Full Data Registry Migration Workflow Finished!")
    unreal.log("==================================================")

def register_menus():
    tool_menus = unreal.ToolMenus.get()
    main_menu = tool_menus.extend_menu("LevelEditor.MainMenu.Tools")
    if main_menu:
        main_menu.add_section("CustomDataRegistryToolsSection", "Data Registry Tools")
        
        entry_full = unreal.ToolMenuEntry(
            name="RunFullDataRegistryMigrationMenuEntry",
            type=unreal.MultiBlockType.MENU_ENTRY
        )
        entry_full.set_label("Run Full Data Registry Migration & Cleanup")
        entry_full.set_tool_tip("Converts legacy assets using staged creation and sub-object duplication")
        entry_full.set_string_command(
            unreal.ToolMenuStringCommandType.PYTHON,
            "",
            "import convert_registries; convert_registries.run_full_migration()"
        )
        main_menu.add_menu_entry("CustomDataRegistryToolsSection", entry_full)

    tool_menus.refresh_all_widgets()