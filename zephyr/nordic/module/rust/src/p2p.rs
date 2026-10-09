#[path = "p2p/noise.rs"]
mod noise;

use alloc::boxed::Box;
use alloc::vec::Vec;
use core::ffi::c_int;

use crate::error::StcpError;

/*
 * Native STCP_P2P protocol core.
 *
 * IMPORTANT: these names/framing are wire compatibility points with the
 * rust-libp2p golden reference.  Do not replace them with STCP-specific
 * shortcuts.  Noise and Yamux remain independent P2P layers above STCP.
 */
pub const MULTISTREAM_ID: &[u8] = b"/multistream/1.0.0\n";
pub const NOISE_ID: &[u8] = b"/noise\n";
pub const YAMUX_ID: &[u8] = b"/yamux/1.0.0\n";
pub const PING_ID: &[u8] = b"/ipfs/ping/1.0.0\n";

const EINVAL: c_int = -22;
const ENOSYS: c_int = -38;
const EPROTO: c_int = -71;

#[repr(C)]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum P2pStage {
    Init = 0,
    TransportReady = 1,
    SecurityMultistream = 2,
    NoiseHandshake = 3,
    MuxerMultistream = 4,
    YamuxReady = 5,
    PingMultistream = 6,
    Ready = 7,
    Failed = 255,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum DecodeStatus {
    NeedMore,
    Frame(usize, usize), /* prefix bytes, payload bytes */
}

/* unsigned-varint as used by multistream-select length prefixes */
pub fn encode_uvarint(mut value: usize, out: &mut [u8]) -> Result<usize, StcpError> {
    let mut i = 0usize;
    loop {
        if i >= out.len() {
            return Err(StcpError::NoMem);
        }
        let mut byte = (value & 0x7f) as u8;
        value >>= 7;
        if value != 0 {
            byte |= 0x80;
        }
        out[i] = byte;
        i += 1;
        if value == 0 {
            return Ok(i);
        }
    }
}

pub fn decode_uvarint(input: &[u8]) -> Result<Option<(usize, usize)>, StcpError> {
    let mut value = 0usize;
    let mut shift = 0usize;

    for (i, byte) in input.iter().copied().enumerate() {
        if shift >= usize::BITS as usize || i >= 10 {
            return Err(StcpError::Protocol);
        }
        value |= ((byte & 0x7f) as usize) << shift;
        if byte & 0x80 == 0 {
            return Ok(Some((value, i + 1)));
        }
        shift += 7;
    }

    Ok(None)
}

pub fn encode_multistream_frame(payload: &[u8], out: &mut [u8]) -> Result<usize, StcpError> {
    let mut prefix = [0u8; 10];
    let pn = encode_uvarint(payload.len(), &mut prefix)?;
    let total = pn.checked_add(payload.len()).ok_or(StcpError::NoMem)?;
    if out.len() < total {
        return Err(StcpError::NoMem);
    }
    out[..pn].copy_from_slice(&prefix[..pn]);
    out[pn..total].copy_from_slice(payload);
    Ok(total)
}

pub fn decode_multistream_frame(input: &[u8]) -> Result<DecodeStatus, StcpError> {
    let Some((len, prefix_len)) = decode_uvarint(input)? else {
        return Ok(DecodeStatus::NeedMore);
    };
    let total = prefix_len.checked_add(len).ok_or(StcpError::Protocol)?;
    if input.len() < total {
        return Ok(DecodeStatus::NeedMore);
    }
    Ok(DecodeStatus::Frame(prefix_len, len))
}

#[derive(Debug)]
pub struct P2pContext {
    stage: P2pStage,
    rx: Vec<u8>,
    tx: Vec<u8>,
}

impl P2pContext {
    pub fn new() -> Self {
        Self { stage: P2pStage::Init, rx: Vec::new(), tx: Vec::new() }
    }

    pub fn stage(&self) -> P2pStage { self.stage }

