fn main() {
    let proto_file = "../proto/data.proto";
    println!("cargo:rerun-if-changed={}", proto_file);
    prost_build::compile_protos(&[proto_file], &["../proto"]).expect("Failed to compile protos");
}
