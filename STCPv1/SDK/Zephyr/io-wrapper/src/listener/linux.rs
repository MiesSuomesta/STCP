#![cfg_attr(not(feature = "std"), no_std)]
use std::net::{TcpListener, TcpStream};
use std::io::{Read, Write};
use crate::stream::StcpStream;
use std::os::fd::AsRawFd;

pub struct Incoming<'a> {
	listener: &'a StcpListener
}

impl<'a> Iterator for Incoming<'a> {
	type Item = Result<StcpStream, ()>;
	fn next(&mut self) -> Option<Self::Item> {
		Some(self.listener.accept())	
	}
}

pub struct StcpListener {
     listener: TcpListener,
}

impl StcpListener {

    pub fn bind(addr: &str) -> Result<Self, ()> {
        TcpListener::bind(addr)
            .map(|listener| Self { listener })
            .map_err(|_| ())
    }

    pub fn accept(&self) -> Result<StcpStream, ()> {
        self.listener
            .accept()
            .map(|(stream, _)| {
                let fd = stream.as_raw_fd();
                StcpStream { fd }
            })
            .map_err(|_| ())
    }

    pub fn incoming(&self) -> Incoming {
	Incoming { listener: self }
    }

}
