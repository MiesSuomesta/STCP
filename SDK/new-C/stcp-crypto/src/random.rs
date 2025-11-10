use rand_core::{RngCore, CryptoRng, Error};

pub struct ZephyrRng;
impl CryptoRng for ZephyrRng {}

extern "C" {
    pub fn zephyr_rand_byte() -> u8;
}

impl RngCore for ZephyrRng {
    fn next_u32(&mut self) -> u32 {
        let mut val: u32 = 0;
        unsafe {
            let ptr = &mut val as *mut u32 as *mut u8;
            for i in 0..4 {
                *ptr.add(i) = unsafe { zephyr_rand_byte() };  // korvaa Zephyr-funktiolla
            }
        }
        val
    }

    fn next_u64(&mut self) -> u64 {
        let mut val: u64 = 0;
        unsafe {
            let ptr = &mut val as *mut u64 as *mut u8;
            for i in 0..8 {
                *ptr.add(i) = unsafe { zephyr_rand_byte() };
            }
        }
        val
    }

    fn fill_bytes(&mut self, dest: &mut [u8]) {
        for b in dest.iter_mut() {
            *b = unsafe { zephyr_rand_byte() };
        }
    }

    fn try_fill_bytes(&mut self, dest: &mut [u8]) -> Result<(), Error> {
        self.fill_bytes(dest);
        Ok(())
    }
}

pub fn fill_random_bytes(buf: &mut [u8]) {
    for (i, b) in buf.iter_mut().enumerate() {
        *b = (i as u8).wrapping_mul(73) ^ 0xA5;
    }
}


#[no_mangle]
pub extern "C" fn __getrandom_custom(buf: *mut u8, len: usize) -> i32 {
    unsafe {
        for i in 0..len {
            *buf.add(i) = unsafe { zephyr_rand_byte() }; // korvaa oikealla Zephyr-funktiolla
        }
    }
    0
}
