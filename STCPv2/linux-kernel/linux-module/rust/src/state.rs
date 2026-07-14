use alloc::{boxed::Box, collections::VecDeque};

pub struct StcpContext {
    pub proto: u8,
    pub accept_queue: VecDeque<Box<StcpContext>>,
}

impl StcpContext {
    pub fn new(proto: u8) -> Self {
        Self { proto, accept_queue: VecDeque::new() }
    }
}
