#[cfg(feature = "zephyr")]
#[deprecated(note = "Zephyr build active")]
pub fn zephyr_feature_active() {}

#[cfg(feature = "linux")]
#[deprecated(note = "Linux build active")]
pub fn linux_feature_active() {}
