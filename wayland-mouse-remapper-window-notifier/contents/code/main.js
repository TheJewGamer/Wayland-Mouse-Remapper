/* 
Author: TheJewGamer
Last Update: 3/8/2026
*/

print("wayland-mouse-remapper-window-notifier: script started");

// run when window is activated
workspace.windowActivated.connect(function (window) {
    //make sure not null
    if (!window)
    {
        return;
    }

    // try standard name (might be hidden)
    var appName = window.resourceName;

    // if nothing was try class name
    if (!appName)
    {
        appName = window.resourceClass;
    }

    //if nothing try window caption
    if (!appName)
    {
        appName = window.caption;
    }

    //pipe to access other script
    callDBus(
        "org.mouse.remap",
        "/org/mouse/remap",
        "org.mouse.remap",
        "SetApp",
        appName
    );
});
