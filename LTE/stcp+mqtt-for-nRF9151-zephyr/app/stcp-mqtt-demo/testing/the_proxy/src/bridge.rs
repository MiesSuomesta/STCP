use std::cmp::min;
use std::ffi::c_void;
use std::io::{Read, Write};
use std::net::TcpStream;
use std::thread;
use std::time::Duration;

use crate::debug::stcp_uptime_ms;
use crate::debug::stcp_dump_hex;
use crate::stcp_dbg;
use crate::stcp_dbg_nln;

use crate::transport::{
    recv_from_transport,
    send_to_transport,
};

use the_stcp_kernel_module::{
    proto_session::ProtoSession,
    stcp_message::stcp_message_unpack_frame_from,
};

pub fn bridge_loop(
    session: &mut ProtoSession,
    transport: *mut c_void,
    mqtt_sock: &mut TcpStream,
) {

    //
    // Raw encrypted STCP frame
    //
    let mut stcp_buf = [0u8; 4096];

    //
    // Plain decrypted MQTT payload
    //
    let mut plain_buf = [0u8; 4096];

    //
    // MQTT -> STCP buffer
    //
    let mut mqtt_buf = [0u8; 4096];

    stcp_dbg!(
        "[BRIDGE] Starting handler loop...."
    );

    loop {

        //
        // =====================================================
        // STCP -> MQTT
        // =====================================================
        //

        let mut recv_len: usize = 0;

        let rc =
            unsafe {
                crate::transport::stcp_tcp_recv(
                    transport,

                    stcp_buf.as_mut_ptr(),

                    stcp_buf.len(),

                    1, // nonblocking

                    0,

                    &mut recv_len,
                )
            };

        if rc > 0 {

            let frame_len =
                rc as usize;

            stcp_dbg!(
                "[BRIDGE] STCP -> MQTT {} bytes",
                frame_len
            );

            stcp_dbg_nln!("[BRIDGE] ");
            stcp_dump_hex("S->M", &stcp_buf[..min(frame_len, 64)]);

            //
            // Remove STCP frame header
            //
            let (_hdr, encrypted_payload) =
                stcp_message_unpack_frame_from(
                    &stcp_buf[..frame_len]
                );

            //
            // Decrypt payload
            //
            match recv_from_transport(
                session,
                &encrypted_payload,
                &mut plain_buf,
            ) {

                Ok(plain_len) => {

                    stcp_dbg!(
                        "[BRIDGE] DECRYPTED {} bytes",
                        plain_len
                    );

                    stcp_dbg_nln!("[BRIDGE] ");
                    stcp_dump_hex("M->S", &plain_buf[..min(plain_len, 64)]);

                    let _ =
                        mqtt_sock.write_all(
                            &plain_buf[..plain_len]
                        );
                }

                Err(rc) => {

                    stcp_dbg!(
                        "[BRIDGE] recvmsg failed rc={}",
                        rc
                    );
                }
            }
        }

        //
        // =====================================================
        // MQTT -> STCP
        // =====================================================
        //

        match mqtt_sock.read(&mut mqtt_buf) {

            Ok(len) => {

                if len > 0 {

                    stcp_dbg!(
                        "[BRIDGE] MQTT -> STCP {} bytes",
                        len
                    );

                    stcp_dbg!(
                        "[BRIDGE] M -> S {:02X?}",
                        &mqtt_buf[..min(len, 64)]
                    );

                    match send_to_transport(
                        session,
                        transport,
                        &mqtt_buf[..len],
                    ) {

                        Ok(sent) => {

                            stcp_dbg!(
                                "[BRIDGE] Sent {} bytes",
                                sent
                            );
                        }

                        Err(rc) => {

                            stcp_dbg!(
                                "[BRIDGE] sendmsg failed rc={}",
                                rc
                            );
                        }
                    }
                }
            }

            Err(_) => {}
        }

        thread::sleep(
            Duration::from_millis(5)
        );
    }
}