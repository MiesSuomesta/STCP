
sudo apt update
sudo apt install -y dkms build-essential kmod rustup linux-headers-$(uname -r)

sudo rustup toolchain install nightly
sudo rustup default nightly

sudo cargo +nightly --version
sudo rustc +nightly --version
