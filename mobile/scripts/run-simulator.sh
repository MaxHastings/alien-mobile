#!/bin/sh
set -eu
mobile_root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
# Prefer an already booted iPhone, then the current target, then any iPhone.
# Accept an explicit UDID as the first argument for another installed device.
simulator_id=${1:-$(xcrun simctl list devices available -j | python3 -c '
import json,sys
phones=[d for group in json.load(sys.stdin)["devices"].values() for d in group if "iPhone" in d["name"]]
phones.sort(key=lambda d:(d["state"]!="Booted",d["name"]!="iPhone 17 Pro"))
if not phones: sys.exit("No iPhone Simulator is installed. Install an iOS runtime in Xcode.")
print(phones[0]["udid"])
')}
cmake -S "$mobile_root" -B "$mobile_root/build-sim" -G Xcode \
  -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_SYSROOT=iphonesimulator \
  -DCMAKE_OSX_ARCHITECTURES=arm64 -DCMAKE_OSX_DEPLOYMENT_TARGET=17.0
cmake --build "$mobile_root/build-sim" --config Release -- CODE_SIGNING_ALLOWED=NO
# Boot is harmlessly rejected if this Simulator is already booted.
xcrun simctl boot "$simulator_id" 2>/dev/null || true
xcrun simctl bootstatus "$simulator_id" -b
xcrun simctl install "$simulator_id" "$mobile_root/build-sim/Release-iphonesimulator/AlienMobileApp.app"
xcrun simctl launch --terminate-running-process "$simulator_id" com.example.AlienMobilePrototype
