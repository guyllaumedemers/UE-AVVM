import unreal
import os

def audit_textures(path_filter="/Game"):
    """
    Scans textures in the specified content path and flags budget/streaming violations.
    """
    unreal.log(f"==================================================")
    unreal.log(f"Starting Texture Memory & Streaming Audit on: {path_filter}")
    unreal.log(f"==================================================")

    # Asset Registry Setup
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    
    # Filter for UTexture2D assets
    filter_struct = unreal.ARFilter(
        package_paths=[path_filter],
        class_names=["Texture2D"],
        recursive_paths=True
    )
    
    assets = asset_registry.get_assets(filter_struct)
    unreal.log(f"Found {len(assets)} texture assets to evaluate.\n")

    flagged_count = 0
    
    for asset_data in assets:
        asset = asset_data.get_asset()
        if not isinstance(asset, unreal.Texture2D):
            continue
            
        asset_name = asset.get_name()
        path = asset_data.package_name
        
        # Extract properties safely using correct UE Python methods
        width = asset.blueprint_get_size_x()
        height = asset.blueprint_get_size_y()
        never_stream = asset.get_editor_property("never_stream")
        max_size = asset.get_editor_property("max_texture_size")
        
        issues = []

        # 1. Check Power of Two (PoT)
        is_pot_w = (width & (width - 1) == 0) and width != 0
        is_pot_h = (height & (height - 1) == 0) and height != 0
        if not (is_pot_w and is_pot_h):
            issues.append(f"Non-Power-of-Two ({width}x{height})")

        # 2. Check Over-Budget Resolution without Max Size Clamp
        if (width > 2048 or height > 2048) and max_size == 0:
            issues.append(f"High-Res ({width}x{height}) with NO Max Texture Size override")

        # 3. Check Streaming Disabled on Large Textures
        if never_stream and (width > 512 or height > 512):
            issues.append(f"NeverStream=True on large texture ({width}x{height}) - bypasses streaming pool")

        # Report if any issues found using warning level logging
        if issues:
            flagged_count += 1
            unreal.log_warning(f"[FLAGGED] Asset: {asset_name} | Path: {path}")
            unreal.log_warning(f"  Resolution: {width}x{height} | MaxSize Setting: {max_size} | NeverStream: {never_stream}")
            for issue in issues:
                unreal.log_warning(f"  -> Issue: {issue}")
            unreal.log_warning("-" * 50)

    unreal.log(f"\nAudit Complete. Flagged {flagged_count} out of {len(assets)} textures for review.")

def register_menus():
    """Registers the texture audit tool into the Unreal Editor ToolMenus system."""
    tool_menus = unreal.ToolMenus.get()
    
    main_menu = tool_menus.extend_menu("LevelEditor.MainMenu.Tools")
    if main_menu:
        section_name = "TextureAuditToolsSection"
        main_menu.add_section(section_name, "Texture Audit Tools")
        
        entry = unreal.ToolMenuEntry(
            name="TextureMemoryAuditMenuEntry",
            type=unreal.MultiBlockType.MENU_ENTRY
        )
        entry.set_label("Run Texture Memory & Streaming Audit")
        entry.set_tool_tip("Scans assets in /Game for non-power-of-two resolutions, unclamped high-res textures, and disabled streaming.")
        
        # Auto-reloads the module on click so you never run into caching errors
        entry.set_string_command(
            unreal.ToolMenuStringCommandType.PYTHON,
            unreal.Name(""),
            "import importlib; import validate_texture_audit; importlib.reload(validate_texture_audit); validate_texture_audit.audit_textures('/Game')"
        )
        main_menu.add_menu_entry(section_name, entry)

    tool_menus.refresh_all_widgets()
    unreal.log("Successfully registered Texture Memory Audit to Unreal Tools menu.")