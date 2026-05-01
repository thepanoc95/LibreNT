use windows_sys::Win32::System::Environment::{SetEnvironmentVariableW, GetEnvironmentVariableW};
use crate::logger;

pub fn set_env_var(key: &str, value: &str) -> Result<(), String> {
    let key_wide: Vec<u16> = key.encode_utf16().chain(Some(0)).collect();
    let value_wide: Vec<u16> = value.encode_utf16().chain(Some(0)).collect();
    
    let result = unsafe {
        SetEnvironmentVariableW(key_wide.as_ptr(), value_wide.as_ptr())
    };
    
    if result != 0 {
        logger::info(&format!("Set environment variable: {}={}", key, value));
        Ok(())
    } else {
        Err(format!("Failed to set environment variable: {}", key))
    }
}

pub fn get_env_var(key: &str) -> Option<String> {
    let key_wide: Vec<u16> = key.encode_utf16().chain(Some(0)).collect();
    
    let size = unsafe {
        GetEnvironmentVariableW(key_wide.as_ptr(), std::ptr::null_mut(), 0)
    };
    
    if size == 0 {
        return None;
    }
    
    let mut buffer: Vec<u16> = vec![0; size as usize + 1];
    
    let result = unsafe {
        GetEnvironmentVariableW(key_wide.as_ptr(), buffer.as_mut_ptr(), size)
    };
    
    if result == 0 {
        return None;
    }
    
    // Remove the null terminator
    buffer.pop();
    
    String::from_utf16(&buffer).ok()
}