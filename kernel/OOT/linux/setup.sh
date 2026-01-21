
sudo apt update
sudo apt install -y dkms build-essential kmod rustup linux-headers-$(uname -r)

sudo rustup toolchain install nightly
sudo rustup default nightly
sudo rustup component add rust-src --toolchain nightly-x86_64-unknown-linux-gnu

sudo cargo +nightly --version
sudo rustc +nightly --version
