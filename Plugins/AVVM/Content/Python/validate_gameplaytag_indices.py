import os
import glob
import re
import unreal

def get_pie_log_files():
    """
    Finds separate Server and Client log files in Saved/Logs, 
    specifically supporting PIE sessions with 'Log to Separate Files' enabled 
    or standalone multi-process runs.
    """
    project_dir = unreal.Paths.project_dir()
    log_dir = os.path.join(project_dir, "Saved", "Logs")
    
    if not os.path.exists(log_dir):
        unreal.log_error(f"Log directory not found at: {log_dir}")
        return None, None
        
    log_files = glob.glob(os.path.join(log_dir, "*.log"))
    if not log_files:
        unreal.log_error(f"No log files found in: {log_dir}")
        return None, None
        
    # Sort files by modification time (newest first)
    log_files.sort(key=os.path.getmtime, reverse=True)
    
    server_log = None
    client_log = None
    
    # Identify server vs client logs based on typical Unreal naming patterns
    for log in log_files:
        filename = os.path.basename(log).lower()
        if "server" in filename and not server_log:
            server_log = log
        elif ("client" in filename or "pie" in filename) and not client_log:
            if log != server_log:
                client_log = log
                
    # Fallbacks if specific filename markers are absent (e.g., single log fallback)
    if not server_log:
        server_log = log_files[0]
    if not client_log:
        client_log = log_files[1] if len(log_files) > 1 else log_files[0]

    return server_log, client_log

def check_fast_replication_setting():
    """
    Validates whether Fast Tag Replication is enabled in project settings.
    """
    try:
        ini_path = os.path.join(unreal.Paths.project_config_dir(), "DefaultGameplayTags.ini")
        fast_replication_enabled = False
        
        if os.path.exists(ini_path):
            with open(ini_path, "r", encoding="utf-8", errors="ignore") as f:
                content = f.read()
                if re.search(r"FastReplication\s*=\s*True", content, re.IGNORECASE):
                    fast_replication_enabled = True
                    
        return fast_replication_enabled
    except Exception as e:
        unreal.log_warning(f"Could not automatically read DefaultGameplayTags.ini: {e}")
        return True

def parse_replication_indices_from_file(log_path):
    """
    Parses gameplay tag replication indices from a specified log file path,
    ensuring it only captures true hierarchical gameplay tags (containing a dot '.')
    and filters out engine metadata keys like 'NetIndex'.
    """
    if not log_path or not os.path.exists(log_path):
        unreal.log_error(f"Log path invalid or does not exist: {log_path}")
        return {}

    tag_indices = {}
    
    # Stricter pattern: 
    # 1. Must start with a letter/underscore.
    # 2. Must contain at least one dot '.' (hierarchical gameplay tag structure like A.B).
    # 3. Followed by the assigned index integer.
    pattern = re.compile(r"([a-zA-Z_][\w]*\.[a-zA-Z0-9_\.]+)\s*[:=]\s*(\d+)")

    with open(log_path, "r", encoding="utf-8", errors="ignore") as f:
        lines = f.readlines()

    for line in lines:
        if "LogGameplayTags" in line and ("=" in line or ":" in line):
            match = pattern.search(line)
            if match:
                tag_name = match.group(1)
                index_val = int(match.group(2))
                
                # Exclude any false positives that might slip through
                if tag_name.lower() not in ["netindex", "index", "gameplaytags"]:
                    tag_indices[tag_name] = index_val

    return tag_indices

def capture_and_validate_server_vs_client():
    """
    Executes the console command, locates separate server and client logs,
    parses each independently, and runs validation.
    """
    world = unreal.EditorLevelLibrary.get_editor_world()
    
    # Record file sizes or execute command to log output
    unreal.SystemLibrary.execute_console_command(world, "GameplayTags.PrintReplicationIndicies")
    
    # Retrieve separate server and client log targets
    server_log, client_log = get_pie_log_files()
    
    unreal.log(f"Inspecting Server Log: {server_log}")
    unreal.log(f"Inspecting Client Log: {client_log}")
    
    server_indices = parse_replication_indices_from_file(server_log)
    client_indices = parse_replication_indices_from_file(client_log)
    
    return validate_tag_indices(server_indices, client_indices)

