use windows_sys::Win32::System::Registry::{RegCreateKeyExW, RegSetValueExW, RegCloseKey, HKEY, HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE, REG_OPTION_NON_VOLATILE, KEY_WRITE, REG_SZ};
use crate::logger;

pub fn set_registry_value(path: &str, value: &str) -> Result<(), String> {
    // Parse the registry path: HKCU\Software\LibreNT\Mode
    let parts: Vec<&str> = path.split('\\').collect();
    if parts.len() < 3 {
        return Err(format!("Invalid registry path: {}", path));
    }
    
    let root = parts[0];
    let subkey = parts[1..].join("\\");
    
    // Determine the root key
    let hkey: HKEY = match root {
        "HKCU" => HKEY_CURRENT_USER,
        "HKLM" => HKEY_LOCAL_MACHINE,
        _ => return Err(format!("Unsupported root key: {}", root)),
    };
    
    // Convert strings to wide
    let subkey_wide: Vec<u16> = subkey.encode_utf16().chain(Some(0)).collect();
    let value_wide: Vec<u16> = value.encode_utf16().chain(Some(0)).collect();
    
    let mut hkey_handle: HKEY = 0;
    let mut disposition: u32 = 0;
    
    // Create/open the key
    let result = unsafe {
        RegCreateKeyExW(
            hkey,
            subkey_wide.as_ptr(),
            0,
            std::ptr::null(),
            REG_OPTION_NON_VOLATILE,
            KEY_WRITE,
            std::ptr::null(),
            &mut hkey_handle,
            &mut disposition,
        )
    };
    
    if result != 0 {
        return Err(format!("Failed to create/open registry key: {}", subkey));
    }
    
    // Set the value (using empty string as the default value name)
    let empty_wide: Vec<u16> = vec![0];
    let data_bytes: Vec<u8> = value_wide.iter().take(value_wide.len() - 1).map(|&c| c as u8).collect();
    
    let result = unsafe {
        RegSetValueExW(
            hkey_handle,
            empty_wide.as_ptr(),
            0,
            REG_SZ,
            data_bytes.as_ptr(),
            data_bytes.len() as u32,
        )
    };
    
    // Close the handle
    unsafe {
        RegCloseKey(hkey_handle);
    }
    
    if result != 0 {
        return Err(format!("Failed to set registry value: {}", value));
    }
    
    logger::info(&format!("Set registry value: {} = {}", subkey, value));
    Ok(())
}