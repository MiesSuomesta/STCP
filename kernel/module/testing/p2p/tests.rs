#[test]
fn linked_core_runs_full_noise_xx_selftest() {
    assert_eq!(p2p::stcp_p2p_core_abi_version(),2);
    assert_eq!(p2p::stcp_p2p_core_selftest(),0);
    assert_eq!(p2p::stcp_p2p_noise_selftest(),0);
    assert_eq!(p2p::stcp_p2p_noise_core_ready(),1);
    assert_eq!(p2p::stcp_p2p_core_ready(),0, "Yamux/ping must not be advertised as implemented");
}
#[test]
fn malformed_noise_message_is_rejected_over_c_abi() {
    let seed=[0x34u8;32];let mut ctx=core::ptr::null_mut();
    assert_eq!(p2p::stcp_p2p_noise_listener_new(seed.as_ptr(),&mut ctx),0);
    let malformed=[0u8;1];let mut out=[0u8;2048];
    assert!(p2p::stcp_p2p_noise_listener_message1(ctx,malformed.as_ptr(),malformed.len(),out.as_mut_ptr(),out.len())<0);
    assert_eq!(p2p::stcp_p2p_noise_listener_complete(ctx),0);
    p2p::stcp_p2p_noise_listener_free(ctx);
}
#[test]
fn static_c_abi_supports_both_noise_roles() {
    let seed_a=[0x43u8;32];let seed_b=[0x79u8;32];
    let mut a=core::ptr::null_mut();let mut b=core::ptr::null_mut();
    assert_eq!(p2p::stcp_p2p_noise_dialer_new(seed_a.as_ptr(),&mut a),0);
    assert_eq!(p2p::stcp_p2p_noise_listener_new(seed_b.as_ptr(),&mut b),0);
    let mut m1=[0u8;64];let mut m2=[0u8;2048];let mut m3=[0u8;2048];
    let n1=p2p::stcp_p2p_noise_dialer_message1(a,m1.as_mut_ptr(),m1.len());assert!(n1>0);
    let n2=p2p::stcp_p2p_noise_listener_message1(b,m1.as_ptr(),n1 as usize,m2.as_mut_ptr(),m2.len());assert!(n2>0);
    let n3=p2p::stcp_p2p_noise_dialer_message2(a,m2.as_ptr(),n2 as usize,m3.as_mut_ptr(),m3.len());assert!(n3>0);
    assert_eq!(p2p::stcp_p2p_noise_listener_message3(b,m3.as_ptr(),n3 as usize),0);
    assert_eq!(p2p::stcp_p2p_noise_dialer_complete(a),1);
    assert_eq!(p2p::stcp_p2p_noise_listener_complete(b),1);
    p2p::stcp_p2p_noise_dialer_free(a);p2p::stcp_p2p_noise_listener_free(b);
}
