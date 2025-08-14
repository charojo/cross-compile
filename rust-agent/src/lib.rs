use prost::Message;
use std::sync::{
    Arc,
    atomic::{AtomicU64, Ordering},
};
use std::time::{Duration, SystemTime, UNIX_EPOCH};

pub mod proto {
    include!(concat!(env!("OUT_DIR"), "/data.rs"));
}

use proto::{ControlCommand, SensorReading};

pub fn run() {
    let context = zmq::Context::new();
    let publisher = context.socket(zmq::PUB).expect("create pub socket");
    publisher.bind("tcp://*:5555").expect("bind pub socket");

    let subscriber = context.socket(zmq::SUB).expect("create sub socket");
    subscriber
        .connect("tcp://localhost:5556")
        .expect("connect sub socket");
    subscriber.set_subscribe(b"").expect("subscribe");

    let rate = Arc::new(AtomicU64::new(1));
    let rate_for_sub = rate.clone();

    std::thread::spawn(move || {
        loop {
            if let Ok(bytes) = subscriber.recv_bytes(0) {
                if let Ok(cmd) = ControlCommand::decode(&*bytes) {
                    rate_for_sub.store(cmd.new_rate, Ordering::Relaxed);
                }
            }
        }
    });

    loop {
        let ts = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .expect("time went backwards")
            .as_secs() as i64;
        let reading = SensorReading {
            sensor_id: 1,
            value: 0.0,
            timestamp: ts,
        };
        let mut buf = Vec::new();
        reading.encode(&mut buf).expect("encode");
        publisher.send(buf, 0).expect("send");
        let delay = rate.load(Ordering::Relaxed);
        std::thread::sleep(Duration::from_secs(delay));
    }
}

#[cfg(test)]
mod tests {
    use super::proto::{ControlCommand, SensorReading};
    use prost::Message;
    use std::time::{Duration, Instant};

    #[test]
    fn sensor_reading_roundtrip() {
        let ctx = zmq::Context::new();
        let pub_socket = ctx.socket(zmq::PUB).unwrap();
        let sub_socket = ctx.socket(zmq::SUB).unwrap();
        let endpoint = "inproc://sensor";
        pub_socket.bind(endpoint).unwrap();
        sub_socket.connect(endpoint).unwrap();
        sub_socket.set_subscribe(b"").unwrap();

        let reading = SensorReading {
            sensor_id: 7,
            value: 1.23,
            timestamp: 99,
        };
        let mut buf = Vec::new();
        reading.encode(&mut buf).unwrap();

        let mut items = [pub_socket.as_poll_item(zmq::POLLOUT)];
        let start = Instant::now();
        while !items[0].is_writable() {
            zmq::poll(&mut items, 10).unwrap();
            if start.elapsed() > Duration::from_secs(1) {
                panic!("subscriber handshake timed out");
            }
        }
        pub_socket.send(buf, 0).unwrap();

        let bytes = sub_socket.recv_bytes(0).unwrap();
        let decoded = SensorReading::decode(&*bytes).unwrap();
        assert_eq!(reading, decoded);
    }

    #[test]
    fn control_command_roundtrip() {
        let ctx = zmq::Context::new();
        let pub_socket = ctx.socket(zmq::PUB).unwrap();
        let sub_socket = ctx.socket(zmq::SUB).unwrap();
        let endpoint = "inproc://cmd";
        pub_socket.bind(endpoint).unwrap();
        sub_socket.connect(endpoint).unwrap();
        sub_socket.set_subscribe(b"").unwrap();

        let cmd = ControlCommand {
            new_rate: 5,
            target: String::new(),
            command: String::new(),
        };
        let mut buf = Vec::new();
        cmd.encode(&mut buf).unwrap();

        let mut items = [pub_socket.as_poll_item(zmq::POLLOUT)];
        let start = Instant::now();
        while !items[0].is_writable() {
            zmq::poll(&mut items, 10).unwrap();
            if start.elapsed() > Duration::from_secs(1) {
                panic!("subscriber handshake timed out");
            }
        }
        pub_socket.send(buf, 0).unwrap();

        let bytes = sub_socket.recv_bytes(0).unwrap();
        let decoded = ControlCommand::decode(&*bytes).unwrap();
        assert_eq!(cmd, decoded);
    }
}
