This binary allows for nodeos to run on a Raspberry Pi 4 with Ubunutu 18.04 operating system. It is highly encouraged that the Pi have 8GB of RAM and 64-bit to support reasonable network syncing and resources. Make sure your Pi has more than enough RAM available for your eosio chain. For example, as of May 11, 2021, Telos utilizes 6GB of RAM, so the 8GB Pi will have just enough resources available.

Additionally, the user should consider using the following options in their config.ini file. The first (database-map-mode=heap) is to extend the longivity of the external storage. Using this option should decrease the number of writes to the SD card. The second option (wasm-runtime=wabt) is default on eosio 2.0.x, but it's a good idea to strictly state it, because currently EOS VM will not run on ARM8.

database-map-mode=heap
and
wasm-runtime=wabt

The following debian package was the result of compiling eosio code from source from https://developers.eos.io/manuals/eos/latest/install/build-from-source/index .

## INSTALL
sudo dpkg -i eosio-2.0-ubuntu-18.04_arm64.deb

Result: Eosio binaries placed in /usr/local/bin


## UNINSTALL
sudo dpkg --remove eosioforrpi4

Result: Eosio binaries removed
