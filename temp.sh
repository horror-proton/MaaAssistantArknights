TERM=xterm jp2a resource/template/Task.png
wget --content-disposition https://ak.hypergryph.com/downloads/android_lastest
adb install -r arknights-hg-*.apk
adb shell am start -n com.hypergryph.arknights/com.u8.sdk.U8UnityContext
sleep 180
adb exec-out screencap -p > sc.png
TERM=xterm jp2a sc.png