    pub fn transport_ready(&mut self) -> Result<(), StcpError> {
        if self.stage != P2pStage::Init {
            return Err(StcpError::InvalidState);
        }
        self.stage = P2pStage::TransportReady;
        self.queue_multistream(MULTISTREAM_ID)?;
        self.stage = P2pStage::SecurityMultistream;
        Ok(())
    }

    fn queue_multistream(&mut self, payload: &[u8]) -> Result<(), StcpError> {
        let mut frame = [0u8; 128];
        let n = encode_multistream_frame(payload, &mut frame)?;
        self.tx.try_reserve(n).map_err(|_| StcpError::NoMem)?;
        self.tx.extend_from_slice(&frame[..n]);
        Ok(())
    }

    pub fn feed(&mut self, data: &[u8]) -> Result<(), StcpError> {
        self.rx.try_reserve(data.len()).map_err(|_| StcpError::NoMem)?;
        self.rx.extend_from_slice(data);
        self.process()
    }

    fn process(&mut self) -> Result<(), StcpError> {
        loop {
            let DecodeStatus::Frame(prefix, len) = decode_multistream_frame(&self.rx)? else {
                return Ok(());
            };
            let end = prefix + len;
            let payload = &self.rx[prefix..end];

            match self.stage {
                P2pStage::SecurityMultistream => {
                    if payload == MULTISTREAM_ID {
                        /* Peer hello: consume it and wait for /noise. */
                    } else if payload == NOISE_ID {
                        self.stage = P2pStage::NoiseHandshake;
                    } else {
                        self.stage = P2pStage::Failed;
                        return Err(StcpError::Protocol);
                    }
                }
                P2pStage::NoiseHandshake => {
                    /*
                     * Noise XX handshake bytes are binary and length framed by
                     * the libp2p Noise transport, not multistream-select.  The
                     * actual Noise state machine lands here in the next step.
                     */
                    return Ok(());
                }
                _ => return Ok(()),
            }

            self.rx.drain(..end);
        }
    }

    pub fn drain_tx(&mut self, out: &mut [u8]) -> usize {
        let n = core::cmp::min(out.len(), self.tx.len());
        out[..n].copy_from_slice(&self.tx[..n]);
        self.tx.drain(..n);
        n
    }
}

