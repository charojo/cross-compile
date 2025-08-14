use chrono::Utc;
use log::info;
use std::env;

fn init_logger(trace_id: String) {
    env_logger::Builder::from_env(env_logger::Env::default().default_filter_or("info"))
        .format(move |buf, record| {
            use std::io::Write;
            writeln!(
                buf,
                "{} {} [{}] [trace={}] {}",
                Utc::now().to_rfc3339(),
                record.level(),
                record.target(),
                trace_id,
                record.args()
            )
        })
        .init();
}

fn main() {
    let args: Vec<String> = env::args().collect();
    let mut trace_id = String::from("0");
    for i in 0..args.len() {
        if args[i] == "--trace-id" && i + 1 < args.len() {
            trace_id = args[i + 1].clone();
        }
    }
    init_logger(trace_id);
    info!(target: "RS1001", "Rust agent ready");

    rust_agent::run();
    println!("Rust agent ready");
}

pub mod proto {
    include!(concat!(env!("OUT_DIR"), "/data.rs"));
}

#[cfg(test)]
mod tests {
    use super::proto::GetUserRequest;

    #[test]
    fn it_runs() {
        let _req = GetUserRequest { user_id: 1 };
        assert_eq!(2 + 2, 4);
    }
}
