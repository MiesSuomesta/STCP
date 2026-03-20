use crate::types::{
    StcpEcdhPubKey,
    StcpEcdhSecret,
};

use crate::abi;

use crate::stcp_dbg;
use crate::stcp_dbg_big;
use crate::stcp_info;
use crate::stcp_info_big;
use crate::stcp_err;
use crate::stcp_err_big;
use crate::stcp_dump;

#[unsafe(no_mangle)]
pub extern "C" fn stcp_crypto_selftest() -> i32 {

    stcp_dbg_big!("🧪 STCP cryto self-test start");

    let mut pub1 = StcpEcdhPubKey::new();
    let mut priv1 = StcpEcdhSecret::new();

    let mut pub2 = StcpEcdhPubKey::new();
    let mut priv2 = StcpEcdhSecret::new();

    let mut shared1 = StcpEcdhSecret::new();
    let mut shared2 = StcpEcdhSecret::new();

    let mut step: i32 = 0;

    unsafe {

        if abi::stcp_crypto_generate_keypair(&mut pub1, &mut priv1) != 0 {
            step = 1;
            stcp_err_big!("🧪 SELFTEST step {}: keypair generation failed", step);
        }

        
        if abi::stcp_crypto_generate_keypair(&mut pub2, &mut priv2) != 0 {
            if step == 0 { step = 2; }
            stcp_err_big!("🧪 SELFTEST step {}: keypair generation failed", step);
        }

        if abi::stcp_crypto_compute_shared(&priv1, &pub2, &mut shared1) != 0 {
            if step == 0 { step = 3; }
            stcp_err_big!("🧪 SELFTEST step {}: shared key compute failed", step);
        }

        if abi::stcp_crypto_compute_shared(&priv2, &pub1, &mut shared2) != 0 {
            if step == 0 { step = 4; }
            stcp_err_big!("🧪 SELFTEST step {}: shared key compute failed", step);
        }
    }

    if step == 0 {

        stcp_dump!("Selftest shared1", &shared1.to_bytes_be());
        stcp_dump!("Selftest shared2", &shared2.to_bytes_be());

        if shared1.to_bytes_be() != shared2.to_bytes_be() {
            step = 5;
            stcp_err_big!("🧪 SELFTEST step {}: shared keys mismatch", step);
        }
    }

    if step == 0 {
        stcp_info_big!("🧪 STCP ECDH self-test: ✅ OK");
        0
    } else {
        stcp_err_big!("🧪 STCP ECDH self-test: ❌ FAIL @ step {}", step);
        -step
    }
}

pub fn stcp_do_selftests() -> bool {

    stcp_dbg_big!("Running STCP selftests...");

    let crypto_ok = stcp_crypto_selftest();

    if crypto_ok != 0 {
        stcp_err_big!("STCP selftests FAILED");
        return false;
    }

    stcp_info_big!("STCP selftests passed");
    true
}