use core::ptr::null_mut;

use iowrapper::stream::StcpStream;
use stcptypes::types::ServerMessageProcessCB;
use zephyrmisc::zephyr_thread::spawn_handler;
use spin::Mutex;

pub fn spawn(stream: Mutex<StcpStream>, cb: ServerMessageProcessCB) {
	spawn_handler(stream, cb);
}