pub fn selftest() -> Result<(), StcpError> {
    noise::selftest()?;
    let cases: &[(usize, &[u8])] = &[
        (0, &[0x00]),
        (1, &[0x01]),
        (127, &[0x7f]),
        (128, &[0x80, 0x01]),
        (300, &[0xac, 0x02]),
    ];
    for (value, expected) in cases {
        let mut buf = [0u8; 10];
        let n = encode_uvarint(*value, &mut buf)?;
        if &buf[..n] != *expected { return Err(StcpError::Protocol); }
        let Some((decoded, used)) = decode_uvarint(&buf[..n])? else {
            return Err(StcpError::Protocol);
        };
        if decoded != *value || used != n { return Err(StcpError::Protocol); }
    }

    for proto in [MULTISTREAM_ID, NOISE_ID, YAMUX_ID, PING_ID] {
        let mut frame = [0u8; 128];
        let n = encode_multistream_frame(proto, &mut frame)?;
        match decode_multistream_frame(&frame[..n])? {
            DecodeStatus::Frame(prefix, len) => {
                if &frame[prefix..prefix + len] != proto { return Err(StcpError::Protocol); }
            }
            DecodeStatus::NeedMore => return Err(StcpError::Protocol),
        }
    }

    let mut ctx = P2pContext::new();
    ctx.transport_ready()?;
    if ctx.stage() != P2pStage::SecurityMultistream { return Err(StcpError::Protocol); }
    let mut tx = [0u8; 64];
    let n = ctx.drain_tx(&mut tx);
    match decode_multistream_frame(&tx[..n])? {
        DecodeStatus::Frame(prefix, len) if &tx[prefix..prefix + len] == MULTISTREAM_ID => {}
        _ => return Err(StcpError::Protocol),
    }
    Ok(())
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_p2p_core_abi_version() -> u32 { 2 }

#[unsafe(no_mangle)]
pub extern "C" fn stcp_p2p_core_selftest() -> c_int {
    match selftest() { Ok(()) => 0, Err(e) => e.errno() }
}

/*
 * 0 until exact libp2p Noise XX + Yamux + ping stream state machines exist.
 * The application must not advertise native compatibility before that point.
 */
#[unsafe(no_mangle)]
pub extern "C" fn stcp_p2p_core_ready() -> c_int { 0 }

/* Phase-2 capability bits. Noise primitives/framing/identity verification are present,
 * but full transport readiness remains false until Yamux and ping are complete. */
#[unsafe(no_mangle)]
pub extern "C" fn stcp_p2p_noise_core_ready() -> c_int { 1 }

#[unsafe(no_mangle)]
pub extern "C" fn stcp_p2p_noise_selftest() -> c_int {
    match noise::selftest() { Ok(()) => 0, Err(e) => e.errno() }
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_p2p_noise_dialer_new(identity_seed: *const u8, out_ctx: *mut *mut core::ffi::c_void) -> c_int {
    if identity_seed.is_null() || out_ctx.is_null() { return EINVAL; }
    let mut seed=[0u8;32];
    unsafe { core::ptr::copy_nonoverlapping(identity_seed,seed.as_mut_ptr(),32); }
    match noise::NoiseInitiator::from_identity_seed(seed) {
        Ok(ctx) => { unsafe { *out_ctx=Box::into_raw(Box::new(ctx)).cast(); } 0 }
        Err(e) => e.errno(),
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_p2p_noise_dialer_free(ctx: *mut core::ffi::c_void) {
    if !ctx.is_null() { unsafe { drop(Box::from_raw(ctx.cast::<noise::NoiseInitiator>())); } }
}

fn copy_out(bytes:&[u8], out:*mut u8, cap:usize)->c_int {
    if out.is_null() || cap < bytes.len() { return -12; }
    unsafe { core::ptr::copy_nonoverlapping(bytes.as_ptr(),out,bytes.len()); }
    bytes.len() as c_int
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_p2p_noise_dialer_message1(ctx:*mut core::ffi::c_void,out:*mut u8,cap:usize)->c_int {
    if ctx.is_null(){return EINVAL;}
    let c=unsafe{&mut *ctx.cast::<noise::NoiseInitiator>()};
    match c.write_message1(){Ok(v)=>copy_out(&v,out,cap),Err(e)=>e.errno()}
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_p2p_noise_dialer_message2(ctx:*mut core::ffi::c_void,msg2:*const u8,msg2_len:usize,out3:*mut u8,cap:usize)->c_int {
    if ctx.is_null() || msg2.is_null(){return EINVAL;}
    let c=unsafe{&mut *ctx.cast::<noise::NoiseInitiator>()};
    let input=unsafe{core::slice::from_raw_parts(msg2,msg2_len)};
    match c.read_message2_write_message3(input){Ok(v)=>copy_out(&v,out3,cap),Err(e)=>e.errno()}
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_p2p_noise_dialer_complete(ctx:*mut core::ffi::c_void)->c_int {
    if ctx.is_null(){return 0;} let c=unsafe{&*ctx.cast::<noise::NoiseInitiator>()}; if c.complete(){1}else{0}
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_p2p_core_stage_name(stage: u32) -> *const u8 {
    match stage {
        0 => b"init\0".as_ptr(),
        1 => b"transport-ready\0".as_ptr(),
        2 => b"security-multistream\0".as_ptr(),
        3 => b"noise-handshake\0".as_ptr(),
        4 => b"muxer-multistream\0".as_ptr(),
        5 => b"yamux-ready\0".as_ptr(),
        6 => b"ping-multistream\0".as_ptr(),
        7 => b"ready\0".as_ptr(),
        _ => b"failed\0".as_ptr(),
    }
}
