const APP_ID = "app.busy.js_input_test";

let appSettings;
const accentColors = {
    blue: "#00A8FFFF",
    green: "#2ECC71FF",
    purple: "#9B59B6FF"
};

let eventCount = 0;
let lastEvent = "READY";
let ledColor = accentColors.blue;
let drawInProgress = false;
let drawPending = false;

function drawFinished() {
    drawInProgress = false;

    if(drawPending) {
        drawPending = false;
        drawStatus();
    }
}

function drawStatus() {
    if(drawInProgress) {
        drawPending = true;
        return;
    }

    drawInProgress = true;

    const request = new Request("http://127.0.0.1/api/display/draw", {
        method: "POST",
        body: JSON.stringify({
            application_name: APP_ID,
            priority: 50,
            led_notification_color: ledColor,
            elements: [
                {
                    id: "title",
                    type: "text",
                    timeout: 0,
                    display: "front",
                    x: 36,
                    y: 4,
                    text: appSettings.display.show_event_count ? "INPUT #" + eventCount : "INPUT",
                    font: "small",
                    align: "center",
                    color: "#FFFFFFFF"
                },
                {
                    id: "event",
                    type: "text",
                    timeout: 0,
                    display: "front",
                    x: 36,
                    y: 12,
                    text: lastEvent,
                    font: "small",
                    align: "center",
                    color: "#FFFFFFFF"
                }
            ]
        })
    });

    try {
        fetch(request).then(
            function() {
                drawFinished();
            },
            function(error) {
                console.error("Display draw failed:", error);
                drawFinished();
            }
        );
    } catch(error) {
        console.error("Display draw failed:", error);
        drawFinished();
    }
}

function recordEvent(control, value, color) {
    if(value === "RELEASE" && !appSettings.input.show_releases) {
        return;
    }

    eventCount++;
    lastEvent = appSettings.display.event_format === "compact" ? control : control + " " + value;
    if(appSettings.display.uppercase) {
        lastEvent = lastEvent.toUpperCase();
    }
    ledColor = accentColors[appSettings.display.accent] || color;

    console.info("Input #" + eventCount + ":", control, value);
    drawStatus();
}

function start(config) {
    appSettings = config.values;
    ledColor = accentColors[appSettings.display.accent];

    Input.on("startPause", function(event) {
        recordEvent("START", event.action.toUpperCase(), "#2ECC71FF");
    });

    Input.on("dial", function(event) {
        const step = appSettings.input.dial_step;
        const value = event.direction === "clockwise" ? "CW +" + step : "CCW -" + step;
        recordEvent("DIAL", value, "#00A8FFFF");
    });

    Input.on("ok", function(event) {
        recordEvent("OK", event.action.toUpperCase(), "#F1C40FFF");
    });

    console.info("JS Input Test registered all input handlers");
    drawStatus();
}

export default function run() {
    Settings.load().then(start, function(error) {
        console.error("Failed to load settings:", error);
    });
}