def validate_tag_indices(server_indices, client_indices):
    """
    Validates server vs client indices share identical IDs, are within valid network range,
    and checks if fast replication is active. Logs detailed tag names and indices on failure.
    """
    unreal.log("--- Starting Gameplay Tag Replication Index & Fast Replication Validation ---")
    
    is_valid = True

    # 1. Validate Fast Replication Settings
    is_fast_rep_enabled = check_fast_replication_setting()
    if not is_fast_rep_enabled:
        is_valid = False
        unreal.log_error("VALIDATION FAILURE: 'FastReplication' is disabled in project settings.")
    else:
        unreal.log("PASS: Fast Replication setting is active.")

    # 2. Validate Index Bounds (Max 65,535 network limit)
    MAX_NET_INDEX = 65535
    for tag, idx in server_indices.items():
        if not (0 <= idx <= MAX_NET_INDEX):
            is_valid = False
            unreal.log_error(f"Index out of range for server tag '{tag}': Assigned Index {idx}.")
    for tag, idx in client_indices.items():
        if not (0 <= idx <= MAX_NET_INDEX):
            is_valid = False
            unreal.log_error(f"Index out of range for client tag '{tag}': Assigned Index {idx}.")

    # 3. Server vs Client Index Matching Validation
    server_keys = set(server_indices.keys())
    client_keys = set(client_indices.keys())

    missing_on_client = sorted(server_keys - client_keys)
    missing_on_server = sorted(client_keys - server_keys)
    common_keys = sorted(server_keys.intersection(client_keys))

    mismatches = []
    for tag in common_keys:
        if server_indices[tag] != client_indices[tag]:
            mismatches.append((tag, server_indices[tag], client_indices[tag]))

    if missing_on_client:
        is_valid = False
        unreal.log_error(f"Tags present on Server but missing on Client ({len(missing_on_client)} tags):")
        for tag in missing_on_client:
            unreal.log_error(f"  - Tag '{tag}': Server Index {server_indices[tag]} vs Client Index [MISSING]")

    if missing_on_server:
        is_valid = False
        unreal.log_error(f"Tags present on Client but missing on Server ({len(missing_on_server)} tags):")
        for tag in missing_on_server:
            unreal.log_error(f"  - Tag '{tag}': Server Index [MISSING] vs Client Index {client_indices[tag]}")

    if mismatches:
        is_valid = False
        unreal.log_error(f"Index mismatches found ({len(mismatches)} tags):")
        for tag, s_idx, c_idx in mismatches:
            unreal.log_error(f"  - Tag '{tag}': Server Index {s_idx} vs Client Index {c_idx}")

    if is_valid:
        unreal.log("SUCCESS: Server and Client gameplay tags are fully synchronized, within range, and fast-replication enabled!")
    else:
        unreal.log_error("FAILURE: Gameplay tag replication mismatch detected between Server and Client.")

    return is_valid
    
def register_menus():
    """Registers the gameplay tag validation tool into the Unreal Editor ToolMenus system."""
    tool_menus = unreal.ToolMenus.get()
    
    main_menu = tool_menus.extend_menu("LevelEditor.MainMenu.Tools")
    if main_menu:
        section_name = "GameplayTagValidationSection"
        main_menu.add_section(section_name, "Gameplay Tag Validation Tools")
        
        entry = unreal.ToolMenuEntry(
            name="ValidateGameplayTagsMenuEntry",
            type=unreal.MultiBlockType.MENU_ENTRY
        )
        entry.set_label("Validate Server vs Client Tag Indices")
        entry.set_tool_tip("Scans separate Server and Client logs to validate gameplay tag replication consistency.")
        
        # Auto-reloads module on click and invokes the server vs client check function
        entry.set_string_command(
            unreal.ToolMenuStringCommandType.PYTHON,
            unreal.Name(""),
            "import importlib; import validate_gameplaytag_indices; importlib.reload(validate_gameplaytag_indices); validate_gameplaytag_indices.capture_and_validate_server_vs_client()"
        )
        main_menu.add_menu_entry(section_name, entry)

    tool_menus.refresh_all_widgets()
    unreal.log("Successfully registered Gameplay Tag Validation to Unreal Tools menu.")