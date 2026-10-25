import unreal
import convert_registries
import validate_asset_versions

unreal.log("Initializing Data Registry Conversion Tools...")
convert_registries.register_menus()

unreal.log("Initializing Asset Validation Tools...")
validate_asset_versions.register_menus()