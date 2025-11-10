#![cfg_attr(not(feature = "std"), no_std)]

extern crate alloc;
use alloc::vec::Vec;
use alloc::boxed::Box;
use alloc::format;

use core::ptr;
use core::net::{IpAddr, SocketAddr};
use core::error;

use stcptypes::types::*;
use stcpthreads::threads::spawn;

use iowrapper::types::StcpServer;
use iowrapper::listener::StcpListener;
use iowrapper::stream::StcpStream;

use debug::zprint;
use debug::dbg;
use utils;

use spin::Mutex;

pub fn stcp_internal_server_bind(ip: &str, port: u16, cb: ServerMessageProcessCB) -> Result<StcpServer, Box<dyn error::Error>> {
    let addr_str = format!("{}:{}", ip, port);

    let listener = match StcpListener::bind(&addr_str) {
		Ok(val) => val,
		Err(e) => {
			let es = format!("Bind error: {:?}", e);
			dbg!("{:?}", es);
			return Err(es.into());
		}
	};

    Ok(StcpServer { listener, port, callback: cb })
}

pub fn stcp_internal_server_listen(server: &mut StcpServer) -> Result<(), Box<dyn error::Error>> {
    let listener = &server.listener;
    let callback = server.callback;

    for stream in listener.incoming() {
        match stream {
            Ok(stream) => {
                let cb = callback.clone();
		let lockable = Mutex::new(stream);
                spawn_client_handler(lockable, cb);
            }
            Err(e) => {
                dbg!("Client handle error: {:?}", e);
            }
        }
    }

    Ok(())
}

pub fn stcp_internal_server_stop(server: &mut StcpServer) {

    unsafe {
        drop(Box::from_raw(server));
    }
}

pub fn spawn_client_handler(stream: Mutex<StcpStream>, callback: ServerMessageProcessCB) {
    spawn(stream, callback);

}

