pub mod proto {
    include!(concat!(env!("OUT_DIR"), "/data.rs"));
}

fn main() {
    println!("Rust agent ready");
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
