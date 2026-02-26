#!/bin/bash
sudo apt -y purge ros-jazzy-pf-driver ros-jazzy-pf-description ros-jazzy-pf-interfaces
rm -f *.deb *.ddeb

pushd pf_description
rm -Rf debian .obj-x86_64-linux-gnu
bloom-generate rosdebian
fakeroot debian/rules binary
popd

pushd pf_interfaces
rm -Rf debian .obj-x86_64-linux-gnu
bloom-generate rosdebian
fakeroot debian/rules binary
popd

sudo dpkg -i ros-jazzy-pf*.deb

pushd pf_driver
rm -Rf debian .obj-x86_64-linux-gnu
bloom-generate rosdebian
fakeroot debian/rules binary
popd

