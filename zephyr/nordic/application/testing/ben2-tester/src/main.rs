mod transport;
use std::{net::Ipv4Addr,process::ExitCode};
fn usage(){eprintln!("ben2-tester --host IPv4 [--total 1048576] [--chunk 4096] [--timeout-ms 60000] [--rounds 1]\n  [--tcp-port 19002] [--udp-port 19003] [--stcp-tcp-port 19000] [--stcp-udp-port 19001]\n  [--transport all|tcp|udp|stcp-tcp|stcp-udp]\nFULL reports aggregate UL+DL bit/sec. Linux STCP tests require stcp.ko.");}
fn run()->Result<bool,String> {
    let mut host:Ipv4Addr="127.0.0.1".parse().unwrap();let mut total=1048576u32;let mut chunk=4096u32;
    let mut timeout=60000u64;let mut rounds=1u32;let mut only="all".to_string();
    let mut ports=[19002u16,19003,19000,19001];let protocols=["tcp","udp","stcp-tcp","stcp-udp"];
    let mut args=std::env::args().skip(1);
    while let Some(key)=args.next() {
        if key=="--help" {usage();return Ok(true);}
        let value=args.next().ok_or_else(||format!("missing value for {key}"))?;
        match key.as_str() {
            "--host"=>host=value.parse().map_err(|_|"invalid IPv4 host")?,
            "--total"=>total=value.parse().map_err(|_|"invalid total")?,
            "--chunk"=>chunk=value.parse().map_err(|_|"invalid chunk")?,
            "--timeout-ms"=>timeout=value.parse().map_err(|_|"invalid timeout")?,
            "--rounds"=>rounds=value.parse().map_err(|_|"invalid rounds")?,
            "--transport"=>only=value,
            "--tcp-port"=>ports[0]=value.parse().map_err(|_|"invalid port")?,
            "--udp-port"=>ports[1]=value.parse().map_err(|_|"invalid port")?,
            "--stcp-tcp-port"=>ports[2]=value.parse().map_err(|_|"invalid port")?,
            "--stcp-udp-port"=>ports[3]=value.parse().map_err(|_|"invalid port")?,
            _=>return Err(format!("unknown option {key}")),
        }
    }
    if total==0 || total>256*1024*1024 || chunk==0 || chunk>1024*1024 || timeout==0 || timeout>86400000 || rounds==0 || rounds>10000 || ports.contains(&0) || (only!="all" && !protocols.contains(&only.as_str())) {return Err("invalid test configuration".into());}
    if !cfg!(target_os="linux") {return Err("tester requires Linux".into());}
    let mut passed=true;
    for round in 1..=rounds {
        if rounds>1 {println!("Round {round}/{rounds}");}
        for(i,proto)in protocols.iter().enumerate() {
            if only!="all" && only!=*proto {continue;}
            let results:Vec<_>=(1..=3).map(|mode|transport::test(host,ports[i],proto,mode,total,chunk,timeout)).collect();
            let success=results.iter().all(Result::is_ok);passed&=success;
            println!("\n* {:<12} [{}]",proto.to_uppercase(),if success{"PASS"}else{"FAIL"});
            for(label,result)in ["UL","DL","FULL"].iter().zip(results) {
                match result {Ok(rate)=>println!("  -> {label:<4} {rate:.0} bit/sec"),Err(e)=>println!("  -> {label:<4} n/a bit/sec [FAIL: {e}]")}
            }
        }
    }
    Ok(passed)
}
fn main()->ExitCode {match run(){Ok(true)=>ExitCode::SUCCESS,Ok(false)=>ExitCode::from(1),Err(e)=>{eprintln!("ERROR: {e}");usage();ExitCode::from(2)}}}
