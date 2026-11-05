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

def convert_datatables_to_avvm():
    unreal.log("Running UDataTable to UAVVMDataTable migration and data preservation workflow...")
    
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    editor_asset_lib = unreal.EditorAssetLibrary
    
    target_class_path = "/Script/AVVMGameplay.AVVMDataTable"
    custom_datatable_class = unreal.load_class(None, target_class_path)
    
    if not custom_datatable_class:
        unreal.log_error(f"Failed to load custom UAVVMDataTable class: {target_class_path}")
        return False

    # Find standard Engine UDataTable assets to convert/reparent
    ar_filter = unreal.ARFilter(
        class_paths=[unreal.TopLevelAssetPath("/Script/Engine", "DataTable")],
        recursive_paths=True
    )
    
    asset_data_list = asset_registry.get_assets(ar_filter)
    if not asset_data_list:
        unreal.log_warning("No standard UDataTable assets found to convert.")
        return True

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    project_root_dir = os.path.abspath(os.path.join(unreal.Paths.project_dir()))
    archived_files_for_batch = []

    for asset_data in asset_data_list:
        asset_path = str(asset_data.package_name)
        source_asset = editor_asset_lib.load_asset(asset_path)
        
        if not source_asset:
            continue

        package_path = str(asset_data.package_path)
        asset_name = unreal.SystemLibrary.get_object_name(source_asset)
        
        # Skip if already using the custom class or archived
        if source_asset.get_class() == custom_datatable_class or "_Archived" in asset_name:
            source_asset = None
            continue

        unreal.log(f"Converting UDataTable asset to UAVVMDataTable: {asset_name}")

        # Step 1: Extract row_struct and export row data from original UDataTable
        row_struct = None
        for prop_name in ["row_struct", "RowStruct"]:
            try:
                row_struct = source_asset.get_editor_property(prop_name)
                if row_struct is not None: 
                    break
            except Exception:
                pass

        # Extract table row contents via CSV string serialization
        csv_data_string = None
        try:
            if hasattr(source_asset, "export_to_csv_string"):
                csv_data_string = source_asset.export_to_csv_string()
            else:
                csv_data_string = unreal.DataTableFunctionLibrary.export_data_table_to_csv_string(source_asset)
        except Exception as e:
            unreal.log_warning(f"Could not export row data for {asset_name}: {e}")

        # Step 2: Create the NEW custom UAVVMDataTable asset directly using the custom class
        unique_suffix = str(uuid.uuid4())[:6]
        temp_new_name = f"{asset_name}_New_{unique_suffix}"
        
        new_asset = asset_tools.create_asset(temp_new_name, package_path, custom_datatable_class, None)
        if not new_asset:
            unreal.log_error(f"Failed to create staged UAVVMDataTable asset for {asset_name}")
            source_asset = None
            continue

        # Step 3: Populate row_struct and row data contents
        if row_struct is not None:
            for struct_prop in ["row_struct", "RowStruct"]:
                try:
                    new_asset.set_editor_property(struct_prop, row_struct)
                    break
                except Exception:
                    pass

        if csv_data_string:
            try:
                if hasattr(new_asset, "fill_from_csv_string"):
                    new_asset.fill_from_csv_string(csv_data_string, row_struct)
                else:
                    unreal.DataTableFunctionLibrary.fill_data_table_from_csv_string(new_asset, csv_data_string, row_struct)
                unreal.log(f"Successfully populated row contents for UAVVMDataTable: {asset_name}")
            except Exception as e:
                unreal.log_error(f"Failed to populate row data into new UAVVMDataTable for {asset_name}: {e}")

        editor_asset_lib.save_loaded_asset(new_asset)
        new_asset = None
        source_asset = None
        unreal.SystemLibrary.collect_garbage()

        # Step 4: Handle original asset checkout and archiving
        check_out_file_safely(asset_path)
        
        archive_name = f"{asset_name}_Archived"
        archive_path = f"{package_path}/{archive_name}"
        if editor_asset_lib.does_asset_exist(archive_path):
            archive_path = f"{package_path}/{asset_name}_Archived_{unique_suffix}"

        success_rename = editor_asset_lib.rename_asset(asset_path, archive_path)
        
        # Resolve absolute disk paths specifically for the newly created _Archived asset
        archive_disk_pkg_path = unreal.PackageTools.package_name_to_filename(archive_path)
        if archive_disk_pkg_path:
            abs_archive_path = unreal.Paths.convert_relative_path_to_full(archive_disk_pkg_path)
            base_disk_path = os.path.splitext(os.path.normpath(abs_archive_path))[0]
            for ext in [".uasset", ".uexp", ".ubulk"]:
                f_path = base_disk_path + ext
                if os.path.exists(f_path):
                    archived_files_for_batch.append(f_path)

        unreal.SystemLibrary.collect_garbage()

        # Step 5: Rename the temporary new asset into the final original path
        temp_new_path = f"{package_path}/{temp_new_name}"
        final_asset_path = f"{package_path}/{asset_name}"
        
        if editor_asset_lib.does_asset_exist(final_asset_path):
            editor_asset_lib.delete_asset(final_asset_path)

        success_final_rename = editor_asset_lib.rename_asset(temp_new_path, final_asset_path)
        if not success_final_rename:
            unreal.log_error(f"CRITICAL: Failed to move new UAVVMDataTable asset into final path for {asset_name}")
            continue

        # Final load and save
        final_loaded = editor_asset_lib.load_asset(final_asset_path)
        if final_loaded:
            editor_asset_lib.save_loaded_asset(final_loaded)
            if unreal.SourceControl.is_enabled():
                new_file_disk = unreal.PackageTools.package_name_to_filename(final_asset_path)
                if new_file_disk and os.path.exists(new_file_disk):
                    try:
                        abs_new_disk = unreal.Paths.convert_relative_path_to_full(new_file_disk)
                        os.chmod(abs_new_disk, 0o777)
                        unreal.SourceControl.mark_file_for_add(abs_new_disk)
                    except Exception:
                        pass
            final_loaded = None

        unreal.SystemLibrary.collect_garbage()
        unreal.log(f"Successfully converted UDataTable {asset_name} to UAVVMDataTable!")

    # Generate batch script containing absolute disk paths for the archived assets
    if archived_files_for_batch:
        batch_script_path = os.path.join(project_root_dir, "PurgeArchivedAssets.bat")
        try:
            with open(batch_script_path, "w") as f:
                f.write("@echo off\n")
                f.write("echo Purging archived legacy DataTable assets...\n")
                for archived_file in sorted(list(set(archived_files_for_batch))):
                    f.write(f'del /f /q "{archived_file}"\n')
                f.write("echo Cleanup complete!\n")
                f.write("pause\n")
            unreal.log_warning(f"-> Generated absolute batch script at: {batch_script_path}")
            unreal.log_warning("-> Close the editor and double-click 'PurgeArchivedAssets.bat' to wipe remaining _Archived legacy files!")
        except Exception as e:
            unreal.log_error(f"Failed to generate cleanup batch script: {e}")
            
    unreal.log("UDataTable to UAVVMDataTable batch processing complete!")
    return True

