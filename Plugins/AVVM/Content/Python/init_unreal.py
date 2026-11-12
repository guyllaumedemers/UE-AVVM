import unreal
import audit_module_dependencies
import convert_registries
import convert_datatables
import validate_asset_versions
import validate_texture_audit

unreal.log("Initializing Module Dependencies Auditing Tools...")
audit_module_dependencies.register_menus()

unreal.log("Initializing Data Registry Conversion Tools...")
convert_registries.register_menus()

unreal.log("Initializing Data Table Conversion Tools...")
convert_datatables.register_menus()

unreal.log("Initializing Asset Validation Tools...")
validate_asset_versions.register_menus()

unreal.log("Initializing Texture Validation Tools...")
validate_texture_audit.register_menus()