use std::sync::{
    Arc,
    atomic::{AtomicBool, AtomicI32, AtomicU64, Ordering},
};
use std::thread;
use std::time::{Duration, SystemTime, UNIX_EPOCH};

use prost::Message;

mod proto {
    include!(concat!(env!("OUT_DIR"), "/data.rs"));
}

use proto::{ControlCommand, SensorReading};

fn main() -> Result<(), Box<dyn std::error::Error>> {
    env_logger::init();

    // Set up ZeroMQ publisher and subscriber.
    let ctx = zmq::Context::new();
    let publisher = ctx.socket(zmq::PUB)?;
    publisher.bind("tcp://*:5555")?;

    let subscriber = ctx.socket(zmq::SUB)?;
    subscriber.connect("tcp://localhost:5556")?;
    subscriber.set_subscribe(b"")?;

    // Shared state for publish rate, target sensor, and shutdown signal.
    let rate = Arc::new(AtomicU64::new(1));
    let sensor_id = Arc::new(AtomicI32::new(1));
    let running = Arc::new(AtomicBool::new(true));

    // CTRL-C handler for clean shutdown.
    {
        let running = running.clone();
        ctrlc::set_handler(move || {
            running.store(false, Ordering::SeqCst);
        })?;
    }

    // Thread to receive control commands.
    let sub_handle = {
        let rate = rate.clone();
        let sensor_id = sensor_id.clone();
        let running = running.clone();
        thread::spawn(move || {
            while running.load(Ordering::SeqCst) {
                match subscriber.recv_bytes(zmq::DONTWAIT) {
                    Ok(bytes) => match ControlCommand::decode(&*bytes) {
                        Ok(cmd) => {
                            if cmd.new_rate > 0 {
                                rate.store(cmd.new_rate, Ordering::SeqCst);
                                log::info!("Updated rate to {}s", cmd.new_rate);
                            }
                            if !cmd.target.is_empty() {
                                if let Ok(id) = cmd.target.parse() {
                                    sensor_id.store(id, Ordering::SeqCst);
                                    log::info!("Updated sensor id to {}", id);
                                }
                            }
                            if cmd.command == "shutdown" {
                                running.store(false, Ordering::SeqCst);
                            }
                        }
                        Err(e) => log::error!("Decode ControlCommand: {e}"),
                    },
                    Err(e) => {
                        if e == zmq::Error::EAGAIN {
                            thread::sleep(Duration::from_millis(100));
                        } else {
                            log::error!("Receive ControlCommand: {e}");
                            break;
                        }
                    }
                }
            }
            if let Err(e) = subscriber.close() {
                log::error!("Close subscriber: {e}");
            }
        })
    };

    // Publishing loop.
    while running.load(Ordering::SeqCst) {
        let ts = SystemTime::now().duration_since(UNIX_EPOCH)?.as_secs() as i64;

        let reading = SensorReading {
            sensor_id: sensor_id.load(Ordering::SeqCst),
            value: 42.0,
            timestamp: ts,
        };

        let mut buf = Vec::new();
        if let Err(e) = reading.encode(&mut buf) {
            log::error!("Encode SensorReading: {e}");
            continue;
        }
        if let Err(e) = publisher.send(buf, 0) {
            log::error!("Publish SensorReading: {e}");
        }

        let delay = rate.load(Ordering::SeqCst);
        thread::sleep(Duration::from_secs(delay));
    }

    log::info!("Shutting down");
    if let Err(e) = publisher.close() {
        log::error!("Close publisher: {e}");
    }
    if let Err(e) = sub_handle.join() {
        log::error!("Join control thread: {e:?}");
    }
    Ok(())
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