def cleanup_archived_assets():
    unreal.log("Cleaning up archived legacy UDataTable assets...")
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
    unreal.log("Starting Full UDataTable to UAVVMDataTable Migration...")
    unreal.log("==================================================")
    
    conversion_success = convert_datatables_to_avvm()
    
    if conversion_success:
        unreal.log("Conversion phase completed successfully. Proceeding to archive cleanup...")
        cleanup_archived_assets()
    else:
        unreal.log_error("Conversion phase encountered errors. Skipping automatic cleanup.")
        
    unreal.log("==================================================")
    unreal.log("Full Migration Workflow Finished!")
    unreal.log("==================================================")

def register_menus():
    tool_menus = unreal.ToolMenus.get()
    main_menu = tool_menus.extend_menu("LevelEditor.MainMenu.Tools")
    if main_menu:
        main_menu.add_section("CustomAVVMDataTableToolsSection", "AVVM DataTable Tools")
        
        entry_full = unreal.ToolMenuEntry(
            name="RunFullAVVMDataTableMigrationMenuEntry",
            type=unreal.MultiBlockType.MENU_ENTRY
        )
        entry_full.set_label("Run UDataTable to UAVVMDataTable Migration")
        entry_full.set_tool_tip("Converts UDataTable assets to extend from UAVVMDataTable and preserves row contents")
        entry_full.set_string_command(
            unreal.ToolMenuStringCommandType.PYTHON,
            "",
            "import convert_datatables; convert_datatables.run_full_migration()"
        )
        main_menu.add_menu_entry("CustomAVVMDataTableToolsSection", entry_full)

    tool_menus.refresh_all_widgets()