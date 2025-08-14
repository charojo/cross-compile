use prost::Message;
use std::time::{SystemTime, UNIX_EPOCH};

mod proto {
    include!(concat!(env!("OUT_DIR"), "/data.rs"));
}

fn main() {
    // Set up a ZeroMQ publisher.
    let ctx = zmq::Context::new();
    let publisher = ctx.socket(zmq::PUB).expect("create pub socket");
    publisher.bind("tcp://*:5555").expect("bind pub socket");

    // Construct a mock sensor reading.
    let ts = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .expect("time went backwards")
        .as_secs() as i64;
    let reading = proto::SensorReading {
        sensor_id: 1,
        value: 42.0,
        timestamp: ts,
    };

    // Encode and publish the message.
    let mut buf = Vec::new();
    reading.encode(&mut buf).expect("encode message");
    publisher.send(buf, 0).expect("send message");

    println!("Published mock SensorReading");
}

#[cfg(test)]
mod tests {
    use super::proto::SensorReading;
    use prost::Message;

    #[test]
    fn encodes_sensor_reading() {
        let reading = SensorReading {
            sensor_id: 1,
            value: 0.0,
            timestamp: 0,
        };
        let mut buf = Vec::new();
        reading.encode(&mut buf).unwrap();
        assert!(!buf.is_empty());
    }
}
