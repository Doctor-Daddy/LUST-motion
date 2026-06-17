==================================================================================================================
                       Workflows for Application Build, Test, and Bring-up
==================================================================================================================

=========================
Erase the ESP32
=========================
    ~/.platformio/penv/bin/pio run -e proto-EJ2640 -t erase --upload-port /dev/ttyACM0

That wipes the flash contents. If the board supports the erase target correctly, it removes firmware and stored settings.

=========================
Ordinary compile + upload
=========================
    ~/.platformio/penv/bin/pio run -e proto-EJ2640 -t upload --upload-port /dev/ttyACM0

That is your normal:

    compile
        → link
        → generate firmware image
        → upload to ESP32

=========================
Clean compile + upload
=========================
If you want to force a rebuild first:

    ~/.platformio/penv/bin/pio run -e proto-EJ2640 -t clean
    ~/.platformio/penv/bin/pio run -e proto-EJ2640 -t upload --upload-port /dev/ttyACM0

=========================
Serial monitor
=========================
    ~/.platformio/penv/bin/pio device monitor --port /dev/ttyACM0 --baud 115200

or whatever baud rate the firmware uses.

=========================
One thing I'd check after an erase, if you're still seeing behavior that doesn't match the source code
=========================
I would immediately check whether:

    ls -lh lib/framework/WWWData.h

shows a fresh timestamp after your build. 

Because at this point your "UI changes don't appear" issue is looking more like:
    wrong web assets embedded
    browser cache
    or serving different assets than expected
than a firmware settings issue.

==================================================================================================================
             Web Interface Rebuild
==================================================================================================================
// if you want to force a full rebuild of the front-end, run this line first
    rm -rf interface/build

    cd interface
    npm run build
    cd ..

    rm -f lib/framework/WWWData.h

    ~/.platformio/penv/bin/pio run -e proto-EJ2640 -t clean
    ~/.platformio/penv/bin/pio run -e proto-EJ2640 -t upload --upload-port /dev/ttyACM0

==================================================================================================================
Local development mode - Web UI on workstation, control actual application in ESP32 on the device
==================================================================================================================
Optional local development mode - for editing convenience:

Computer runs Vite locally at localhost:5173
Computer is also connected to ESP32 AP
Vite forwards /rest and /ws to the ESP32
------------------------------------------------------------------------------------------------------------------

When you run local dev:

    cd interface
    npm run dev

then:

    Open in browser:
    http://localhost:5173

    Vite serves UI from:
    your workstation

    /rest and /ws are proxied to:
    ESP32 at 192.168.4.1

So in that mode:

    UI comes from workstation
    machine control comes from ESP32