import unreal
import audit_module_dependencies
import convert_registries
import validate_asset_versions

unreal.log("Initializing Module Dependencies Auditing Tools...")
audit_module_dependencies.register_menus()

unreal.log("Initializing Data Registry Conversion Tools...")
convert_registries.register_menus()

unreal.log("Initializing Asset Validation Tools...")
validate_asset_versions.register_menus()